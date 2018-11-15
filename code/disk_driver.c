#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include "disk_driver.h"
#include "error.h"

// ERROR_HELPER is used for hardware/software failure to close the progam 
// -1 for errors to check in caller functions

//write, read and free already at offset of blocks (offset for diskHeader and bitmap is already calculated inside functions)
//here we use the error helper to define errors as fatal errors 

// opens the file (creating it if necessary)
// allocates the necessary space on the disk
// calculates how big the bitmap should be 
// if the file was new
// compiles a disk header, and fills in the bitmap of appropriate size
// with all 0 (to denote the free space);
void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks){
	if (disk == NULL || filename == NULL || num_blocks < 1){													//check parameters
		ERROR_HELPER(-1, "Impossible to Init: Bad Parameters\n");
	}	
	
	int bitmap_size = num_blocks/bit_in_byte;
	if(num_blocks % 8) bitmap_size+=1;
	
	int fd;
	int is_file = access(filename, F_OK) == 0;
	
	if(is_file){
		fd = open(filename, O_RDWR, (mode_t)0666);																//open file
		ERROR_HELPER(fd, "Impossible to Init: Error int open, exit\n");
		
		DiskHeader* disk_mem = (DiskHeader*) mmap(0, sizeof(DiskHeader)+bitmap_size
			, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);  														//mmap header
		if (disk_mem == MAP_FAILED){
			close(fd);
			ERROR_HELPER(-1, "Impossible to Init: Error mmapping the file\n");
		}
		
		disk->header = disk_mem;																				//save pointers to mmap space 
		disk->bitmap_data = (char*)disk_mem + sizeof(DiskHeader);
	}
	else{
		fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0666);											//create a file
		ERROR_HELPER(fd, "Impossible to Init: Error int open, exit\n");
	
		if(posix_fallocate(fd, 0, sizeof(DiskHeader)+bitmap_size) > 0){											//File is new zero sized file,
			ERROR_HELPER(-1, "Impossible to Init: Error in fallocate\n");										//fallocate alloc the necessary space to mmap  
		}																										//on file
		DiskHeader* disk_mem = (DiskHeader*) mmap(0, sizeof(DiskHeader)+bitmap_size
			, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);  														//mmap header
		if (disk_mem == MAP_FAILED){
			close(fd);
			ERROR_HELPER(-1, "Impossible to Init: Error mmapping the file\n");
		}
	
		disk->header = disk_mem;																				//save pointers to mmap space 
		disk->bitmap_data = (char*)disk_mem + sizeof(DiskHeader);												//pointer to bitmap calculated by header offset
    
		disk_mem->num_blocks = num_blocks;																		//complete header
		disk_mem->bitmap_blocks = num_blocks;
		disk_mem->bitmap_entries = bitmap_size;
		disk_mem->free_blocks = num_blocks;			
		disk_mem->first_free_block = 0;	
		bzero(disk->bitmap_data, bitmap_size);
	}																											//function to put 0 in every byte for bitmap_size length
	
	
	disk->fd = fd;
}

// reads the block in position block_num
// returns -1 if the block is free according to the bitmap
// 0 otherwise
int DiskDriver_readBlock(DiskDriver* disk, void* dest, int block_num){
	if(block_num > disk->header->bitmap_blocks || block_num < 0 || dest == NULL || disk == NULL ){				//check parameters
		ERROR_HELPER(-1, "Impossible to read: Bad Parameters\n");
	}
	
	BitMap bmap;															                                    //allocate bitmap structure, necessary to use bitmap functions
	bmap.num_bits = disk->header->bitmap_blocks;																//copy num blocks bitmap
	bmap.entries = disk->bitmap_data;																			//copy bitmap													
	
	if(!BitMap_getBit(&bmap, block_num)) 																		//check it the block is free
        return -1;				

    int fd = disk->fd;
	off_t offset = lseek(fd, sizeof(DiskHeader)+disk->header->bitmap_entries+(block_num*BLOCK_SIZE), SEEK_SET);	//set pointer of fd on the block to read it
	ERROR_HELPER(offset, "Impossible to read: Error in lseek\n");												
														
	
	int ret, bytes_reads = 0;
	while(bytes_reads < BLOCK_SIZE){																			//loop to check errors 
		ret = read(fd, dest + bytes_reads, BLOCK_SIZE - bytes_reads);

		if (ret == -1 && errno == EINTR) continue;																//if error occured by interrupt just restart
	    ERROR_HELPER(ret, "Impossible to read: error in loop\n");												//if impossible to read	exit

		bytes_reads +=ret;																						
	}

	return 0;	 
}

// writes a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num){
	if(block_num > disk->header->bitmap_blocks || block_num < 0 || src == NULL || disk == NULL ){				//check parameters
		ERROR_HELPER(-1, "Impossible to write: Bad Parameters\n");
	}
	
	BitMap bmap;			                                    												//same as read
	bmap.num_bits = disk->header->bitmap_blocks;
	bmap.entries = disk->bitmap_data;
		
    if(BitMap_getBit(&bmap, block_num))                                                                         //check it the block is already full
        return -1;
	
	if(block_num == disk->header->first_free_block)																//update first_free_block
	    disk->header->first_free_block = DiskDriver_getFreeBlock(disk, block_num+1);
	    
	BitMap_set(&bmap, block_num, 1);																			//block become full on bitmap
	disk->header->free_blocks -=1;

    int fd = disk->fd;
	off_t offset = lseek(fd, sizeof(DiskHeader)+disk->header->bitmap_entries+(block_num*BLOCK_SIZE), SEEK_SET);	//set pointer of fd on the block to write it
	ERROR_HELPER(offset, "Impossible to write: Error in lseek\n");
	
		
	int ret, written_bytes = 0;
	while(written_bytes < BLOCK_SIZE){																			//loop to check errors 
		ret = write(fd, src + written_bytes, BLOCK_SIZE - written_bytes);

		if (ret == -1 && errno == EINTR) continue;																//if error occured by interrupt just restart
        ERROR_HELPER(ret, "Impossible to write: error in loop\n");												//if it's impossible to write => exit

		written_bytes += ret;						
	}	
	return 0;	 
}

