#pragma once
#include "error.h"
#include <stdint.h>

typedef struct{
  int num_bits;			// num bits (blocks) bitmap
  char* entries;		// bitmap 
}  BitMap;

typedef struct {
  int entry_num;		// num byte
  char bit_num;			// offset bit in byte
} BitMapEntryKey;

#define bit_in_byte 8 //number of bit in byte
#define mask 0x01 //mask for one bit

//returns the value of the bit in position pos
int BitMap_getBit(BitMap* bmap, int pos);

// converts a block index to an index in the array,
// and a char that indicates the offset of the bit inside the array
BitMapEntryKey BitMap_blockToIndex(int num);
	
// converts a bit to a linear index
// return -1 if invalid paramaters
int BitMap_indexToBlock(int entry, char bit_num);
	
// returns the index of the first bit having status "status"
// in the bitmap bmap, and starts looking from position start
int BitMap_get(BitMap* bmap, int start, int status);

// sets the bit at index pos in bmap to status
int BitMap_set(BitMap* bmap, int pos, int status);
