/******************************************************************************
*       cv_rgb.c
*       By: Kalyn (kmuhle01) and Hannah (hshade01)
*       3/8/2023
*
*       Comp40 Project 4: arith
*   
*       This file contains the functions necessary to convert an image in the
*       form of a Pnm_ppm to a cv_pixmap (compression), and back from
*       a cv_pixmap to a Pnm_ppm (decompression). 
*   
******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <a2methods.h>
#include "assert.h"
#include "pnm.h"
#include "a2blocked.h"
#include "uarray2b.h"
#include "cv_rgb.h"

#define BLOCK_SIZE 2
#define CHOSEN_DENOMINATOR 3000


/******** COMPRESSION HELPER FUNCTIONS ********/
void rgb_to_cv(Pnm_rgb rgb, int denominator, cv_t cv);
void rgb_to_cv_mapping(int i, int j, A2Methods_UArray2 array2, 
                A2Methods_Object *cv, void *ppm);

/******** DECOMPRESSION HELPER FUNCTIONS ********/
void cv_to_rgb_mapping(int i, int j, A2Methods_UArray2 array2, 
                A2Methods_Object *rgb, void *pixmap);
void cv_to_rgb(cv_t cv, Pnm_rgb rgb);
unsigned rgb_unsigned(float color); 



/************ COMPRESSION ************/

/*
*       Description: A function that takes Pnm_oom containing
*       a 2D array of types Pnm_rgb and creates an associated struct
*       cv_pixmap containing a 2D array of cv_ts. 
*
*   
*       In/Out Expectations: Expects a valid struct type Pnm_ppm. Mallocs
*       memory for a struct cv_pixmap that the client must eventually free.
*       Returns this created type of cv_pixmap.  
*/
cv_pixmap rgb_to_cv_pixmap(Pnm_ppm ppm) {
        assert(ppm != NULL);

        int width = ppm->width;
        int height = ppm->height;

        cv_pixmap new_cv_pixmap = malloc(sizeof(*new_cv_pixmap));
        assert(new_cv_pixmap != NULL);

        new_cv_pixmap->width = width;
        new_cv_pixmap->height = height;
        new_cv_pixmap->methods = uarray2_methods_blocked;
        
        cv_t dummy_cv;
        A2Methods_UArray2 pixmap = new_cv_pixmap->
                methods->new_with_blocksize(width, height, sizeof(*dummy_cv),
                BLOCK_SIZE);
        new_cv_pixmap->pixels = pixmap;

        new_cv_pixmap->methods->map_block_major(new_cv_pixmap->pixels,
                rgb_to_cv_mapping, ppm);

        return new_cv_pixmap;
}

/*
*       Description: A function that converts a Pnm_rgb element of an 
*       Pnm_ppm to an associated cv_t, and places it in 
*       the associated element of a cv_pixmap. 
*
*       In/Out Expectations: expects to take in an uninitialized cv_t 
*       element, the row and col where it lies the cv_pixmap, and a 
*       pointer to a Pnm_ppm. Assigns value to the cv_t from
*       an Pnm_rgb element of the Pnm_ppm with a helper function. 
*       Returns void. 
*/
void rgb_to_cv_mapping(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *cv, void *ppm) {
        assert(ppm != NULL);

        Pnm_ppm rgb_pixmap = (Pnm_ppm)ppm;
        Pnm_rgb curr_rgb = rgb_pixmap->methods->at(rgb_pixmap->pixels, i, j);
        cv_t curr_cv = (cv_t)cv;
        rgb_to_cv(curr_rgb, rgb_pixmap->denominator, curr_cv);
        (void)array2;
}

/*
*       Description: Generates a type cv_t from a type Pnm_rgb.
*
*       In/Out Expectations: expects to take a Pnm_rgb, a denominator value,
*       and an instance of struct cv_t. Sets values of
*       the cv_t from the Pnm_rgb and the denominator. Returns void.
*/
void rgb_to_cv(Pnm_rgb rgb, int denominator, cv_t cv) {
        assert(cv != NULL);

        float red_scaled = (((float)(rgb->red)) / denominator);
        float green_scaled = (((float)(rgb->green)) / denominator);
        float blue_scaled = (((float)(rgb->blue)) / denominator);
        float y = 0.299 * red_scaled + 0.587 * green_scaled 
                        + 0.114 * blue_scaled;
        float pb = -0.168736 * red_scaled - 0.331264 * green_scaled 
                        + 0.5 * blue_scaled;
        float pr = 0.5 * red_scaled - 0.418688 * green_scaled 
                        - 0.081312 * blue_scaled;
        cv->y = y;
        cv->pb = pb;
        cv->pr = pr;
}


