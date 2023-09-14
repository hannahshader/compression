/******************************************************************************
*       unpacked_cv.c
*       By: Kalyn (kmuhle01) and Hannah (hshade01)
*       3/8/2023
*
*       Comp40 Project 4: arith
*   
*       This file contains the functions necessary to convert an image in the
*       form of a cv_pixmap to an unpacked_pixmap (compression), and back from
*       an unpacked_pixmap to a cv_pixmap (decompression). 
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
#include "arith40.h"
#include "cv_rgb.h"
#include "unpacked_cv.h"

#define BLOCK_SIZE 2
#define COMPRESSED_BLOCK_SIZE 1

/* 
 * struct unpacked_data
 *      This is a struct to be used as a closure in the mapping functions. It
 *      contains an unpacked_pixmap pixmap, 3 float arrays for the pb, pr, and
 *      y values of the 4 pixels in each block, and a counter (used to 
 *      determine when the 4 pixels have all been seen)
 */
struct unpacked_data{
        unpacked_pixmap pixmap;
        float pb_values[BLOCK_SIZE * BLOCK_SIZE];
        float pr_values[BLOCK_SIZE * BLOCK_SIZE];
        float y_values[BLOCK_SIZE * BLOCK_SIZE];
        int counter;
};


/******** COMPRESSION HELPER FUNCTIONS ********/
void cv_to_unpacked_mapping(int i, int j, A2Methods_UArray2 array2, 
                A2Methods_Object *cv, void *unpacked_pixmap);
void cv_to_unpacked(struct unpacked_data *block_data, unpacked_t curr);
signed scaled_val(float val);
float average_values(float values[]);

/******** DECOMPRESSION HELPER FUNCTIONS ********/
void unpacked_to_cv_mapping(int i, int j, A2Methods_UArray2 array2, 
                A2Methods_Object *cv, void *old_unpacked_pixmap);
void calculate_y_vals(unpacked_t curr_unpacked, 
                struct unpacked_data *block_data);
float unscaled_val(signed val);


/************ COMPRESSION ************/

/*
*       Description: A function that takes a struct cv_pixmap containing
*       a 2D array of structs cv_t and creates an associated struct
*       unpacked_pixmap containing a 2D array of unpacked_ts, where every
*       four cv_ts are associated with one struct unpacked_t.
*
*       In/Out Expectations: Expects a struct type cv_pixmap. Mallocs
*       memory for a struct unpacked_pixmap that the client must eventually 
*       free. Returns this created type of unpacked_pixmap.  
*/
unpacked_pixmap cv_to_unpacked_pixmap(cv_pixmap old_cv_pixmap) {
        assert(old_cv_pixmap != NULL);

        int width = old_cv_pixmap->width / BLOCK_SIZE;
        int height = old_cv_pixmap->height / BLOCK_SIZE;

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

        struct unpacked_data block_data;
        block_data.pixmap = new_unpacked_pixmap;
        block_data.counter = 0;

        
        new_unpacked_pixmap->methods->map_block_major(old_cv_pixmap->pixels, 
                cv_to_unpacked_mapping, &block_data);

        return new_unpacked_pixmap;
}

/*
*       Description: A function that converts four cv_t elements of a 
*       cv_pixmap to an associated unpacked_t, and places it in 
*       the associated element of a unpacked_pixmap. 
*
*       In/Out Expectations: expects to take in a cv_t 
*       element, the row and col where it lies the cv_pixmap, and a 
*       pointer to struct of type unpacked_data. After populating
*       data fields of an unpacked_data struct with data from four
*       cv_ts, a value is assigned to one associated unpacked_t.
*       Returns void. 
*/
void cv_to_unpacked_mapping(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *cv, void *cl_struct) {
        assert(cl_struct != NULL);

        struct unpacked_data *block_data = ((struct unpacked_data*)cl_struct);
        cv_t curr_cv = (cv_t)cv;
        unpacked_pixmap new_pixmap = block_data->pixmap;

        block_data->pb_values[block_data->counter] = curr_cv->pb;
        block_data->pr_values[block_data->counter] = curr_cv->pr;
        block_data->y_values[block_data->counter] = curr_cv->y;

        if (block_data->counter == 3) {
                unpacked_t curr_unpacked = new_pixmap->methods->at
                        (new_pixmap->pixels, i / BLOCK_SIZE, j / BLOCK_SIZE);
                cv_to_unpacked(block_data, curr_unpacked);
                block_data->counter = 0;
        } else {
                block_data->counter++;
        }

        (void)array2;
}

