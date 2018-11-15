#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "bitmap.h"



//returns the value of the bit in the byte
int BitMap_getBit(BitMap* bmap, int pos){
    if(pos >= bmap->num_bits) return -1;                                //error, start not in range of bitmap blocks
    
    BitMapEntryKey map = BitMap_blockToIndex(pos);                      //convert to easy use shift
    return bmap->entries[map.entry_num] >> map.bit_num & mask;
}

// converts a block index to an index in the array,
// and a char that indicates the offset of the bit inside the array
BitMapEntryKey BitMap_blockToIndex(int num){
    BitMapEntryKey map;
    int byte = num / bit_in_byte;                                       //find the byte where is the bit
    map.entry_num = byte;
    char offset = num - (byte * bit_in_byte);                           //find offset of bit in byte found
    map.bit_num = offset;
    return map;
}


// converts a bit to a linear index
// return -1 if invalid paramaters
int BitMap_indexToBlock(int entry, char bit_num){
    if (entry < 0 || bit_num < 0) return -1;                            //error, block not in range of bitmap blocks
    return (entry * bit_in_byte) + bit_num;
}


// returns the index of the first bit having status "status"
// in the bitmap bmap, and starts looking from position start
// return -1 if can't find or staus bigger than num_bits
// starts by 0
int BitMap_get(BitMap* bmap, int start, int status){
    if (start > bmap->num_bits) return -1;                              //error, start not in range of bitmap blocks
    
    while(start < bmap->num_bits){                                      //loop to search status on bitmap
        if(BitMap_getBit(bmap, start) == status) return start;
        start++;
    }
    return -1;
    
}



// sets the bit at index pos in bmap to status
int BitMap_set(BitMap* bmap, int pos, int status){
    if(pos >= bmap->num_bits) return -1;                                //error, start not in range of bitmap blocks
    
    BitMapEntryKey map = BitMap_blockToIndex(pos);
    unsigned char for_change = 1 << map.bit_num;                        //create mask for the bit in the byte
    unsigned char to_change = bmap->entries[map.entry_num];             //byte where to use mask
    
    if(status == 1){
        bmap->entries[map.entry_num] = to_change | for_change;          //mask by OR to have 1
        return to_change | for_change;
    }
    else{
        bmap->entries[map.entry_num] = to_change & (~for_change);       //mask by AND to have 0 (with !mask)
        return to_change & (~for_change);
    }
}
