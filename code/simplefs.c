#include <stdlib.h>
#include <stdio.h>
#include "simplefs.h"
#include "error.h"
#include "shell.h"

// -1 on header => error, no block on the chain for now
// same for directories in FCB

//needs to write all 0 in the directory before to create it
//there are commented print to check functions working flow


// initializes a file system on an already made disk
// returns a handle to the top level directory stored in the first block
DirectoryHandle* SimpleFS_init(SimpleFS* fs, DiskDriver* disk){
	if (fs == NULL || disk == NULL)
		ERROR_HELPER(-1, "Impossible to Init: Bad Parameters\n");

	fs->disk = disk;

	FirstDirectoryBlock* fdb = malloc(sizeof(FirstDirectoryBlock));

	int ret = DiskDriver_readBlock(disk, fdb, 0);													//read block 0 of already made disk for the root directory
	if (ret == -1){																					//no root directory, problems on filesystem
		free(fdb);
		//printf("%simpossible to use SimpleFS_init, needs to make the disk\n%s", COL_RED, COL_GRAY);
		return NULL;
	}

	DirectoryHandle* dh = (DirectoryHandle*) malloc(sizeof(DirectoryHandle));						//create and populate handle to return
	dh->sfs = fs;
	dh->dcb = fdb;
	dh->directory = NULL;
	//dh->current_block = &fdb->header;																//save the pointer to directory
	//dh->pos_in_dir = 0;
	dh->pos_in_block = 0;

	//printf("init completed\n");
	return dh;																						//return handler to root
}



// creates the inital structures, the top level directory
// has name "/" and its control block is in the first position
// it also clears the bitmap of occupied blocks on the disk
// the current_directory_block is cached in the SimpleFS struct
// and set to the top level directory
void SimpleFS_format(SimpleFS* fs){
	if (fs == NULL)
		ERROR_HELPER(-1, "Impossible to format: Bad Parameters\n");

	int ret = 0;

	FirstDirectoryBlock rootDir = {0};																//create the block for root directory, set to 0 to clean old data
	rootDir.header.block_in_file = 0;																//populate header
	rootDir.header.previous_block = -1;
	rootDir.header.next_block = -1;
																									//populate fcb
	rootDir.fcb.directory_block = -1;																//no parents => -1
	rootDir.fcb.block_in_disk = 0;
	//rootDir.fcb.size_in_blocks = 1;
	//rootDir.fcb.size_in_bytes = BLOCK_SIZE;
	rootDir.fcb.is_dir = 1;
	strcpy(rootDir.fcb.name, "/");

	fs->disk->header->free_blocks = fs->disk->header->num_blocks;									//clear bitmap to simple format disk
	fs->disk->header->first_free_block = 0;															//starts by 0 because writeBlock will change bitmap
	int bitmap_size = fs->disk->header->bitmap_entries;
	bzero(fs->disk->bitmap_data, bitmap_size);														//function to put 0 in every bytes for bitmap_size length

	ret = DiskDriver_writeBlock(fs->disk, &rootDir, 0);												//write root directory on block 0, offset of diskHeader and bitmap_data already calculated by write
	if (ret == -1)
		printf("%sImpossible to format: problem on writeBlock\n%s", COL_RED, COL_GRAY);				//can't return error becouse function return imposted on void

	//printf("formatting completed\n");
}



//same as format + set every block to 0
//then become impossible to restore old data
void SimpleFS_Extremeformat(SimpleFS* fs){
	if (fs == NULL)
		ERROR_HELPER(-1, "Impossible to format: Bad Parameters\n");

	int ret = 0, i = 0;

	FirstDirectoryBlock rootDir = {0};																//create the block for root directory
	rootDir.header.block_in_file = 0;																//populate header
	rootDir.header.previous_block = -1;
	rootDir.header.next_block = -1;
																									//populate fcb
	rootDir.fcb.directory_block = -1;																//no parents => -1
	rootDir.fcb.block_in_disk = 0;
	//rootDir.fcb.size_in_blocks = 1;
	//ootDir.fcb.size_in_bytes = BLOCK_SIZE;
	rootDir.fcb.is_dir = 1;
	strcpy(rootDir.fcb.name, "/");

																									//ATTENTION to do before to clean the bitmap, becouse writeBlock also set bitmap
	char buffer[BLOCK_SIZE] = {0};																	//write all 0 on every block in the disk, starts by 1 becouse block 0 is already written
	for (i=1; i < fs->disk->header->num_blocks; i++){												//also root for file_blocks part
		ret = DiskDriver_writeBlock(fs->disk, buffer, i);
		if (ret == -1)
			printf("%sImpossible to extreme format: problem to write 0 on blocks\n%s", COL_RED, COL_GRAY);
	}

	fs->disk->header->free_blocks = fs->disk->header->num_blocks;									//clear bitmap to simple format disk
	fs->disk->header->first_free_block = 0;															//starts by 0 because writeBlock will change bitmap
	int bitmap_size = fs->disk->header->bitmap_entries;
	bzero(fs->disk->bitmap_data, bitmap_size);														//function to put 0 in all byte for bitmap_size length

	DiskDriver_writeBlock(fs->disk, &rootDir, 0);													//write root directory on block 0, offset of diskHeader and bitmap_data already calculated by write
	if (ret == -1)
		printf("%sImpossible to format: problem on writeBlock\n%s", COL_RED, COL_GRAY);

	//printf("extreme formatting completed\n");
}



