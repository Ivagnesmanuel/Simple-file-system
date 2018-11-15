#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include "disk_driver.h"
#include "bitmap.h"
#include "error.h"
#include "simplefs.h"

int main(int argc, char** argv){
	DiskDriver* disk_driver = (DiskDriver*) malloc(sizeof(DiskDriver));
	
	int i;
	const char* filename = "./disk_file.txt";
	
	BlockHeader header;
	header.previous_block = 2;
	header.next_block = 2;
	header.block_in_file = 2;
	
	FileBlock* fb1 = malloc(sizeof(FileBlock));
	fb1->header = header;
	char data_block1[BLOCK_SIZE-sizeof(BlockHeader)];
	for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader); i++) data_block1[i] = '1';
	data_block1[BLOCK_SIZE-sizeof(BlockHeader)-1] = '\0';
	strcpy(fb1->data, data_block1);
	
	FileBlock* fb2 = malloc(sizeof(FileBlock));
	fb2->header = header;
	char data_block2[BLOCK_SIZE-sizeof(BlockHeader)];
	for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader); i++) data_block2[i] = '2';
	data_block2[BLOCK_SIZE-sizeof(BlockHeader)-1] = '\0';
	strcpy(fb2->data, data_block2);
	
	FileBlock* fb3 = malloc(sizeof(FileBlock));
	fb3->header = header;
	char data_block3[BLOCK_SIZE-sizeof(BlockHeader)];
	for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader); i++) data_block3[i] = '3';
	data_block3[BLOCK_SIZE-sizeof(BlockHeader)-1] = '\0';
	strcpy(fb3->data, data_block3);
	
	FileBlock* fb4 = malloc(sizeof(FileBlock));
	fb4->header = header;
	char data_block4[BLOCK_SIZE-sizeof(BlockHeader)];
	for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader); i++) data_block4[i] = '4';
	data_block4[BLOCK_SIZE-sizeof(BlockHeader)-1] = '\0';
	strcpy(fb4->data, data_block4);
	
	FileBlock* fb5 = malloc(sizeof(FileBlock));
	fb5->header = header;
	char data_block5[BLOCK_SIZE-sizeof(BlockHeader)];
	for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader); i++) data_block5[i] = '5';
	data_block5[BLOCK_SIZE-sizeof(BlockHeader)-1] = '\0';
	strcpy(fb5->data, data_block5);
	
	printf("INIT disk with 5 blocks\n\n");
	DiskDriver_init(disk_driver, filename, 5);
	DiskDriver_flush(disk_driver);
	
	
	printf("------------------TEST FUNCTION WRITE and FUNCTION GET FREE BLOCK---------------\n\n");
	
	printf("WRITE block 0\n");
	DiskDriver_writeBlock(disk_driver, fb1, 0);
	DiskDriver_flush(disk_driver);
	printf("free blocks:%d\n", disk_driver->header->free_blocks);
	printf("first free block:%d\n", disk_driver->header->first_free_block);
	printf("bitmap:%d\n\n", disk_driver->bitmap_data[0]);
	
	printf("WRITE block 1\n");
	DiskDriver_writeBlock(disk_driver, fb2, DiskDriver_getFreeBlock(disk_driver, 0));
	DiskDriver_flush(disk_driver);
	printf("free blocks:%d\n", disk_driver->header->free_blocks);
	printf("first free block:%d\n", disk_driver->header->first_free_block);
	printf("bitmap:%d\n\n", disk_driver->bitmap_data[0]);
	
	printf("WRITE block 2\n");
	DiskDriver_writeBlock(disk_driver, fb3, DiskDriver_getFreeBlock(disk_driver, 1));
	DiskDriver_flush(disk_driver);
	printf("free blocks:%d\n", disk_driver->header->free_blocks);
	printf("first free block:%d\n", disk_driver->header->first_free_block);
	printf("bitmap:%d\n\n", disk_driver->bitmap_data[0]);
	
	printf("WRITE block 3\n");
	DiskDriver_writeBlock(disk_driver, fb4, DiskDriver_getFreeBlock(disk_driver, 2));
	DiskDriver_flush(disk_driver);
	printf("free blocks:%d\n", disk_driver->header->free_blocks);
	printf("first free block:%d\n", disk_driver->header->first_free_block);
	printf("bitmap:%d\n\n", disk_driver->bitmap_data[0]);
	
	printf("WRITE block 4\n");
	DiskDriver_writeBlock(disk_driver, fb5, DiskDriver_getFreeBlock(disk_driver, 3));
	DiskDriver_flush(disk_driver);	
	printf("free blocks:%d\n", disk_driver->header->free_blocks);
	printf("first free block:%d\n", disk_driver->header->first_free_block);
	printf("bitmap:%d\n\n", disk_driver->bitmap_data[0]);
		
	printf("TO SEE CHANGES ON FILE: cat disk_file.txt \n\n");
	
	printf("---------------------------TEST FUNCTION READ-------------------------------------\n\n");
	FileBlock* fb_test = malloc(sizeof(FileBlock));
	printf("Read Block 0:\n");
	DiskDriver_readBlock(disk_driver, fb_test, 0);
	printf("%s\n", fb_test->data);
	printf("Read Block 1:\n");
	DiskDriver_readBlock(disk_driver, fb_test, 1);
	printf("%s\n", fb_test->data);
	printf("Read Block 2:\n");
	DiskDriver_readBlock(disk_driver, fb_test, 2);
	printf("%s\n", fb_test->data);
	printf("Read Block 3:\n");
	DiskDriver_readBlock(disk_driver, fb_test, 3);
	printf("%s\n", fb_test->data);
	printf("Read Block 4:\n");
	DiskDriver_readBlock(disk_driver, fb_test, 4);
	printf("%s\n\n", fb_test->data);
	
	
	printf("---------------------------TEST FUNCTION FREE-------------------------------------\n\n");
	printf("Free Block 0\n");
	DiskDriver_freeBlock(disk_driver, 0);
	printf("Try to read block 0:\n");
	DiskDriver_readBlock(disk_driver, fb_test, 0);
	printf("free blocks:%d\n", disk_driver->header->free_blocks);
	printf("first free block:%d\n", disk_driver->header->first_free_block);
	printf("bitmap:%d\n\n", disk_driver->bitmap_data[0]);
	
	printf("Free Block 1\n");
	DiskDriver_freeBlock(disk_driver, 1);
	printf("Try to read block 1:\n");
	DiskDriver_readBlock(disk_driver, fb_test, 1);
	printf("free blocks:%d\n", disk_driver->header->free_blocks);
	printf("first free block:%d\n", disk_driver->header->first_free_block);
	printf("bitmap:%d\n\n", disk_driver->bitmap_data[0]);
	
	printf("Free Block 2\n");
	DiskDriver_freeBlock(disk_driver, 2);
	printf("Try to read block 2:\n");
	DiskDriver_readBlock(disk_driver, fb_test, 2);
	printf("free blocks:%d\n", disk_driver->header->free_blocks);
	printf("first free block:%d\n", disk_driver->header->first_free_block);
	printf("bitmap:%d\n\n", disk_driver->bitmap_data[0]);
	
	printf("Free Block 3\n");
	DiskDriver_freeBlock(disk_driver, 3);
	printf("Try to read block 3:\n");
	DiskDriver_readBlock(disk_driver, fb_test, 3);
	printf("free blocks:%d\n", disk_driver->header->free_blocks);
	printf("first free block:%d\n", disk_driver->header->first_free_block);
	printf("bitmap:%d\n\n", disk_driver->bitmap_data[0]);
	
	printf("Free Block 4\n");
	DiskDriver_freeBlock(disk_driver, 4);
	printf("Try to read block 4:\n");
	DiskDriver_readBlock(disk_driver, fb_test, 4);
	DiskDriver_flush(disk_driver);
	printf("free blocks:%d\n", disk_driver->header->free_blocks);
	printf("first free block:%d\n", disk_driver->header->first_free_block);
	printf("bitmap:%d\n\n", disk_driver->bitmap_data[0]);
	
	free(disk_driver);
	free(fb_test);
	free(fb1);
	free(fb2);
	free(fb3);
	free(fb4);
	free(fb5);
	
	
	printf("\nNOW ALL BLOCKS ARE FREE, IF YOU CHECK THE FILE YOU CAN SEE THAT IN THE FIRST PART WE HAVE DISK_HEADER STRUCTURE\n\n");
	printf("TO SEE CHANGES ON FILE: cat disk_file.txt \n\n");
	return 0;
}