/*
*       Description: Generates a unpacked_t from a populated
*       struct of type block_data.
*
*       In/Out Expectations: expects a pointer to a struct block_data
*       which must have been already populated with data from four cv_ts.
*       Takes an instance of unpacked_t. Sets values of the unpacked_t
*       from the associated block_data struct. Returns void.
*/
void cv_to_unpacked(struct unpacked_data *block_data, unpacked_t curr) {
        assert(block_data != NULL);
        assert(curr != NULL);

        float pb_avg_temp = average_values(block_data->pb_values);
        float pr_avg_temp = average_values(block_data->pr_values);
        unsigned pb_avg = Arith40_index_of_chroma(pb_avg_temp);
        unsigned pr_avg = Arith40_index_of_chroma(pr_avg_temp);
        
        /* 
         * Note that y_values[1] represents Y2 and y_values[2] represents Y3,
         * since map block block major traverses Y1->Y3->Y2->Y4
         */
        float a_temp = (block_data->y_values[3] + block_data->y_values[1] +
                        block_data->y_values[2] + block_data->y_values[0]) / 
                        (BLOCK_SIZE * BLOCK_SIZE);
        float b_temp = (block_data->y_values[3] + block_data->y_values[1] - 
                        block_data->y_values[2] - block_data->y_values[0]) / 
                        (BLOCK_SIZE * BLOCK_SIZE);
        float c_temp = (block_data->y_values[3] - block_data->y_values[1] + 
                        block_data->y_values[2] - block_data->y_values[0]) / 
                        (BLOCK_SIZE * BLOCK_SIZE);
        float d_temp = (block_data->y_values[3] - block_data->y_values[1] - 
                        block_data->y_values[2] + block_data->y_values[0]) / 
                        (BLOCK_SIZE * BLOCK_SIZE);

        unsigned a = a_temp * 511;
        signed b = scaled_val(b_temp);
        signed c = scaled_val(c_temp);
        signed d = scaled_val(d_temp);

        curr->a = a;
        curr->b = b;
        curr->c = c;
        curr->d = d;
        curr->pr_avg = pr_avg;
        curr->pb_avg = pb_avg;
}

/*
*       Description: Scales a float to a signed value between 15 and -15
*
*       In/Out Expectations: expects a float (most commonly between -0.3
*       and 0.3). Returns a signed value between -15 and 15. 
*/
signed scaled_val(float val) {
        if (val >= 0.3) {
                return 15;
        } else if (val <= -0.3) {
                return -15;
        } else {
                return (signed) (val / 0.02);
        }
}

/*
*       Description: Averages the values in an array of floats. 
*
*       In/Out Expectations: Expects an array of floats. Stores
*       and returns the mean as a float. Expects a defined 
*       block size that determines the size of the array.
*/
float average_values(float *values) {
        assert(values != NULL);

        float sum = 0;
        for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++) {
                sum += values[i];
        }
        return (sum / (BLOCK_SIZE * BLOCK_SIZE));
}

/************ DECOMPRESSION ************/