//helping function
//returns position of file if it already exist in the current block of the directory
//otherwise return -1
int checkFileEsistence(DiskDriver* disk, int entries, int* file_blocks, const char* filename){
    FirstFileBlock to_check;                                                                        //to save the current file, needs to read the name
    int i;
    
    for (i = 0; i < entries; i++){                                               	                //checks every block indicator in the directory block
        ///printf("inside checkFileEsistence, file_blocks[%d] = %d\n", i, file_blocks[i]);
		if (file_blocks[i]> 0 && (DiskDriver_readBlock(disk, &to_check, file_blocks[i]) != -1)){    //read the FirstFileBlock of the file to check the name (if to_check block is not empty, block[i]>0)
            if (!strncmp(to_check.fcb.name, filename, MAX_NAME_LEN)){                          		//function to compare name strings, return 0 if s1 == s2
                return i;                                                                       	//found the file, return 1 to exit on create_file
            }
        }
    }
    return -1;
}

// creates an empty file in the directory d
// returns null on error (file existing, no free blocks)
// an empty file consists only of a block of type FirstBlock
FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename){
	if (d == NULL || filename == NULL)
		ERROR_HELPER(-1, "Impossible to create file: Bad Parameters\n");

	int ret = 0;
	FirstDirectoryBlock *fdb = d->dcb;																//save pointers, used for performance
	DiskDriver* disk = d->sfs->disk;
	//printf("inside createFile, num entries = %d\n", fdb->num_entries);

	if (fdb->num_entries > 0){																		//directory not empty, needs to check if file already exist
		//printf("check if file already exist\n");
		if (checkFileEsistence(disk, FDB_space, fdb->file_blocks, filename) != -1){					//funcion to check if the file is in the FirstDirectoryBlock
			printf("%sImpossible to create file: file already exist\n%s", COL_RED, COL_GRAY);
			return NULL;																			//if yes return null
		}

		int next = fdb->header.next_block;
		DirectoryBlock db;

		while (next != -1){																			//to check if file in another block of the directory
			//printf("file is not the block, check other block\n");
			ret = DiskDriver_readBlock(disk, &db, next);											//read new directory block
			if (ret == -1){
				printf("%sImpossible to create file: problem on readBlock to read next block and find already made same file\n%s", COL_RED, COL_GRAY);
				return NULL;
			}

			if (checkFileEsistence(disk, DB_space, db.file_blocks, filename) != -1){				//to check if the file is in the DirectoryBlock
				printf("%sImpossible to create file: file already exist\n%s", COL_RED, COL_GRAY);
				return NULL;
			}
			next = db.header.next_block;															//goto next directoryBlock
		}
	}

	int new_block = DiskDriver_getFreeBlock(disk, disk->header->first_free_block);					//search a free block in the disk for the file
	if (new_block == -1){
		printf("%sImpossible to create file: impossible to find free block\n%s", COL_RED, COL_GRAY);
		return NULL;
	}
	//printf("allocation structures for file and write on disk, block number = %d\n", new_block);
																									//create new file
	FirstFileBlock* new_file = calloc(1, sizeof(FirstFileBlock));									//use calloc to put all 0 on the file
	new_file->header.block_in_file = 0;																//populate header of the file
	new_file->header.next_block = -1;
	new_file->header.previous_block = -1;

	new_file->fcb.directory_block = fdb->fcb.block_in_disk;											//populate FileControlBlock
	new_file->fcb.block_in_disk = new_block;
	//new_file->fcb.size_in_bytes = BLOCK_SIZE;
	//new_file->fcb.size_in_blocks = 1;
	new_file->fcb.is_dir = 0;
	new_file->fcb.written_bytes = 0;
	strncpy(new_file->fcb.name, filename, MAX_NAME_LEN);

	ret = DiskDriver_writeBlock(disk, new_file, new_block);											//write file on the disk
	if (ret == -1){
		printf("%sImpossible to create file: problem on writeBlock to write file on disk\n%s", COL_RED, COL_GRAY);
		return NULL;
	}

																									//save changes on directory
	int i = 0;
	int found = 0;																					//to check if there is space in already created blocks of the directory
	int block_number = fdb->fcb.block_in_disk;														//number of the current block inside the disk
	DirectoryBlock db_last;																			//in case of no space in firstDirectoryBlock this will save the current block

	int entry = 0;																					//to save the number of the entry in file_blocks of the directoryBlock / firstDurectoryBlock
	int blockInFile = 0;																			//indicate the number of the block in the directory
	int first_or_not = 0;																			//if 0 save the next block after fdb, if 1 after db_last 										
    int where_to_save = 0;																			//if 0 there is space in firstDirectoryBlock, if 1 space in another block																	

	if (fdb->num_entries < FDB_space){																//check if free space in FirstDirectoryBlock (implicit if num_entry == 0)
		//printf("there is space in fdb\n");
		int* blocks = fdb->file_blocks;
		for(i=0; i<FDB_space; i++){																	//loop to find the position
			///printf("finding space, blocks[%d] = %d\n", i, blocks[i]);
			if (blocks[i] == 0){																	//free space in fdb->blocks[i]
				found = 1;
				entry = i;
				break;
			}
		}
	} else{																							//in case of not space in firstDirectoryBlock
		where_to_save = 1;
		int next = fdb->header.next_block;

		while (next != -1 && !found){																//loop to find position in another DirectoryBlock
			ret = DiskDriver_readBlock(disk, &db_last, next);										//read new directory block
			if (ret == -1){
				printf("%sImpossible to create file: problem on readBlock to read directory and change status\n%s", COL_RED, COL_GRAY);
				DiskDriver_freeBlock(disk, fdb->fcb.block_in_disk);									//needs to free the block already written because it's imposssible to complete operations
				return NULL;
			}
			int* blocks = db_last.file_blocks;
			blockInFile++;																			//change block => update variables states
			block_number = next;
			for(i=0; i<DB_space; i++){																//loop to find the position
				///printf("finding space, blocks[%d] = %d\n", i, blocks[i]);
				if (blocks[i] == 0){
					found = 1;
					entry = i;
					break;
				}

			}
			first_or_not = 1;
			next = db_last.header.next_block;
		}
	}

	if (!found){																					//in case of all blocks of the directory are already full
		//printf("all blocks are already full, directory needs new block\n");
		DirectoryBlock new_db = {0};																//create new directoryBlock
		new_db.header.next_block = -1;
		new_db.header.block_in_file = blockInFile;
		new_db.header.previous_block = block_number;											
		new_db.file_blocks[0] = new_block;															//save the FirstFileBlock of the file just created

		int new_dir_block = DiskDriver_getFreeBlock(disk, disk->header->first_free_block);			//search a free block for the directoryBlock in the disk
		if (new_block == -1){
			printf("%sImpossible to create file: impossible to find free block to create a new block for directory\n%s", COL_RED, COL_GRAY);
			DiskDriver_freeBlock(disk, fdb->fcb.block_in_disk);										//needs to free the block already written because it's imposssible to complete operations
			return NULL;
		}

		ret = DiskDriver_writeBlock(disk, &new_db, new_dir_block);									//write block on the disk
		if (ret == -1){
				printf("%sImpossible to create file: problem on writeBlock to write file on disk\n%s", COL_RED, COL_GRAY);
				DiskDriver_freeBlock(disk, fdb->fcb.block_in_disk);									//needs to free the block already written because it's imposssible to complete operations
				return NULL;
		}
	
		if (first_or_not == 0){
			fdb->header.next_block = new_dir_block;													//update header current block
		} else{
			db_last.header.next_block = new_dir_block;
		}
		db_last = new_db;
		block_number = new_dir_block;

	} 

	if (where_to_save == 0){																		//space in firstDirectoryBlock
		fdb->num_entries++;	
		fdb->file_blocks[entry] = new_block;														//save new_block position in firstDirectoryBlock
		DiskDriver_updateBlock(disk, fdb, fdb->fcb.block_in_disk);
	} else{
		fdb->num_entries++;	
		DiskDriver_updateBlock(disk, fdb, fdb->fcb.block_in_disk);
		db_last.file_blocks[entry] = new_block;
		DiskDriver_updateBlock(disk, &db_last, block_number);
	}
	
	FileHandle* fh = malloc(sizeof(FileHandle));													//create and populate handle to return
	fh->sfs = d->sfs;
	fh->fcb = new_file;
	fh->directory = fdb;
	//fh->current_block = (BlockHeader*) new_file;													//necessary cast
	fh->pos_in_file = 0;
	
	//printf("file created\n");
	return fh;
}



