#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bitmap.h"

int findBit(unsigned char c, int n) {
    static unsigned char mask2[] = {128, 64, 32, 16, 8, 4, 2, 1};
    return ((c & mask2[n]) != 0);
}

int main(int argc, char** argv){
	if(argc < 2){
		printf("usage: program 1<parametre>\n");
		printf("parametre can be: b2i , i2b  ,  get  ,  set\n");
		
		return 0;
	}
	
/************* TEST FUNCTION bitmap_blockToIndex ***********************/
	if(strcmp(argv[1], "b2i") == 0){
	
		printf("\ntest bitmap_blockToIndex\n");

		BitMapEntryKey b1= BitMap_blockToIndex(50);
		BitMapEntryKey b2= BitMap_blockToIndex(52);
		BitMapEntryKey b3= BitMap_blockToIndex(86);
		BitMapEntryKey b4= BitMap_blockToIndex(33);
		BitMapEntryKey b5= BitMap_blockToIndex(1);
		BitMapEntryKey b6= BitMap_blockToIndex(0);

		printf("50 => %d %d\n", b1.entry_num, b1.bit_num);
		printf("52 => %d %d\n", b2.entry_num, b2.bit_num);
		printf("86 => %d %d\n", b3.entry_num, b3.bit_num);
		printf("33 => %d %d\n", b4.entry_num, b4.bit_num);
		printf("1 => %d %d\n", b5.entry_num, b5.bit_num);
		printf("0 => %d %d\n", b6.entry_num, b6.bit_num);
		
	}

/************************************************************************/


/************* TEST FUNCTION bitmap_indexToBlock ***********************/ 
	
	else if(strcmp(argv[1], "i2b") == 0){

		printf("\n\ntest bitmap_indexToBlock\n");

		int n1 = BitMap_indexToBlock( 6, 2);
		int n2 = BitMap_indexToBlock( 6, 4);
		int n3 = BitMap_indexToBlock( 10, 6);
		int n4 = BitMap_indexToBlock(4, 1);
		int n5 = BitMap_indexToBlock(0, 1);
		int n6 = BitMap_indexToBlock(0, 0);

		printf("6, 2 => %d\n", n1);
		printf("6, 4 => %d\n", n2);
		printf("10, 6 => %d\n", n3);
		printf("4, 1 => %d\n", n4);
		printf("0, 1 => %d\n", n5);
		printf("0, 0 => %d\n", n6);
		
	}

/************************************************************************/

/********************************************* TEST FUNCTION BitMap_get *********************************/ 

	else if(strcmp(argv[1], "get") == 0){
		printf("\n\ntest BitMap_get ATTENTION bits are inverted\n");
		int check, bit;

		printf("\nbmap1 num_bits=8 => 11111111 (255) \n");
		BitMap *bmap1 = (BitMap*) malloc(sizeof (BitMap));
		bmap1->num_bits = 8;
		bmap1->entries = (char*) malloc(sizeof(char));
		bmap1->entries[0] = 255;
		
		printf("\nbmap1.2 num_bits=8 => 10010110 (150) \n");
		BitMap *bmap12 = (BitMap*) malloc(sizeof (BitMap));
		bmap12->num_bits = 8;
		bmap12->entries = (char*) malloc(sizeof(char));
		bmap12->entries[0] = 150;
	
		printf("\nbmap1.3 num_bits=8 => 11001011 (203) \n");
		BitMap *bmap13 = (BitMap*) malloc(sizeof (BitMap));
		bmap13->num_bits = 8;
		bmap13->entries = (char*) malloc(sizeof(char));
		bmap13->entries[0] = 203;


		printf("\nbmap2 num_bits=32 => 11111111 11111111 00000000 10010110 \n");
		BitMap *bmap2 = (BitMap*) malloc(sizeof (BitMap));
		bmap2->num_bits = 32;
		bmap2->entries = (char*) malloc(sizeof(char)*4);
		bmap2->entries[0] = 255;
		bmap2->entries[1] = 255;
		bmap2->entries[2] = 0;
		bmap2->entries[3] = 150;
    
		printf("\nbmap3 num_bits=32 => 11111111 11111111 00000000 10010000\n");
		BitMap *bmap3 = (BitMap*) malloc(sizeof (BitMap));
		bmap3->num_bits = 22;
		bmap3->entries = (char*) malloc(sizeof(char)*4);
		bmap3->entries[0] = 255;
		bmap3->entries[1] = 255;
		bmap3->entries[2] = 0;
		bmap3->entries[3] = 144;	
    
    
		printf("\nbmap4 num_bits=8 => 10010000 (144) \n");
		BitMap *bmap4 = (BitMap*) malloc(sizeof (BitMap));
		bmap4->num_bits = 8;
		bmap4->entries = (char*) malloc(sizeof(char));
		bmap4->entries[0] = 144;
	
	

		printf("\ntest bmap1 => 11111111\n check = "); 
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap1->entries), check);
            printf("%d ", bit);
        }
		printf("\n"); 

		int get1_1 = BitMap_get(bmap1, 0, 0);
		int get1_2 = BitMap_get(bmap1, 2, 0);
		int get1_3 = BitMap_get(bmap1, 0, 1);
		int get1_4 = BitMap_get(bmap1, 3, 1);
		int get1_5 = BitMap_get(bmap1, 9, 1);

		printf("\ntest bmap1:start = 0, status = 0  => %d [correct = -1]\n", get1_1);
		printf("test bmap1:start = 2, status = 0  => %d [correct = -1]\n", get1_2);
		printf("test bmap1:start = 0, status = 1  => %d [correct = 0]\n", get1_3);
		printf("test bmap1:start = 3, status = 1  => %d [correct = 3]\n", get1_4);
		printf("test bmap1:start = 9, status = 1  => %d [correct = -1]\n\n\n", get1_5);
		
		free(bmap1->entries);
		free(bmap1);



		printf("\ntest bmap1.2 => 10010110\n check = "); 
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap12->entries), check);
            printf("%d ", bit);
        }
		printf("\n"); 

		int get12_1 = BitMap_get(bmap12, 0, 0);
		int get12_2 = BitMap_get(bmap12, 4, 0);
		int get12_3 = BitMap_get(bmap12, 0, 1);
		int get12_4 = BitMap_get(bmap12, 3, 1);
	
		printf("\ntest bmap1.2:start = 0, status = 0  => %d [correct = 0]\n", get12_1);
		printf("test bmap1.2:start = 4, status = 0  => %d [correct = 5]\n", get12_2);
		printf("test bmap1.2:start = 0, status = 1  => %d [correct = 1]\n", get12_3);
		printf("test bmap1.2:start = 3, status = 1  => %d [correct = 4]\n\n\n", get12_4);

		free(bmap12->entries);
		free(bmap12);

		printf("\ntest bmap1.3 => 11001011\n check = "); 
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap13->entries), check);
            printf("%d ", bit);
        }
		printf("\n");

		int get13_1 = BitMap_get(bmap13, 0, 0);
		int get13_2 = BitMap_get(bmap13, 4, 0);
		int get13_3 = BitMap_get(bmap13, 0, 1);
		int get13_4 = BitMap_get(bmap13, 3, 1);

		printf("\ntest bmap1.3:start = 0, status = 0  => %d [correct = 2]\n", get13_1);
		printf("test bmap1.3:start = 4, status = 0  => %d [correct = 4]\n", get13_2);
		printf("test bmap1.3:start = 0, status = 1  => %d [correct = 0]\n", get13_3);
		printf("test bmap1.3:start = 3, status = 1  => %d [correct = 3]\n\n\n", get13_4);
		
		free(bmap13->entries);
		free(bmap13);
		
		printf("\ntest bmap2 => 11111111 11111111 00000000 10010110\n check = "); 
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap2->entries), check);
            printf("%d ", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap2->entries[1], check);
            printf("%d ", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap2->entries[2], check);
            printf("%d ", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap2->entries[3], check);
            printf("%d ", bit);
        }
		printf("\n");

		int get2_1 = BitMap_get(bmap2, 16, 0);
		int get2_2 = BitMap_get(bmap2, 9, 0);
		int get2_3 = BitMap_get(bmap2, 0, 1);
		int get2_4 = BitMap_get(bmap2, 3, 1);

		int get2_5 = BitMap_get(bmap2, 9, 0);
		int get2_6 = BitMap_get(bmap2, 11, 0);
		int get2_7 = BitMap_get(bmap2, 9, 1);
		int get2_8 = BitMap_get(bmap2, 11, 1);

		int get2_9 = BitMap_get(bmap2, 17, 0);
		int get2_10 = BitMap_get(bmap2, 22, 0);
		int get2_11 = BitMap_get(bmap2, 17, 1);
		int get2_12  = BitMap_get(bmap2, 22, 1);

		printf("\ntest bmap2:start = 16, status = 0  => %d [correct = 16]\n", get2_1);
		printf("test bmap2:start = 9, status = 0  => %d [correct = 16]\n", get2_2);
		printf("test bmap2:start = 0, status = 1  => %d [correct = 0]\n", get2_3);
		printf("test bmap2.:start = 3, status = 1  => %d [correct = 3]\n\n\n", get2_4);

		printf("\ntest bmap2:start = 9, status = 0  => %d [correct = 16]\n", get2_5);
		printf("test bmap2:start = 11, status = 0  => %d [correct = 16]\n", get2_6);
		printf("test bmap2:start = 9, status = 1  => %d [correct = 9]\n", get2_7);
		printf("test bmap2.:start = 11, status = 1  => %d [correct = 11]\n\n\n", get2_8);

		printf("\ntest bmap2:start = 17, status = 0  => %d [correct = 17]\n", get2_9);
		printf("test bmap2:start = 22, status = 0  => %d [correct = 22]\n", get2_10);
		printf("test bmap2:start = 17, status = 1  => %d [correct = 25]\n", get2_11);
		printf("test bmap2.:start = 22, status = 1  => %d [correct = 25]\n\n\n", get2_12);
		
		free(bmap2->entries);
		free(bmap2);

		printf("\ntest bmap3 => 11111111 11111111 00000000\n check = "); 
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap3->entries), check);
            printf("%d ", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[1], check);
            printf("%d ", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[2], check);
            printf("%d ", bit);
        }
		printf("\n");

		int get3_1 = BitMap_get(bmap3, 24, 0);
		int get3_2 = BitMap_get(bmap3, 25, 0);
		int get3_3 = BitMap_get(bmap3, 17, 1);
		int get3_4 = BitMap_get(bmap3, 21, 1);

		printf("\ntest bmap3 :start = 24, status = 0  => %d [correct = -1]\n", get3_1);
		printf("test bmap3 :start = 25, status = 0  => %d [correct = -1]\n", get3_2);
		printf("test bmap3 :start = 17, status = 1  => %d [correct = -1]\n", get3_3);
		printf("test bmap3 :start = 22, status = 1  => %d [correct = -1]\n\n\n", get3_4);
		
		free(bmap3->entries);
		free(bmap3);

		printf("\ntest bmap4 => 10010000\n check = "); 
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap4->entries), check);
            printf("%d ", bit);
        }
		printf("\n");

		int get4_1 = BitMap_get(bmap4, 0, 0);
		int get4_2 = BitMap_get(bmap4, 3, 0);
		int get4_3 = BitMap_get(bmap4, 0, 1);
		int get4_4 = BitMap_get(bmap4, 3, 1);

		printf("\ntest bmap4 :start = 0, status = 0  => %d [correct = 0]\n", get4_1);
		printf("test bmap4 :start = 3, status = 0  => %d [correct = 3]\n", get4_2);
		printf("test bmap4 :start = 0, status = 1  => %d [correct = 4]\n", get4_3);
		printf("test bmap4 :start = 3, status = 1  => %d [correct = 4]\n\n\n", get4_4);
		
		free(bmap4->entries);
		free(bmap4);
	}
	