/*
*       Description: A function that takes a struct unpacked_pixmap containing
*       a 2D array of structs unpacked_t and creates an associated struct
*       cv_pixmap containing a 2D array of cv_ts, where every
*       four cv_ts are associated with one struct unpacked_t.
*
*       In/Out Expectations: Expects a struct type unpacked_pixmap. Mallocs
*       memory for a struct cv_pixmap that the client must eventually 
*       free. Returns this created type of cv_pixmap.  
*/
cv_pixmap unpacked_to_cv_pixmap(unpacked_pixmap old_unpacked_pixmap) {
        assert(old_unpacked_pixmap != NULL);

        int width = old_unpacked_pixmap->width * BLOCK_SIZE;
        int height = old_unpacked_pixmap->height * BLOCK_SIZE;

        cv_pixmap new_cv_pixmap = malloc(sizeof(*new_cv_pixmap));
        assert(new_cv_pixmap != NULL);

        new_cv_pixmap->width = width;
        new_cv_pixmap->height = height;
        new_cv_pixmap->methods = uarray2_methods_blocked; 
        
        cv_t dummy_cv;
        A2Methods_UArray2 pixmap = new_cv_pixmap->methods->
                new_with_blocksize(width, height, sizeof(*dummy_cv),
                BLOCK_SIZE);
        new_cv_pixmap->pixels = pixmap;

        struct unpacked_data block_data;
        block_data.pixmap = old_unpacked_pixmap;
        block_data.counter = 0;
        
        new_cv_pixmap->methods->map_block_major(new_cv_pixmap->pixels, 
                unpacked_to_cv_mapping, &block_data);

        return new_cv_pixmap;
}

/*
*       Description: A function that gets data values for a cv_t from 
*       the associated unpacked_t element of an unpacked_pixmap. Fills
*       data fields with this data for four associated cv_ts.
*
*       In/Out Expectations: expects to take in a cv_t 
*       element, the row and col where it lies the cv_pixmap, and a 
*       pointer to struct of type unpacked_data. Gets unpacked_pixmap
*       from unpacked_data. Populates four cv_ts and their Y values 
*       from data fields in unpacked_t. Returns void. 
*/
void unpacked_to_cv_mapping(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *cv, void *cl_struct) {

        struct unpacked_data *block_data = ((struct unpacked_data*)cl_struct);
        unpacked_pixmap old_pixmap = block_data->pixmap;
        unpacked_t curr_unpacked = old_pixmap->methods->
                at(old_pixmap->pixels, i / BLOCK_SIZE, j / BLOCK_SIZE);
        cv_t curr_cv = (cv_t)cv;

        if (block_data->counter == 0){
                /*
                 * before populating four cv_ts, gets a, b, c, d values from
                 * the associated unpacked_t to get a Y value for each cv_t 
                 */
                calculate_y_vals(curr_unpacked, block_data);
        }

        curr_cv->y = block_data->y_values[block_data->counter];
        curr_cv->pb = Arith40_chroma_of_index(curr_unpacked->pb_avg);
        curr_cv->pr = Arith40_chroma_of_index(curr_unpacked->pr_avg);
        block_data->counter = (block_data->counter + 1) % 4;

        (void)array2;
}

/*
*       Description: A function that takes a type unpacked_t and gets the
*       Y values for four instances of cv_t stored in a struct 
*       unpacked_data.
*
*       In/Out Expectations: expects to take in an unpacked_t 
*       element, and an unpopulated instance of struct unpacked_data. 
*       Sets y value array in the unpacked_data struct and returns void.
*/
void calculate_y_vals(unpacked_t curr_unpacked, struct unpacked_data 
                      *block_data) {
        assert(curr_unpacked != NULL);
        assert(block_data != NULL);

        float a = ((float)(curr_unpacked->a)) / 511;
        float b = unscaled_val(curr_unpacked->b);
        float c = unscaled_val(curr_unpacked->c);
        float d = unscaled_val(curr_unpacked->d);

        block_data->y_values[0] = a - b - c + d;
        block_data->y_values[2] = a - b + c - d;
        block_data->y_values[1] = a + b - c - d;
        block_data->y_values[3] = a + b + c + d;
}

/*
*       Description: gets a scaled down version of an signed number between 
*       -15 and 15 as a float. 
*
*       In/Out Expectations: expects to take in a type signed between -15 
*       and 15. Returns a float between -0.3 and 0.3.
*/
float unscaled_val(signed val) {
        assert(val <= 15 && val >= -15);
        return ((float) val) * 0.02;
}

/*
*       Description: Frees memory associated with a unpacked_pixmap.
*
*       In/Out Expectations: expects to take a unpacked_pixmap that's been
*       mallocked on the heap. Frees memory for the pointer and for 
*       each element in the 2D array of pixels. Returns void.
*/
void free_unpacked_pixmap(unpacked_pixmap pixmap) {
        assert(pixmap != NULL);

        pixmap->methods->free(&(pixmap->pixels));
        free(pixmap);
}