// reads in the (preallocated) blocks array, the name of all files in a directory
// return number of file / directory reads or -1 in case of error
int SimpleFS_readDir(char** names, DirectoryHandle* d){
	if (d == NULL || names == NULL)
		ERROR_HELPER(-1, "Impossible to read directory: Bad Parameters\n");

	int ret = 0, num_tot = 0;
	FirstDirectoryBlock *fdb = d->dcb;																//save pointers, used for performance
	DiskDriver* disk = d->sfs->disk;

	if (fdb->num_entries > 0){																		//directory not empty
		int i;																						
		FirstFileBlock to_check;																	//to save the current file, needs to read the name (can use also for directory)

		int* blocks = fdb->file_blocks;
		for (i = 0; i < FDB_space; i++){															//checks every block entry in the directory FirstDirectoryBlock
			if (blocks[i]> 0 && DiskDriver_readBlock(disk, &to_check, blocks[i]) != -1){			//read the FirstFileBlock of the file to check the name (if to_check block is not empty, block[i]>0)
				names[num_tot] = strndup(to_check.fcb.name, MAX_NAME_LEN);
                num_tot++;
			}
		}

		//printf("num = %d\n", i);
		//printf("num = %d\n", fdb->num_entries);

		if (fdb->num_entries > i){																	//check if more entries in other bloks of the directory
			int next = fdb->header.next_block;
			DirectoryBlock db;

			while (next != -1){																		//to check files / directories in other blocks of the directory
				ret = DiskDriver_readBlock(disk, &db, next);										//read new directory block
				if (ret == -1){
					printf("%sImpossible to read all direcory, problems on next block\n%s", COL_RED, COL_GRAY);
					return -1;
				}

				int* blocks = db.file_blocks;
				for (i = 0; i < DB_space; i++){														//checks every block indicator in the directory FirstDirectoryBlock
					if (blocks[i]> 0 && DiskDriver_readBlock(disk, &to_check, blocks[i]) != -1){	//read the FirstFileBlock of the file to check the name (if to_check block is not empty, block[i]>0)
						names[num_tot] = strndup(to_check.fcb.name, MAX_NAME_LEN);
                        num_tot++;
					}
				}

				next = db.header.next_block;														//goto next directoryBlock
			}
		}
		//printf("completed to read directory\n");

	} /*else {
		printf("directory empty\n");
		
	}*/

	return 0;
}