/********************************************************************************************************************/


/********************************************* TEST FUNCTION BitMap_set *********************************************/ 
	else if(strcmp(argv[1], "set") == 0){
		printf("\n\ntest BitMap_set ATTENTION bits are inverted\n");
		int check, bit;

		printf("\nbmap1 num_bits=8 => 11111111 (255) \n");
		BitMap *bmap1 = (BitMap*) malloc(sizeof (BitMap));
		bmap1->num_bits = 8;
		bmap1->entries = (char*) malloc(sizeof(char));
		bmap1->entries[0] = 255;	
		
		printf("\nbmap3 num_bits=32 => 11111111 11111111 00000000 10010000 \n");
		BitMap *bmap3 = (BitMap*) malloc(sizeof (BitMap));
		bmap3->num_bits = 32;
		bmap3->entries = (char*) malloc(sizeof(char)*4);
		bmap3->entries[0] = 255;
		bmap3->entries[1] = 255;
		bmap3->entries[2] = 0;
		bmap3->entries[3] = 144;	
		
		


		printf("\ntest bmap1 => 11111111\n check = "); 
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap1->entries), check);
            printf("%d ", bit);
        }
		printf("\n");

		
		int set1_1 = BitMap_set(bmap1, 4, 0);
		printf("\ntest bmap1: pos = 4, status = 0  => %d [", set1_1);
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap1->entries), check);
            printf("%d", bit);
        }
		printf("] (correct = 239)\n");


		int set1_2 = BitMap_set(bmap1, 1, 0);
		printf("test bmap1: pos = 1, status = 0  => %d [", set1_2);
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap1->entries), check);
            printf("%d", bit);
        }
		printf("] (correct = 237)\n");


		int set1_3 = BitMap_set(bmap1, 4, 1);
		printf("test bmap1: pos = 4, status = 1  => %d [", set1_3);
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap1->entries), check);
            printf("%d", bit);
        }
		printf("] (correct = 253)\n");


		int set1_4 = BitMap_set(bmap1, 1, 0);
		printf("test bmap1: pos = 1, status = 0  => %d [", set1_2);
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap1->entries), check);
            printf("%d", bit);
        }
		printf("] (correct = 237)\n");




		printf("\ntest bmap3 => 11111111 11111111 00000000 10010000\n check = "); 
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap3->entries), check);
            printf("%d ", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[1], check);
            printf("%d ", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[2], check);
            printf("%d ", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[3], check);
            printf("%d ", bit);
        }
		printf("\n");

	

		int set3_1 = BitMap_set(bmap3, 17, 1); 
		printf("\ntest bmap3: pos = 17, status = 1  => %d [", set3_1);
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap3->entries), check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[1], check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[2], check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[3], check);
            printf("%d", bit);
        }
		printf("] (correct = 2) \n");



		int set3_2 = BitMap_set(bmap3, 22, 1);
		printf("test bmap3: pos = 22, status = 1  => %d [", set3_2);
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap3->entries), check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[1], check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[2], check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[3], check);
            printf("%d", bit);
        }
		printf("] (correct = 66) \n");



		int set3_3 = BitMap_set(bmap3, 17, 0); 
		printf("test bmap3: pos = 17, status = 0  => %d [", set3_3);
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap3->entries), check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[1], check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[2], check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[3], check);
            printf("%d", bit);
        }
		printf("] (correct = 64) \n"); 



		int set3_4 = BitMap_set(bmap3, 22, 0);  
		printf("test bmap3: pos = 22, status = 0  => %d [", set3_4);
		for (check = 0; check < 8; check ++){
            bit = findBit(*(bmap3->entries), check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[1], check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[2], check);
            printf("%d", bit);
        }
		printf("  ");
		for (check = 0; check < 8; check ++){
            bit = findBit(bmap3->entries[3], check);
            printf("%d", bit);
        }
		printf("] (correct = 0) \n");
		
		free(bmap1->entries);
		free(bmap1);
		free(bmap3->entries);
		free(bmap3);
	}
	else printf("Invalid Parametrer\n");
	return 0;
}
