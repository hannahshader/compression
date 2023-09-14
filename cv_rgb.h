/******************************************************************************
*       cv_rgb.h
*       By: Kalyn (kmuhle01) and Hannah (hshade01)
*       3/8/2023
*
*       Comp40 Project 4: arith
*   
*       This file contains two structs, cv_t and cv_pixmap. cv_t represents a
*       single pixel of an image in component color space, and cv_pixmap 
*       represents an pixmap of those pixels. It also includes the function 
*       declarations for two functions used to convert between a Pnm_ppm and 
*       a cv_pixmap, as well as a function to free the cv_pixmap.
*   
******************************************************************************/

#ifndef CV_RBG_
#define CV_RBG_

#include <a2methods.h>
#include "pnm.h"
#include "cv_rgb.h"

/* 
 * struct cv_t
 *      A struct that represents a pixel in component video color. Contains 3 
 *      floats for the y, pb, and pr values
 */
typedef struct cv_t {
    float y, pb, pr;
} *cv_t;

/* 
 * struct cv_pixmap
 *      A struct that represents an entire pixmap in component video color. 
 *      Contains an unsigned width and height of the pixmap, an 
 *      A2Methods_Uarray2 pixels (which will contain cv_ts), and an 
 *      A2Methods_T methods.
 */
typedef struct cv_pixmap {
        unsigned width, height;
        A2Methods_UArray2 pixels;
        const struct A2Methods_T *methods;
} *cv_pixmap;

/********** COMPRESSION **********/
cv_pixmap rgb_to_cv_pixmap(Pnm_ppm ppm);

/********** DECOMPRESSION **********/
Pnm_ppm cv_to_rgb_pixmap(cv_pixmap old_cv_pixmap);

void free_cv_pixmap(cv_pixmap pixmap);

#endif

