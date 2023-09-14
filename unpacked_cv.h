/******************************************************************************
*       unpacked_cv.h
*       By: Kalyn (kmuhle01) and Hannah (hshade01)
*       3/8/2023
*
*       Comp40 Project 4: arith
*   
*       This file contains two structs, unpacked_t and unpacked_pixmap. 
*       unpacked_t represents one compressed pixel (with the quantized average
*       yb, yr, and DCT y values of 4 original pixels) and unpacked_pixmap 
*       represents a pixmap of those compressed pixels. It also includes the
*       function declarations for two functions used to convert between a
*       cv_pixmap and an unpacked_pixmap, as well as a function to free the 
*       unpacked_pixmap
*   
******************************************************************************/

#ifndef UNPACKED_CV_
#define UNPACKED_CV_

#include <a2methods.h>
#include "cv_rgb.h"
#include "unpacked_cv.h"

/* 
 * struct unpacked_t
 *      A struct that represents a pixel (4 pixels compressed) as an unpacked 
 *      word, meaning a word which has not been converted to a uint32_t. 
 *      Contains unsigned pb_avg and pr_avg (the average pb and pr values from
 *      the 4 pixel block, quantized), unsigned a, and signed b, c, and d 
 *      (calculated using DCT, then quantized).
 */
typedef struct unpacked_t {
    unsigned pb_avg, pr_avg;
    signed b, c, d;
    unsigned a;
} *unpacked_t;

/* 
 * struct unpacked_pixmap
 *      A struct that represents an entire pixmap of unpacked words. 
 *      Contains an unsigned width and height of the pixmap, an 
 *      A2Methods_Uarray2 pixels (which will contain unpacked_ts), and an 
 *      A2Methods_T methods.
 */
typedef struct unpacked_pixmap {
        unsigned width, height;
        A2Methods_UArray2 pixels;
        const struct A2Methods_T *methods;
} *unpacked_pixmap;


/********** COMPRESSION **********/
unpacked_pixmap cv_to_unpacked_pixmap(cv_pixmap old_cv_pixmap);

/********** DECOMPRESSION **********/
cv_pixmap unpacked_to_cv_pixmap(unpacked_pixmap old_unpacked_pixmap);

void free_unpacked_pixmap(unpacked_pixmap pixmap);

#endif