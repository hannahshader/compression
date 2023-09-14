#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <a2methods.h>
#include "assert.h"
#include "pnm.h"
#include "a2plain.h"

struct cl {
    unsigned max_val_one;
    float *sum;
    Pnm_ppm ppm_two;
};

void get_numerator(int i, int j, A2Methods_UArray2 pixels_one, 
                    A2Methods_Object *ptr, void *cl);

int main(int argc, char *argv[]) {
    FILE *fp_one;
    FILE *fp_two;

    fp_one = fopen(argv[1], "r");
    fp_two = fopen(argv[2], "r");

    A2Methods_T methods = uarray2_methods_plain;

    Pnm_ppm ppm_one = Pnm_ppmread(fp_one, methods);
    Pnm_ppm ppm_two = Pnm_ppmread(fp_two, methods);

    int width_one = ppm_one->methods->width(ppm_one->pixels);
    int width_two = ppm_one->methods->width(ppm_two->pixels);
    int height_one = ppm_one->methods->height(ppm_one->pixels);
    int height_two = ppm_one->methods->height(ppm_two->pixels);


    int width_smaller;
    if (abs(width_one - width_two) > 1) {
        fprintf(stderr, "Error: Width difference is greater than 1.\n");
        fprintf(stdout, "%0.1f\n", 1.0);
        return 1;
    } else if(width_one - width_two > 0){
        width_smaller = width_two;
    }
    else {
        width_smaller = width_one;
    }

    int height_smaller;
    if (abs(height_one - height_two) > 1) {
        fprintf(stderr, "Error: Height difference is greater than 1.\n");
        fprintf(stdout, "%0.1f\n", 1.0);
        return 1;
    } else if(height_one - height_two > 0){
        height_smaller = height_two;
    }
    else{
        height_smaller = height_one;
    }

    float numerator = 0;
    struct cl closure;
    closure.sum = &numerator;
    closure.ppm_two = ppm_two;
    closure.max_val_one = ppm_one->denominator;

    methods->map_row_major(ppm_one->pixels, get_numerator, &closure);
    
    float denominator = 3 * width_smaller * height_smaller;
    float final_sum = sqrt(numerator / denominator);
    fprintf(stdout, "%.4f\n", final_sum);
    
    (void)denominator;
    (void)argc;

}

void get_numerator(int i, int j, A2Methods_UArray2 pixels_one, 
                   A2Methods_Object *ptr, void *cl){
    struct cl *current = (struct cl *) cl;
    Pnm_ppm ppm_two = current->ppm_two;
    float *sum = current->sum;
    assert(sum != NULL);
    
    float maxval_one = current->max_val_one;
    float maxval_two = ppm_two->denominator;


    if(i < ppm_two->methods->width(ppm_two->pixels) && 
        j < ppm_two->methods->height(ppm_two->pixels)){
        A2Methods_UArray2 pixels_two = ppm_two->pixels;
        Pnm_rgb colors_one = (Pnm_rgb)ptr;
        Pnm_rgb colors_two = ppm_two->methods->at(pixels_two, i, j);
        float red_diff = (colors_one->red / maxval_one) - 
                        (colors_two->red / maxval_two);
        float green_diff = (colors_one->green / maxval_one) - 
                        (colors_two->green / maxval_two);
        float blue_diff = (colors_one->blue / maxval_one) - 
                        (colors_two->blue / maxval_two);
        *sum += (red_diff * red_diff) + (green_diff * green_diff) + 
                (blue_diff * blue_diff);
    }

    (void)pixels_one;
}

