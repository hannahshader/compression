/******************************************************************************
*       word_unpacked.c
*       By: Kalyn (kmuhle01) and Hannah (hshade01)
*       3/8/2023
*
*       Comp40 Project 4: arith
*   
*       This file contains the functions necessary to convert an image in the
*       form of an unpacked_pixmap to a word_pixmap (compression), and back 
*       from a word_pixmap to an unpacked_pixmap (decompression). 
*   
******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <a2methods.h>
#include "assert.h"
#include "a2blocked.h"
#include "uarray2b.h"
#include "unpacked_cv.h"
#include "bitpack.h"
#include "word_unpacked.h"


#define COMPRESSED_BLOCK_SIZE 1
#define MAX_BITS 32
#define LSB_A 23
#define LSB_B 18
#define LSB_C 13
#define LSB_D 8
#define LSB_PB 4
#define LSB_PR 0

/******** COMPRESSION HELPER FUNCTIONS ********/
void unpacked_to_word_mapping(int i, int j, A2Methods_UArray2 array2, 
                A2Methods_Object *word, void *pixmap);
void unpacked_to_word(uint32_t* curr_word, unpacked_t curr_unpacked);

/******** DECOMPRESSION HELPER FUNCTIONS ********/
void word_to_unpacked_mapping(int i, int j, A2Methods_UArray2 array2, 
                A2Methods_Object *unpacked, void *pixmap);
void word_to_unpacked(unpacked_t curr_unpacked, uint32_t* curr_word);


/************ COMPRESSION ************/

/*
*       Description: A function that takes a struct unpacked_pixmap containing
*       a 2D array of structs unpacked_t and creates an associated struct
*       word_pixmap containing a 2D array of unint32_ts. 
*
*   
*       In/Out Expectations: Expects a struct type unpacked_pixmap. Mallocs
*       memory for a struct word_pixmap that the client must eventually free.
*       Returns this created type of word_pixmap.  
*/
word_pixmap unpacked_to_word_pixmap(unpacked_pixmap old_unpacked_pixmap) {
        assert(old_unpacked_pixmap != NULL);

        int width = old_unpacked_pixmap->width;
        int height = old_unpacked_pixmap->height;

        word_pixmap new_word_pixmap = malloc(sizeof(*new_word_pixmap));
        assert(new_word_pixmap != NULL);

        new_word_pixmap->width = width;
        new_word_pixmap->height = height;
        new_word_pixmap->methods = uarray2_methods_blocked;
        
        uint32_t dummy_uint32;
        A2Methods_UArray2 pixmap = new_word_pixmap->methods->
                new_with_blocksize(width, height, sizeof(dummy_uint32),    
                COMPRESSED_BLOCK_SIZE);
        new_word_pixmap->pixels = pixmap;
        
        new_word_pixmap->methods->map_block_major(new_word_pixmap->pixels, 
                unpacked_to_word_mapping, old_unpacked_pixmap);

        return new_word_pixmap;
}

/*
*       Description: A function that converts a unpacked_t element of an 
*       unpacked_pixmap to an associated unint32_t, and places it in 
*       the associated element of a word_pixmap. 
*
*       In/Out Expectations: expects to take in an uninitialized uint64_t 
*       element, the row and col where it lies the word_pixmap, and a 
*       pointer to a unpacked_pixmap. Assigns value to the unint64_t from
*       the unpacked_pixmap with a helper function. Returns void. 
*/
void unpacked_to_word_mapping(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *word, void *pixmap) {
        assert(pixmap != NULL);

        unpacked_pixmap old_unpacked_pixmap = (unpacked_pixmap)pixmap;
        unpacked_t curr_unpacked = old_unpacked_pixmap->methods->at
                (old_unpacked_pixmap->pixels, i, j);

        uint32_t *curr_word = (uint32_t*)word;

        unpacked_to_word(curr_word, curr_unpacked);

        (void)array2;
}

/*
*       Description: Generates a word (uint32_t) from a type unpacked_t.
*
*       In/Out Expectations: expects to take a uint32_t, passed by 
*       reference, and an instance of struct unpacked_t. Sets values of
*       the uint32_t from the unpacked_t. Returns void.
*/
void unpacked_to_word(uint32_t* curr_word, unpacked_t curr_unpacked) {
        assert(curr_unpacked != NULL);
        assert(curr_word != NULL);

        *curr_word = 0;
        *curr_word = Bitpack_newu(*curr_word, (MAX_BITS - LSB_A), LSB_A,
                curr_unpacked->a);
        *curr_word = Bitpack_news(*curr_word, (LSB_A - LSB_B), LSB_B,
                curr_unpacked->b);
        *curr_word = Bitpack_news(*curr_word, (LSB_B - LSB_C), LSB_C,
                curr_unpacked->c);
        *curr_word = Bitpack_news(*curr_word, (LSB_C - LSB_D), LSB_D,
                curr_unpacked->d);
        *curr_word = Bitpack_newu(*curr_word, (LSB_D - LSB_PB), LSB_PB,
                curr_unpacked->pb_avg);
        *curr_word = Bitpack_newu(*curr_word, (LSB_PB - LSB_PR), LSB_PR,
                curr_unpacked->pr_avg);
}