// opens a file in the  directory d. The file should be exisiting
FileHandle* SimpleFS_openFile(DirectoryHandle* d, const char* filename){
	if (d == NULL || filename == NULL)
		ERROR_HELPER(-1, "Impossible to open file: Bad Parameters\n");

	int ret = 0;
	FirstDirectoryBlock *fdb = d->dcb;																//save pointers, used for performance
	DiskDriver* disk = d->sfs->disk;

	if (fdb->num_entries > 0){																		//directory not empty, maybe there is the file
		FileHandle* fh = malloc(sizeof(FileHandle));												//create and start to populate handle to return
		fh->sfs = d->sfs;
		fh->directory = fdb;
		fh->pos_in_file = 0;

		int found = 0;																				//boolean file exist
		FirstFileBlock* to_check = malloc(sizeof(FirstFileBlock));

		int pos = checkFileEsistence(disk, FDB_space, fdb->file_blocks, filename);					//funcion to check if the file is in the FirstDirectoryBlock
			if (pos >= 0){
				found = 1;
				DiskDriver_readBlock(disk, to_check, fdb->file_blocks[pos]);						//read again the correct file to complete handler
				fh->fcb = to_check;
				//fh->current_block = (BlockHeader*) to_check;
			}

		int next = fdb->header.next_block;
		DirectoryBlock db;

		while (next != -1 && !found){																//to check if file in another block of the directory
			ret = DiskDriver_readBlock(disk, &db, next);											//read new directory block
			if (ret == -1){
				printf("%sImpossible to create file: problem on readBlock to read next block and find already made same file\n%s", COL_RED, COL_GRAY);
				return NULL;
			}
			
			pos = checkFileEsistence(disk, DB_space, db.file_blocks, filename);						//funcion to check if the file is in the FirstDirectoryBlock
			if (pos >= 0){
				found = 1;
				DiskDriver_readBlock(disk, to_check, db.file_blocks[pos]);							//read again the correct file to complete handler
				fh->fcb = to_check;
				//fh->current_block = (BlockHeader*) to_check;
			}

			next = db.header.next_block;															//goto next directoryBlock
		}

		if (found){
			//printf("file opened\n");
			return fh;
		} else {
			printf("%sImpossible to open file: file doesn't exist\n%s", COL_RED, COL_GRAY);
			free(fh);
			return NULL;
		}

	} else {																						//if 0 entries => directory empty
		printf("%sImpossible to open file: direcoty is empty\n%s", COL_RED, COL_GRAY);
		return NULL;
	}
}


// closes a file handle (destroyes it)
int SimpleFS_close(FileHandle* f){
	free(f->fcb);
	free(f);
	return 0;
}


