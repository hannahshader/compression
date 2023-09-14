/******************************************************************************
*       file_word.c
*       By: Kalyn (kmuhle01) and Hannah (hshade01)
*       3/8/2023
*
*       Comp40 Project 4: arith
*   
*       This file contains the functions necessary to read words from a file 
*       and store them in a word_pixmap (decompression), and write words 
*       stored in a word_pixmap to a file (compression). 
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
#include "bitpack.h"
#include "word_unpacked.h"
#include "file_word.h"

#define COMPRESSED_BLOCK_SIZE 1
#define MAX_BITS 32
#define CHAR_BITS 8
#define BLOCK_SIZE 2

/******** COMPRESSION HELPER FUNCTIONS ********/
void write_word_mapping(int i, int j, A2Methods_UArray2 array2, 
                A2Methods_Object *word, void *cl);

/******** DECOMPRESSION HELPER FUNCTIONS ********/
void read_word_mapping(int i, int j, A2Methods_UArray2 array2, 
                A2Methods_Object *word, void *fp);



/************ COMPRESSION ************/

/*
*       Description: A function that writes the file header and calls a mapping
*       function that writes word bits as characters to an output file
*   
*       In/Out Expectations: Expects a valid type word_pixmap storing a 2D
*       array of words, which are of type uint32_t. No outputs. Expects that
*       the word_pixmap has an even number of rows and columns. 
*/
void write_to_file(word_pixmap pixmap) {
        assert(pixmap != NULL);

        pixmap->methods = uarray2_methods_blocked;
        printf("COMP40 Compressed image format 2\n%u %u\n", 
                pixmap->width * BLOCK_SIZE, pixmap->height * BLOCK_SIZE);
        pixmap->methods->map_block_major(pixmap->pixels, write_word_mapping,
                NULL);
}

/*
*       Description: A function that takes an element of a word_pixmap, and 
*       writes four corresponding chars to standard output. 
*   
*       In/Out Expectations: expects to take in an word element, which is a 
*       unint32_t, from a 2D array of words. Writes four chars to standard
*       output. No return values. 
*/
void write_word_mapping(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *word, void *cl) {
        uint32_t *curr_word = (uint32_t*)word;

        for (int i = MAX_BITS - CHAR_BITS; i >= 0; i = (i - CHAR_BITS)) {
                uint64_t c = Bitpack_getu(*curr_word, CHAR_BITS, i);
                putchar(c);
        }

        (void)i;
        (void)j;
        (void)array2;
        (void)cl;
}


/************ DECOMPRESSION ************/

/*
*       Description: A function that reads the header and characters from
*       an input file, and populates a 2D array of words, which are type
*       uint32_t, as a type word_pixmap. Populates this array by calling
*       a mapping function.
*   
*       In/Out Expectations: Expects a valid output file that has 
*       been sucessfully opened. Expects a valid header of this file that
*       gives the height and with of the number of words encoded as chars.
*       Mallocs space for a new word_pixmap and returns this 2D array. 
*/
word_pixmap read_from_file(FILE *input) {
        assert(input != NULL);

        unsigned height, width;
        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u",
                          &width, &height);
        assert(read == 2);
        int c = getc(input);
        assert(c == '\n');

        word_pixmap new_word_pixmap = malloc(sizeof(*new_word_pixmap));
        assert(new_word_pixmap);

        new_word_pixmap->width = width / BLOCK_SIZE;
        new_word_pixmap->height = height / BLOCK_SIZE;
        new_word_pixmap->methods = uarray2_methods_blocked;

        uint32_t dummy_uint32;
        A2Methods_UArray2 pixmap = new_word_pixmap->methods->
                new_with_blocksize(width / BLOCK_SIZE, height / BLOCK_SIZE, 
                sizeof(dummy_uint32), COMPRESSED_BLOCK_SIZE);
        new_word_pixmap->pixels = pixmap;

        new_word_pixmap->methods->map_block_major(new_word_pixmap->pixels, 
                read_word_mapping, input);
        return new_word_pixmap;
}

/*
*       Description: A function that takes an element of a word_pixmap, and 
*       gets four corresponding bits and imbeddeds them into a uint32_t. This
*       uint32_t will be stored in an corresponding element of the word_pixmap. 
*   
*       In/Out Expectations: expects to take in an word element, which is a 
*       unint32_t, that is initially zero. Expects that four characters can 
*       be read from a valid input file. Sets the current word. No return val.
*/
void read_word_mapping(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *word, void *fp) {
        assert(fp != NULL);
        FILE *input = (FILE*)fp;  
        uint32_t *curr_word = (uint32_t*)word;
        for (int i = MAX_BITS - CHAR_BITS; i >= 0; i = (i - CHAR_BITS)) {
                int c = getc(input);
                if (c >= 0){
                        *curr_word = Bitpack_newu(*curr_word, CHAR_BITS, 
                                i, (uint64_t)c);
                }
        }  
            
        (void)i;
        (void)j;
        (void)array2;
}