// update a block in position block_num, do not alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_updateBlock(DiskDriver* disk, void* src, int block_num){
	if(block_num > disk->header->bitmap_blocks || block_num < 0 || src == NULL || disk == NULL ){				//check parameters
		ERROR_HELPER(-1, "Impossible to write: Bad Parameters\n");
	}
	
    int fd = disk->fd;
	off_t offset = lseek(fd, sizeof(DiskHeader)+disk->header->bitmap_entries+(block_num*BLOCK_SIZE), SEEK_SET);	//set pointer of fd on the block to write it
	ERROR_HELPER(offset, "Impossible to write: Error in lseek\n");
	
		
	int ret, written_bytes = 0;
	while(written_bytes < BLOCK_SIZE){																			//loop to check errors 
		ret = write(fd, src + written_bytes, BLOCK_SIZE - written_bytes);

		if (ret == -1 && errno == EINTR) continue;																//if error occured by interrupt just restart
        ERROR_HELPER(ret, "Impossible to write: error in loop\n");												//if it's impossible to write => exit

		written_bytes += ret;						
	}	
	return 0;	 
}

// frees a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_freeBlock(DiskDriver* disk, int block_num){
	if(block_num > disk->header->bitmap_blocks || block_num < 0 || disk == NULL){								//check parameters
		ERROR_HELPER(-1, "Impossible to free: Bad Parameters\n");
	}
	
	BitMap bmap;																								//same as read
	bmap.num_bits = disk->header->bitmap_blocks;
	bmap.entries = disk->bitmap_data;
	
	if(!BitMap_getBit(&bmap, block_num)) 																		//block already free
        return -1;

	if(BitMap_set(&bmap, block_num, 0) < 0){																	//set bit to 0 on bitmap
		printf("(from disk) Impossible to free: impossible to set bit on bitmap\n");
		return -1;
	}
	
	if(block_num < disk->header->first_free_block || disk->header->first_free_block == -1)																
	    disk->header->first_free_block = block_num;																//update first_free_block
	    
	disk->header->free_blocks += 1;																				//set change on disk structure

	return 0;														
}


// same as free block + set every block to 0
//become impossible to restore old data
int DiskDriver_ExtremeFreeBlock(DiskDriver* disk, int block_num){
	if(block_num > disk->header->bitmap_blocks || block_num < 0 || disk == NULL){								//check parameters
		ERROR_HELPER(-1, "Impossible to free: Bad Parameters\n");
	}
	
	BitMap bmap;														                                    	//same as read
	bmap.num_bits = disk->header->bitmap_blocks;
	bmap.entries = disk->bitmap_data;
	
	if(!BitMap_getBit(&bmap, block_num)) 
        return -1;
	
    int fd = disk->fd;
	off_t offset = lseek(fd, sizeof(DiskHeader)+disk->header->bitmap_entries+(block_num*BLOCK_SIZE), SEEK_SET);	//set pointer of fd on the block
	ERROR_HELPER(offset, "Impossible to free: Error in lseek\n");
	
    char buffer[BLOCK_SIZE] = {0};																				//write all 0 on block to free
    int ret, written_bytes = 0;																					//I can't use DiskDriver_writeBlock, 
	while(written_bytes < BLOCK_SIZE){																			//because it return -1 if block is, 
		ret = write(fd, buffer + written_bytes, BLOCK_SIZE - written_bytes);								    //already written

		if (ret == -1 && errno == EINTR) continue;																//if error occured by interrupt just restart
        ERROR_HELPER(ret, "Impossible to free: error in loop\n");												//if impossible to write exit

		written_bytes += ret;
	}

	if(BitMap_set(&bmap, block_num, 0) < 0){																	//set bit to 0 on bitmap
		printf("(from disk) Impossible to free: impossible to set bit on bitmap\n");
		return -1;
	}
	
	if(block_num < disk->header->first_free_block || disk->header->first_free_block == -1)																
	    disk->header->first_free_block = block_num;	
	
	disk->header->free_blocks += 1;																				//set change on disk structure

	return 0;														
}

// returns the first free blockin the disk from position (checking the bitmap)
int DiskDriver_getFreeBlock(DiskDriver* disk, int start){
	if(start > disk->header->bitmap_blocks){																	//check parameters
		ERROR_HELPER(-1, "Impossible to getFreeBlock: Bad Parameters\n");
	}
	
	BitMap bmap;																								//same as read
	bmap.num_bits = disk->header->bitmap_blocks;
	bmap.entries = disk->bitmap_data;
	
	int free_block = BitMap_get(&bmap, start, 0);																//use get to find the firsdt free block by bitma
	
	return free_block;
}

// writes the data (flushing the mmaps)
int DiskDriver_flush(DiskDriver* disk){
	int bitmap_size = disk->header->num_blocks/bit_in_byte+1;
	int ret = msync(disk->header, (size_t)sizeof(DiskHeader)+bitmap_size, MS_SYNC);								//Flush header and bitmap on file 
    ERROR_HELPER(ret, "Could not sync the file to disk\n");
	return 0;
}