// writes in the file, at current position for size bytes stored in data
// overwriting and allocating new space if necessary
// returns the number of bytes written
int SimpleFS_write(FileHandle* f, void* data, int size){
	FirstFileBlock* ffb = f->fcb;
	
	int written_bytes = 0;
	int to_write = size;
	int off = f->pos_in_file;
		
	if(off < FFB_space && to_write <= FFB_space-off){											//If byte to be written are smaller or equal of space available
		memcpy(ffb->data+off, (char*)data, to_write);											//then we write the bytes
		written_bytes += to_write;
		if(f->pos_in_file+written_bytes > ffb->fcb.written_bytes) 
			ffb->fcb.written_bytes = f->pos_in_file+written_bytes;
		DiskDriver_updateBlock(f->sfs->disk, ffb, ffb->fcb.block_in_disk);
		return written_bytes;
	}
	else if(off < FFB_space && to_write > FFB_space-off){							
		memcpy(ffb->data+off, (char*)data, FFB_space-off);										//then we write the bytes
		written_bytes += FFB_space-off;
		to_write = size - written_bytes;
		DiskDriver_updateBlock(f->sfs->disk, ffb, ffb->fcb.block_in_disk);
		off = 0;
	}
	else off-=FFB_space;
	
	int block_in_disk = ffb->fcb.block_in_disk;													//id on disk of current block
	int next_block = ffb->header.next_block;													//id on disk of next block
	int block_in_file = ffb->header.block_in_file;												//id on file of current block
	FileBlock fb_tmp;																			//structure auxiliary
	int one_block = 0;																		
	if(next_block == -1) one_block = 1;
	
	while(written_bytes < size){
		if(next_block == -1){																	//allocate new block if space for data is terminated
			FileBlock n_fb = {0};
			n_fb.header.block_in_file = block_in_file+1;												
			n_fb.header.next_block = -1;
			n_fb.header.previous_block = block_in_disk;
			
			next_block = DiskDriver_getFreeBlock(f->sfs->disk, block_in_disk);					//update id of next_block
			if(one_block == 1){																	//we are on FirstFileBlock
				ffb->header.next_block = next_block;
				DiskDriver_updateBlock(f->sfs->disk, ffb, ffb->fcb.block_in_disk);				//write changes on disk
				one_block = 0;	
			}																					
			else{
				fb_tmp.header.next_block = next_block;											//update id nextblock of prevoius fileblock
				DiskDriver_updateBlock(f->sfs->disk, &fb_tmp, block_in_disk);					//write changes on disk
			}
			DiskDriver_writeBlock(f->sfs->disk, &n_fb, next_block);								//write changes on disk
			
			fb_tmp = n_fb;
		}
		else 
			if(DiskDriver_readBlock(f->sfs->disk, &fb_tmp, next_block) == -1) return -1;		//read FileBlock by disk
				
		if(off < FB_space && to_write <= FB_space-off){											//if byte to be written are smaller of space available
			memcpy(fb_tmp.data+off, (char*)data+written_bytes, to_write);						//then we write the last bytes and exit from cycle
			written_bytes += to_write;															//update changes
			if(f->pos_in_file+written_bytes > ffb->fcb.written_bytes) 
				ffb->fcb.written_bytes = f->pos_in_file+written_bytes;
			DiskDriver_updateBlock(f->sfs->disk, ffb, ffb->fcb.block_in_disk);
			DiskDriver_updateBlock(f->sfs->disk, &fb_tmp, next_block);
			return written_bytes;
		}
		else if(off < FB_space && to_write > FB_space-off){										//if byte to be written are smaller of space available
			memcpy(fb_tmp.data+off, (char*)data+written_bytes, FB_space-off);					//then we write the last bytes and exit from cycle
			written_bytes += FB_space-off;														//update changes
			to_write = size - written_bytes;
			DiskDriver_updateBlock(f->sfs->disk, &fb_tmp, next_block);
			off = 0;
		}
		else off-=FB_space;
		
		block_in_disk = next_block;																//update id of current_block
		next_block = fb_tmp.header.next_block;
		block_in_file = fb_tmp.header.block_in_file;											//update id of next_block																			
	}

	return written_bytes;
}

// reads in the file, at current position size bytes stored in data
// returns the number of bytes read
int SimpleFS_read(FileHandle* f, void* data, int size){
	FirstFileBlock* ffb = f->fcb;
	
	int off = f->pos_in_file;																	//position in file
	int written_bytes = ffb->fcb.written_bytes;													//bytes written in file
	
	if(size+off > written_bytes){																
		printf("%sINVALID SIZE: choose a smaller size\n%s", COL_RED, COL_GRAY);
		bzero(data, size);
		return -1;
	}
	
	int bytes_read = 0;
	int to_read = size;
	
	if(off < FFB_space && to_read <= FFB_space-off){											//If byte to be read are smaller or equal of space available
		memcpy(data, ffb->data+off, to_read);													//then we read the bytes				
		bytes_read += to_read;
		to_read = size - bytes_read;
		f->pos_in_file += bytes_read;
		return bytes_read;
	}
	else if(off < FFB_space && to_read > FFB_space-off){										//else read bytes
		memcpy(data, ffb->data+off, FFB_space-off);																	
		bytes_read += FFB_space-off;
		to_read = size - bytes_read;
		off = 0;
	}
	else off -= FFB_space;
		
	int next_block = ffb->header.next_block;													//id on disk of next block
	FileBlock fb_tmp;																			//structure auxiliary
	
	while(bytes_read < size && next_block != -1){
		if(DiskDriver_readBlock(f->sfs->disk, &fb_tmp, next_block) == -1) return -1;
				
		if(off < FB_space && to_read <= FB_space-off){											//if byte to be read are smaller of space available
			memcpy(data+bytes_read, fb_tmp.data+off, to_read);									//then we read the bytes and terminate						
			bytes_read += to_read;
			to_read = size - bytes_read;
			f->pos_in_file += bytes_read;
			return bytes_read;
		}
		else if(off < FB_space && to_read > FB_space-off){										//else read bytes and continue
			memcpy(data+bytes_read, fb_tmp.data+off, FB_space-off);																	
			bytes_read += FB_space-off;
			to_read = size - bytes_read;
			off = 0;
		}
		else off -= FB_space;
		
		next_block = fb_tmp.header.next_block;
	}

	return bytes_read;
}

// returns the number of bytes read (moving the current pointer to pos)
// returns pos on success
// -1 on error (file too short)
int SimpleFS_seek(FileHandle* f, int pos){
	FirstFileBlock* ffb = f->fcb;																	//auxiliary structure
		
	if(pos > ffb->fcb.written_bytes){																		
		printf("%sINVALID POS: choose a smaller pos\n%s", COL_RED, COL_GRAY);
		return -1;
	}
	
	f->pos_in_file = pos;
	return pos;
}