/************ DECOMPRESSION ************/

/*
*       Description: A function that takes a struct word_pixmap containing
*       a 2D array of uint32_ts and creates an associated struct
*       unpacked_pixmap containing a 2D array of structs type unpacked_t. 
*
*   
*       In/Out Expectations: Expects a struct type word_pixmap. Mallocs
*       memory for a struct unpacked_pixmap that the client must eventually 
*       free. Returns this created type of unpacked_pixmap.  
*/
unpacked_pixmap word_to_unpacked_pixmap(word_pixmap old_word_pixmap) {
        assert(old_word_pixmap != NULL);

        int width = old_word_pixmap->width;
        int height = old_word_pixmap->height;
        
        unpacked_pixmap new_unpacked_pixmap = 
                        malloc(sizeof(*new_unpacked_pixmap));
        assert(new_unpacked_pixmap != NULL);

        new_unpacked_pixmap->width = width;
        new_unpacked_pixmap->height = height;
        new_unpacked_pixmap->methods = uarray2_methods_blocked;
        
        unpacked_t dummy_unpacked;
        A2Methods_UArray2 pixmap = new_unpacked_pixmap->methods->
                new_with_blocksize(width, height, sizeof(*dummy_unpacked), 
                COMPRESSED_BLOCK_SIZE);
        new_unpacked_pixmap->pixels = pixmap;
        
        new_unpacked_pixmap->methods->map_block_major(new_unpacked_pixmap->
                pixels,word_to_unpacked_mapping, old_word_pixmap);
        
        return new_unpacked_pixmap;
}

/*
*       Description: A function that converts a unint32_t element of an 
*       word_pixmap to an associated unpacked_t, and places it in 
*       the associated element of an unpacked_pixmap. 
*
*       In/Out Expectations: expects to take in an uninitialized unpacked_t 
*       element, the row and col where it lies the unpacked_pixmap, and a 
*       pointer to a word_pixmap. Assigns values from the word_pixmap to 
*       the unpacked_t with a helper function. Returns void. 
*/
void word_to_unpacked_mapping(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *unpacked, void *pixmap) {
        assert(pixmap != NULL);

        word_pixmap old_word_pixmap = (word_pixmap)pixmap;
        uint32_t *curr_word = old_word_pixmap->methods->
                                        at(old_word_pixmap->pixels, i, j);
        unpacked_t curr_unpacked = (unpacked_t)unpacked;

        word_to_unpacked(curr_unpacked, curr_word);

        (void)array2;
}

/*
*       Description: Generates a struct unpacked_t from a word (uint32_t).
*
*       In/Out Expectations: expects to take a uint32_t, passed by 
*       reference, and an instance of struct unpacked_t. Sets values of
*       the struct unpacked_t from the word (uint32_t). Returns void.
*/
void word_to_unpacked(unpacked_t curr_unpacked, uint32_t* curr_word) {
        assert(curr_unpacked != NULL);
        assert(curr_word != NULL);

        curr_unpacked->a = Bitpack_getu(*curr_word, (MAX_BITS - LSB_A), LSB_A);
        curr_unpacked->b = Bitpack_gets(*curr_word, (LSB_A - LSB_B), LSB_B);
        curr_unpacked->c = Bitpack_gets(*curr_word, (LSB_B - LSB_C), LSB_C);
        curr_unpacked->d = Bitpack_gets(*curr_word, (LSB_C - LSB_D), LSB_D);
        curr_unpacked->pb_avg = Bitpack_getu(*curr_word, 
                                        (LSB_D - LSB_PB), LSB_PB);
        curr_unpacked->pr_avg = Bitpack_getu(*curr_word, 
                                        (LSB_PB - LSB_PR), LSB_PR);
}

/*
*       Description: Frees memory associated with a word_pixmap.
*
*       In/Out Expectations: expects to take a word_pixmap that's been
*       mallocked on the heap. Frees memory for the pointer and for 
*       each element in the 2D array of pixels. Returns void.
*/
void free_word_pixmap(word_pixmap pixmap) {
        assert(pixmap != NULL);

        pixmap->methods->free(&(pixmap->pixels));
        free(pixmap);
}