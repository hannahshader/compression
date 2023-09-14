/******************************************************************************
*       word_unpacked.h
*       By: Kalyn (kmuhle01) and Hannah (hshade01)
*       3/8/2023
*
*       Comp40 Project 4: arith
*   
*       This file contains a struct word_pixmap, which represents a compressed
*       pixmap of uint32_t words. It also includes the function declarations 
*       for two functions used to convert between an unpacked_pixmap and a 
*       word_pixmap, as well as a function to free the word_pixmap.
*   
******************************************************************************/

#ifndef WORD_UNPACKED_
#define WORD_UNPACKED_

#include <a2methods.h>
#include "bitpack.h"
#include "unpacked_cv.h"


/* 
 * struct word_pixmap
 *      A struct that represents an entire pixmap of words. 
 *      Contains an unsigned width and height of the pixmap, an 
 *      A2Methods_Uarray2 pixels (which will contain uint32_ts), and an 
 *      A2Methods_T methods.
 */
typedef struct word_pixmap {
    unsigned width, height;
    A2Methods_UArray2 pixels;
    const struct A2Methods_T *methods;
} *word_pixmap;

/********** COMPRESSION **********/
word_pixmap unpacked_to_word_pixmap(unpacked_pixmap old_unpacked_pixmap);

/********** DECOMPRESSION **********/
unpacked_pixmap word_to_unpacked_pixmap(word_pixmap old_word_pixmap);

void free_word_pixmap(word_pixmap pixmap);

#endif