//helping function
//returns position of direcroty if it already exist in the current block of the directory
//otherwise return -1
int checkDirEsistence(DiskDriver* disk, int entries, int* file_blocks, const char* filename){
    FirstDirectoryBlock to_check;                                                                   //to save the current file, needs to read the name
    int i;
    
    for (i = 0; i < entries; i++){                                               	                //checks every block indicator in the directory block
        ///printf("inside checkFileEsistence, file_blocks[%d] = %d, i = %d\n", i, file_blocks[i]);
        if (file_blocks[i]> 0 && DiskDriver_readBlock(disk, &to_check, file_blocks[i]) != -1){      //read the FirstFileBlock of the file to check the name (if to_check block is not empty, block[i]>0)
            if (!strncmp(to_check.fcb.name, filename, MAX_NAME_LEN)){                          		//function to compare name strings, return 0 if s1 == s2
                return i;                                                                       	//found the file, return 1 to exit on create_file
            }
        }
    }
    return -1;
}

// seeks for a directory in d. If dirname is equal to ".." it goes one level up
// 0 on success, negative value on error
// it does side effect on the provided handle
int SimpleFS_changeDir(DirectoryHandle* d, char* dirname){
	if (d == NULL || dirname == NULL)
		ERROR_HELPER(-1, "Impossible to change dir: Bad Parameters\n");
	
	int ret = 0;

	if (!strncmp(dirname, "..", 2)){																//go back
		if (d->dcb->fcb.block_in_disk == 0){														//check if root
			printf("%sImpossible to read parent directory, this is root directory\n%s", COL_RED, COL_GRAY);
			return -1;
		}
		d->pos_in_block = 0;																		//reset directory
		//d->pos_in_dir = 0;
		//d->current_block = &(d->directory->header);
		d->dcb = d->directory; 																		//this directory become the parent directory
																									//search parent directory
		int parent_block =  d->dcb->fcb.directory_block;											//save first block of parent directory
		if (parent_block == -1){																	//new directory is root directory
			d->directory = NULL;
			//printf("directory changed\n");
			return 0;
		}
		FirstDirectoryBlock* parent = malloc(sizeof(FirstDirectoryBlock));
		ret = DiskDriver_readBlock(d->sfs->disk, parent, parent_block);								//read the parent directory to save it
		if (ret == -1){
			printf("%sImpossible to read parent directory during go back\n%s", COL_RED, COL_GRAY);
			d->directory = NULL;																	//problems to read, handle will nor have the parent
		} else{
			d->directory = parent;																	//save the correct parent
		}
		//printf("directory changed\n");
		return 0;
	} else if (d->dcb->num_entries < 0){
		printf("%sImpossible to change directory, this directory is empty\n%s", COL_RED, COL_GRAY);
		return -1;
	}
																									//if not go back and directory not empty
	FirstDirectoryBlock *fdb = d->dcb;																//save pointers, used for performance
	DiskDriver* disk = d->sfs->disk;

	FirstDirectoryBlock* to_check = malloc(sizeof(FirstDirectoryBlock));

	int pos = checkDirEsistence(disk, FDB_space, fdb->file_blocks, dirname);						//funcion to check if the directory is in the FirstDirectoryBlock
	if (pos >= 0){
		DiskDriver_readBlock(disk, to_check, fdb->file_blocks[pos]);								//read again the correct directory to save it
		d->pos_in_block = 0;																		//reset directory
		//d->pos_in_dir = 0;
		d->directory = fdb;																			//parent directory become this director
		d->dcb = to_check; 																			//this directory become the read directory
		//d->current_block = &(to_check->header);
		//printf("directory changed\n");
		return 0;
	}

	int next = fdb->header.next_block;
	DirectoryBlock db;

	while (next != -1){																				//to check if file in another block of the directory
		ret = DiskDriver_readBlock(disk, &db, next);												//read new directory block
			if (ret == -1){
				printf("%sImpossible to read all direcory, problems on next block\n%s", COL_RED, COL_GRAY);
				return -1;
			}
																					
		pos = checkDirEsistence(disk, DB_space, db.file_blocks, dirname);							//funcion to check if the directory is in the FirstDirectoryBlock
		if (pos >= 0){
			DiskDriver_readBlock(disk, to_check, db.file_blocks[pos]);								//read again the correct file to complete handler
			d->pos_in_block = 0;																	//reset directory
			//d->pos_in_dir = 0;
			d->directory = fdb;																		//parent directory become this director
			d->dcb = to_check; 																		//this directory become the read directory
			//d->current_block = &(to_check->header);
			//printf("directory changed\n");
			return 0;
		}
		next = db.header.next_block;																//goto next directoryBlock
	}

	printf("%sImpossible to change directory, it doesn't exist\n%s", COL_RED, COL_GRAY);
	return -1;
}