/************ DECOMPRESSION ************/

/*
*       Description: A function that takes a struct cv_pixmap containing
*       a 2D array of cv_ts and creates an associated Pnm_ppm containing 
*       a 2D array of types Pnm_rgb. 
*
*       In/Out Expectations: Expects a struct type cv_pixmap. Mallocs
*       memory for a Pnm_ppm that the client must eventually 
*       free. Returns this created Pnm_ppm.  
*/
Pnm_ppm cv_to_rgb_pixmap(cv_pixmap old_cv_pixmap) {
        assert(old_cv_pixmap != NULL);

        int width = old_cv_pixmap->width;
        int height = old_cv_pixmap->height;

        Pnm_ppm new_rgb_pixmap = malloc(sizeof(*new_rgb_pixmap));
        assert(new_rgb_pixmap != NULL);

        new_rgb_pixmap->width = width;
        new_rgb_pixmap->height = height;
        new_rgb_pixmap->denominator = CHOSEN_DENOMINATOR;
        new_rgb_pixmap->methods = uarray2_methods_blocked;

        Pnm_rgb dummy_rgb;
        A2Methods_UArray2 rgb_pixmap = new_rgb_pixmap->methods->
                new_with_blocksize(width, height, sizeof(*dummy_rgb), 
                BLOCK_SIZE);
        new_rgb_pixmap->pixels = rgb_pixmap;
        
        
        new_rgb_pixmap->methods->map_block_major(new_rgb_pixmap->pixels, 
                cv_to_rgb_mapping, old_cv_pixmap);

        return new_rgb_pixmap;
}

/*
*       Description: A function that converts a cv_t element of an 
*       cv_pixmap to an associated Pnm_rgb, and places it in 
*       the associated index of a Pnm_ppm. 
*
*       In/Out Expectations: expects to take in an uninitialized Pnm_rgb
*       element, the row and col where it lies the Pnm_ppm, and a 
*       pointer to a cv_pixmap. Assigns value from and associated cv_t in
*       the  cv_pixmap to the current Pnm_rgb with a helper function.
*       Returns void. 
*/
void cv_to_rgb_mapping(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *rgb, void *pixmap) {
        assert(pixmap != NULL);

        cv_pixmap pixmap_cv = (cv_pixmap)pixmap;
        cv_t curr_cv = pixmap_cv->methods->at(pixmap_cv->pixels, i, j);
        Pnm_rgb curr_rgb = (Pnm_rgb)rgb;

        cv_to_rgb(curr_cv, curr_rgb);

        (void)array2;
}

/*
*       Description: Generates a type Pnm_rgb from a type cv_t.
*
*       In/Out Expectations: expects to take instace of cv_t, and an
*       Pnm_rgb. Sets values of the Pnm_rgb with calculations using 
*       the cv_t. Returns void.
*/
void cv_to_rgb(cv_t cv, Pnm_rgb rgb) {
        assert(cv != NULL);
        assert(rgb != NULL);


        float y = cv->y;
        float pb = cv->pb;
        float pr = cv->pr; 

        float red_scaled = (1.0 * y + 0.0 * pb + 1.402 * pr);
        float green_scaled = (1.0 * y - 0.344136 * pb - 0.714136 * pr);
        float blue_scaled = (1.0 * y + 1.772 * pb + 0.0 * pr);

        rgb->red= rgb_unsigned(red_scaled);
        rgb->green = rgb_unsigned(green_scaled);
        rgb->blue = rgb_unsigned(blue_scaled);
}

/*
*       Description: Scales a float to a larger unsigned with a 
*       denominator value. 
*
*       In/Out Expectations: expects to take in a float (any value). Returns
*       an unsinged value between 0 and the defined denominator.
*/
unsigned rgb_unsigned(float color){
        if (color <= 0){
                return (unsigned) 0;
        } else if ((color * CHOSEN_DENOMINATOR) >= CHOSEN_DENOMINATOR) {
                return (unsigned) CHOSEN_DENOMINATOR;
        } else {
                return (unsigned)(color * CHOSEN_DENOMINATOR);
        }
}

/*
*       Description: Frees memory associated with a cv_pixmap.
*
*       In/Out Expectations: expects to take a cv_pixmap that's been
*       mallocked on the heap. Frees memory for the pointer and for 
*       each element in the 2D array of pixels. Returns void.
*/
void free_cv_pixmap(cv_pixmap pixmap) {
        assert(pixmap != NULL);
        pixmap->methods->free(&(pixmap->pixels));
        free(pixmap);
}