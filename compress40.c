/******************************************************************************
*       compress40.c
*       By: Kalyn (kmuhle01) and Hannah (hshade01)
*       3/8/2023
*
*       Comp40 Project 4: arith
*   
*       This file contains the functions necessary to compress an image or 
*       decompress a compressed image from a given file from the user (parsed 
*       in 40image.c), as well as trim the original image if it has an odd 
*       width/height
*   
******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <a2methods.h>
#include "assert.h"
#include "pnm.h"
#include "a2blocked.h"
#include "uarray2b.h"
#include "compress40.h"
#include "cv_rgb.h"
#include "unpacked_cv.h"
#include "word_unpacked.h"
#include "file_word.h"

/******** HELPER FUNCTIONS ********/
Pnm_ppm make_even(Pnm_ppm image);
void copy_pixmap(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *rgb, void *image);
                     

/*
*       Description: A function that reads a Pnm_ppm from an input file
*       and writes a header and chars associated with the Pnm_ppm pixels 
*       values to standard output. 
*
*       In/Out Expectations: expects to take in a valid ppm type input file.
*       After writing to standard output frees all memory associated with 
*       the structs made to preform the compression. No return value.
*/
void compress40  (FILE *input) {
        assert(input != NULL);

        A2Methods_T methods = uarray2_methods_blocked;
        Pnm_ppm image = Pnm_ppmread(input, methods);
        assert(image != NULL);
        
        image = make_even(image);

        cv_pixmap cv_image = rgb_to_cv_pixmap(image);
        unpacked_pixmap unpacked_image = cv_to_unpacked_pixmap(cv_image);
        word_pixmap packed_image = unpacked_to_word_pixmap(unpacked_image);
        write_to_file(packed_image);
 
        free_cv_pixmap(cv_image);
        free_unpacked_pixmap(unpacked_image);
        free_word_pixmap(packed_image);
        Pnm_ppmfree(&image);

}

/*
*       Description: A function that reads a header and characters from a file
*       and converts this data to a Pnm_ppm, which is read to standard output.
*
*       In/Out Expectations: expects to take in a valid input file with
*       formatted header and characters as specified in the Arith40 spec. Frees
*       all memory associated with the Pnm_ppm. Returns nothing.
*/
void decompress40(FILE *input){
        assert(input != NULL);

        word_pixmap word_image = read_from_file(input);
        unpacked_pixmap unpacked_image = word_to_unpacked_pixmap(word_image);
        cv_pixmap cv_image = unpacked_to_cv_pixmap(unpacked_image);
        Pnm_ppm rgb_image = cv_to_rgb_pixmap(cv_image);
        Pnm_ppmwrite(stdout, rgb_image);

        free_word_pixmap(word_image);
        free_unpacked_pixmap(unpacked_image);
        free_cv_pixmap(cv_image);
        Pnm_ppmfree(&rgb_image);
}

/*
*       Description: A function that trims row and columns if needed to 
*       create even numbers of rows and columns. 
*
*       In/Out Expectations: expects to take in a valid type Pnm_ppm. Mallocs
*       space for the trimmed new Pnm_ppm and frees memory for the old.
*       Returns a type Pnm_ppm. Memory for this Pnm_ppm must be freed by the
*       client. 
*/
Pnm_ppm make_even(Pnm_ppm image) {
        int old_width = (image->methods)->width(image->pixels);
        int old_height = (image->methods)->height(image->pixels);
        
        if ((old_height % 2) == 0 && (old_width % 2) == 0) {
                return image;
        }

        int new_width = (old_width / 2) * 2;
        int new_height = (old_height / 2) * 2;
        Pnm_ppm new_image = malloc(sizeof(struct Pnm_ppm));
        A2Methods_UArray2 new_pixmap = image->methods->
                        new(new_width, new_height,image->methods->
                        size(image->pixels));
        new_image->width = new_width;
        new_image->height = new_height;
        new_image->pixels = new_pixmap;
        new_image->methods = image->methods;
        new_image->denominator = image->denominator;

        image->methods->map_block_major(new_image->pixels, copy_pixmap, image);

        Pnm_ppmfree(&image);
        
        return new_image;
}

/*
*       Description: A function that sets all values of an new Pnm_ppm
*       equal to all values of an old Pnm_ppm. 
*
*       In/Out Expectations: expects to take in element to set in the new 
*       Pnm_ppm, the row and col value of this element, and the old Pnm_ppm.
*       The element passed will be set with the corresponding value at the 
*       same row and col index of the old Pnm_ppm. Returns void. Expects the
*       element of the new Pnm_ppm to be of type Pnm_rgb, and for the old 
*       Pnm_ppm to be a valid Pnm_ppm containing types Pnm_rgb.
*/
void copy_pixmap(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *rgb, void *image) {
        Pnm_ppm old_image = (Pnm_ppm)image;
        Pnm_rgb new_rgb =  (Pnm_rgb)rgb;
        Pnm_rgb old_rgb = old_image->methods->
                                   at(old_image->pixels, i, j);
        *(new_rgb) = *(old_rgb);
        (void)array2;
}