// creates a new directory in the current one (stored in fs->current_directory_block)
// 0 on success
// -1 on error
int SimpleFS_mkDir(DirectoryHandle* d, char* dirname){
	if (d == NULL || dirname == NULL)
		ERROR_HELPER(-1, "Impossible to create directory: Bad Parameters\n");
	
	int ret = 0;
	FirstDirectoryBlock *fdb = d->dcb;																//save pointers, used for performance
	DiskDriver* disk = d->sfs->disk;

	if (fdb->num_entries > 0){																		//directory not empty, needs to check if file already exist
		//printf("check if file already exist\n");
		if (checkDirEsistence(disk, FDB_space, fdb->file_blocks, dirname) != -1){					//funcion to check if the file is in the FirstDirectoryBlock
			printf("%sImpossible to create directory: file already exist\n%s", COL_RED, COL_GRAY);
			return -1;																				//if yes return null
		}

		int next = fdb->header.next_block;
		DirectoryBlock db;

		while (next != -1){																			//to check if file in another block of the directory
			//printf("file is not in fdb, check other block\n");
			ret = DiskDriver_readBlock(disk, &db, next);											//read new directory block
			if (ret == -1){
				printf("%sImpossible to create directory: problem on readBlock to read next block and find already made same file\n%s", COL_RED, COL_GRAY);
				return -1;
			}

			if (checkDirEsistence(disk, DB_space, db.file_blocks, dirname) != -1){					//to check if the file is in the DirectoryBlock
				printf("%sImpossible to create directory: file already exist\n%s", COL_RED, COL_GRAY);
				return -1;
			}
			next = db.header.next_block;															//goto next directoryBlock
		}
	}

	int new_block = DiskDriver_getFreeBlock(disk, disk->header->first_free_block);					//search a free block in the disk for the directory
	if (new_block == -1){
		printf("%sImpossible to create directory: impossible to find free block\n%s", COL_RED, COL_GRAY);
		return -1;
	}
																									//create new directory
	FirstDirectoryBlock * new_directory = calloc(1, sizeof(FirstFileBlock));						//use calloc to put all 0 on the directory
	new_directory->header.block_in_file = 0;														//populate header of the directory
	new_directory->header.next_block = -1;
	new_directory->header.previous_block = -1;

	new_directory->fcb.directory_block = fdb->fcb.block_in_disk;									//populate FileControlBlock
	new_directory->fcb.block_in_disk = new_block;
	//new_directory->fcb.size_in_bytes = BLOCK_SIZE;
	//new_directory->fcb.size_in_blocks = 1;
	new_directory->fcb.is_dir = 1;
	strcpy(new_directory->fcb.name, dirname);

	ret = DiskDriver_writeBlock(disk, new_directory, new_block);									//write directory on the disk
	if (ret == -1){
		printf("%sImpossible to create directory: problem on writeBlock to write directory on disk\n%s", COL_RED, COL_GRAY);
		return -1;
	}
																									//save changes on directory
	int i = 0;
	int found = 0;																					//to check if there is space in already created blocks of the directory
	int block_number = fdb->fcb.block_in_disk;														//number of the current block inside the disk
	DirectoryBlock db_last;																			//in case of no space in firstDirectoryBlock this will save the current block

	int entry = 0;																					//to save the number of the entry in file_blocks of the directoryBlock / firstDurectoryBlock
	int blockInFile = 0;																			//indicate the number of the block inside the file (directory)
	int first_or_not = 0;																			//if 0 save the next block after fdb, if 1 after db_last 										
    int where_to_save = 0;																			//if 0 there is space in firstDirectoryBlock, if 1 space in another block																	

	if (fdb->num_entries < FDB_space){																//check if free space in FirstDirectoryBlock (implicit if num_entry == 0)
		int* blocks = fdb->file_blocks;
		for(i=0; i<FDB_space; i++){																	//loop to find the position
			if (blocks[i] == 0){																	//free space in fdb->blocks[i]
				found = 1;
				entry = i;
				break;
			}																				
		}
	} else{																							//in case of not space in firstDirectoryBlock
		where_to_save = 1;
		int next = fdb->header.next_block;

		while (next != -1 && !found){																//loop to find position in another DirectoryBlock
			DiskDriver_readBlock(disk, &db_last, next);												//read new directory block
			if (ret == -1){
				printf("%sImpossible to create directory: problem on readBlock to read directory and change status\n%s", COL_RED, COL_GRAY);
				DiskDriver_freeBlock(disk, fdb->fcb.block_in_disk);									//needs to free the block already written because it's imposssible to complete operations
				return -1;
			}
			int* blocks = db_last.file_blocks;
			blockInFile++;
			block_number = next;
			for(i=0; i<DB_space; i++){																//loop to find the position
				if (blocks[i] == 0){
					found = 1;
					entry = i;
					break;
				}		
			}
			first_or_not = 1;
			next = db_last.header.next_block;
		}
	}

	if (!found){																					//in case of all blocks of the directory are already full
		DirectoryBlock new_db = {0};																//create new directoryBlock
		new_db.header.next_block = -1;
		new_db.header.block_in_file = blockInFile;
		new_db.header.previous_block = block_number;
		new_db.file_blocks[0] = new_block;

		int new_dir_block = DiskDriver_getFreeBlock(disk, disk->header->first_free_block);			//search a free block in the disk
		if (new_block == -1){
			printf("%sImpossible to create directory: impossible to find free block to create a new block for directory\n%s", COL_RED, COL_GRAY);
			DiskDriver_freeBlock(disk, fdb->fcb.block_in_disk);										//needs to free the block already written because it's imposssible to complete operations
			return -1;
		}

		ret = DiskDriver_writeBlock(disk, &new_db, new_dir_block);									//write block on the disk
		if (ret == -1){
			printf("%sImpossible to create directory: problem on writeBlock to write directory on disk\n%s", COL_RED, COL_GRAY);
			DiskDriver_freeBlock(disk, fdb->fcb.block_in_disk);										//needs to free the block already written because it's imposssible to complete operations
			return -1;
		}

		if (first_or_not == 0){
			fdb->header.next_block = new_dir_block;													//update header currente block
		} else{
			db_last.header.next_block = new_dir_block;
		}
		db_last = new_db;
		block_number = new_dir_block;

	}

	if (where_to_save == 0){																		//space in firstDirectoryBlock
		fdb->num_entries++;	
		fdb->file_blocks[entry] = new_block;														//save new_block position in firstDirectoryBlock
		DiskDriver_updateBlock(disk, fdb, fdb->fcb.block_in_disk);
	} else{
		fdb->num_entries++;	
		DiskDriver_updateBlock(disk, fdb, fdb->fcb.block_in_disk);
		db_last.file_blocks[entry] = new_block;
		DiskDriver_updateBlock(disk, &db_last, block_number);
	}

	return 0;
}


// removes the file in the current directory
// returns -1 on failure 0 on success
// if a directory, it removes recursively all contained files
int SimpleFS_remove(DirectoryHandle* d, char* filename){
	if (d == NULL || filename == NULL)
		ERROR_HELPER(-1, "Impossible to remove directory: Bad Parameters\n");
	
	FirstDirectoryBlock* fdb = d->dcb;																		
	
	int id = checkFileEsistence(d->sfs->disk, FDB_space, fdb->file_blocks, filename);
	int first = 1;
							
	
	DirectoryBlock* db_tmp = (DirectoryBlock*) malloc(sizeof(DirectoryBlock));								//auxiliary structure 
	int next_block = fdb->header.next_block;																//variable with next block of fdb 
	int block_in_disk = fdb->fcb.block_in_disk;
		
	while(id == -1){																						//if file isn't in fdb contiunue research
		if(next_block != -1){
			first = 0;																						//on the other blocks of current directory
			if(DiskDriver_readBlock(d->sfs->disk, db_tmp, next_block) == -1) return -1;
			id = checkFileEsistence(d->sfs->disk, DB_space, db_tmp->file_blocks, filename);
			block_in_disk = next_block;
			next_block = db_tmp->header.next_block;
		}
		else{																								//if blocks of current directory are terminated there is an error
			printf("%sINVALID FILENAME: file not existent or is a directory\n%s", COL_RED, COL_GRAY);
			return -1;
		}		
	}
	
	int idf;
	int ret;
	if(first == 0) idf = db_tmp->file_blocks[id];
	else idf = fdb->file_blocks[id];
	
	FirstFileBlock ffb_rm;
	if(DiskDriver_readBlock(d->sfs->disk, &ffb_rm, idf) == -1) return -1;
	if(ffb_rm.fcb.is_dir == 0){																				//if filename is a file we free its block
		FileBlock fb_tmp;
		int next = ffb_rm.header.next_block;
		int block_in_disk = idf;
		while(next != -1){																					
			if(DiskDriver_readBlock(d->sfs->disk, &fb_tmp, next) == -1) return -1;
			block_in_disk = next;
			next = fb_tmp.header.next_block;
			DiskDriver_ExtremeFreeBlock(d->sfs->disk, block_in_disk);
		}
		DiskDriver_ExtremeFreeBlock(d->sfs->disk, idf);
		d->dcb = fdb;
		ret = 0;
	}
	else{																									//else we recursively free all contained blocks
		FirstDirectoryBlock fdb_rm;
		if(DiskDriver_readBlock(d->sfs->disk, &fdb_rm, idf) == -1) return -1;
		if(fdb_rm.num_entries > 0){
			if(SimpleFS_changeDir(d, fdb_rm.fcb.name) == -1) return -1;
			int i;
			for(i = 0; i < FDB_space; i++){
				FirstFileBlock ffb;
				if(fdb_rm.file_blocks[i] > 0 && DiskDriver_readBlock(d->sfs->disk, &ffb, fdb_rm.file_blocks[i]) != -1)
					SimpleFS_remove(d, ffb.fcb.name);
			}
			int next = fdb_rm.header.next_block;
			int block_in_disk = idf;
			DirectoryBlock db_tmp;
			while(next != -1){																					
				if(DiskDriver_readBlock(d->sfs->disk, &db_tmp, next) == -1) return -1;
				int j;
				for(j = 0; j < DB_space; j++){
					FirstFileBlock ffb;
					if(DiskDriver_readBlock(d->sfs->disk, &ffb, db_tmp.file_blocks[i]) == -1) return -1;
					SimpleFS_remove(d, ffb.fcb.name);
				}
				block_in_disk = next;
				next = db_tmp.header.next_block;
				DiskDriver_ExtremeFreeBlock(d->sfs->disk, block_in_disk);
			}
			DiskDriver_ExtremeFreeBlock(d->sfs->disk, idf);
			d->dcb = fdb;
			ret = 0;
		}
		else{
			DiskDriver_ExtremeFreeBlock(d->sfs->disk, idf);
			d->dcb = fdb;
			ret = 0;
		}
	}
	
	if(first == 0){
		db_tmp->file_blocks[id] = -1;
		fdb->num_entries-=1;
		DiskDriver_updateBlock(d->sfs->disk, db_tmp, block_in_disk);
		DiskDriver_updateBlock(d->sfs->disk, fdb, fdb->fcb.block_in_disk);
		return ret;
	}
	else{
		fdb->file_blocks[id] = -1;
		fdb->num_entries-=1;
		DiskDriver_updateBlock(d->sfs->disk, fdb, fdb->fcb.block_in_disk);
		return ret;
	}
	
	return -1;
}
