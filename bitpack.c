/******************************************************************************
*       bitpack.c
*       By: Kalyn (kmuhle01) and Hannah (hshade01)
*       3/8/2023
*
*       Comp40 Project 4: arith
*   
*       This file contains the implementation of the bitpack interface. It 
*       includes functions to get a value (signed or unsigned) from a 
*       specified field from a unint64_t, add a new value (signed or unsigned)
*       to a specified field in a unit64_t, or determine if a value can fit in
*       a certain width. 
*   
******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdint.h>
#include "bitpack.h"
#include "assert.h"
#include "except.h"

#define MAX_BITS 64

/******** HELPER FUNCTIONS ********/
uint64_t external_ones_mask(unsigned width, unsigned lsb);
bool first_idx_one(uint64_t word, unsigned width);
uint64_t shift_u_right(uint64_t word, unsigned shift_val);
uint64_t shift_u_left(uint64_t word, unsigned shift_val);
int64_t shift_s_left(int64_t word, unsigned shift_val);

Except_T Bitpack_Overflow = { "Overflow packing bits" };

/*
*       Description: A function that determined whether the bits
*       used in a uint64_t is less than or equal to 
*       a specified number of bits. 
*   
*       In/Out Expectations: Expects a uint64_t, and a width value for 
*       number of bits we are checking if the uint64_t fits into. 
*       The width value should be less than 64 (defined as MAX_BITS). 
*       Returns true of the uint64_t fits into the width, false otherwise.
*/
bool Bitpack_fitsu(uint64_t n, unsigned width) {
        assert(width <= MAX_BITS); 
        
        if (width == 0 && n == 0){
                return true;
        } else if (width <= 0) {
                return false;
        } else if (width == MAX_BITS){
                return true;
        }
        
        uint64_t mask = shift_u_left((uint64_t)1, width);

        if (n < mask) {
                return true;
        } else {
                return false;
        }

}

/*
*       Description: A function that determined whether the bits
*       used in a int64_t is less than or equal to 
*       a specified number of bits. 
*   
*       In/Out Expectations: Expects a int64_t, and a width value for 
*       number of bits we are checking if the int64_t fits into. 
*       The width value should be less than 64 (defined as MAX_BITS). 
*       Returns true of the int64_t fits into the width, false otherwise.
*/
bool Bitpack_fitss(int64_t n, unsigned width) {
        assert(width <= MAX_BITS);
        if (width == 0 && n == 0){
                return true;
        } else if (width <= 0) {
                return false;
        } else if (width == MAX_BITS){
                return true;
        }

        int64_t mask_neg = ~0;
        mask_neg = shift_s_left(mask_neg, width - 1);

        int64_t mask_pos = 1;
        mask_pos = shift_s_left(mask_pos, width - 1);

        if (n < mask_pos && n >= mask_neg) {
                return true;
        } else {
                return false;
        }
}

/*
*       Description: A function that gets a sequence of bits imbedded in
*       a uint64_t. This sequence found by starting at an specified index, and
*       getting a consecutive specfied number of bits after that index. 
*   
*       In/Out Expectations: Expects a uint64_t, a width value for 
*       number of bits we get getting, and a index of the least significant
*       bit to start from. The width value should be less than 64 
*       (defined as MAX_BITS). Returns the imbedded bit sequence found
*       as a uint64_t.
*/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb){
        assert(width <= MAX_BITS);
        word = shift_u_left(word, MAX_BITS - (lsb + width));
        word = shift_u_right(word, MAX_BITS - width);
        return word;
}

/*
*       Description: A function that gets a sequence of bits imbedded in
*       a uint64_t. This sequence found by starting at an specified index, and
*       getting a consecutive specfied number of bits after that index. 
*   
*       In/Out Expectations: Expects a uint64_t, a width value for 
*       number of bits we get getting, and a index of the least significant
*       bit to start from. The width value should be less than 64 
*       (defined as MAX_BITS). Returns the imbedded bit sequence found
*       as an int64_t.
*/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb){
        assert(width <= MAX_BITS);
        word = Bitpack_getu(word, width, lsb);
        if (first_idx_one(word, width)) {
                uint64_t mask = ~0;
                mask = shift_u_left(mask, width);
                return (int64_t)(word | mask);
        }
        return word;
}

/*
*       Description: A function that gets a sequence of bits, and imbedds 
*       them into a uint64_t. Where to imbed this sequence is found by 
*       starting at an specified index, and replacing a consecutive specfied 
*       number of bits after that index. 
*   
*       In/Out Expectations: Expects a larger uint64_t, a width value for 
*       number of bits we're replacing, and a index of the least significant
*       bit to start from. Expects a uint64_t that holds the bit sequence to
*       imbed in the larger uint64_t. The width value should be less than 64 
*       (defined as MAX_BITS). Expected the bit sequence to imbed fits
*       in the width provided. Expected that the width + the lsb will not be
*       greater than MAX_BITS, as that would mean values of the imbedded bit 
*       sequence would be lost as they wouldn't fit between the lsb and the
*       end of the  larger uint64_t. Returns the updated larger word as a 
*       uint64_t with the given value imbedded in it.
*/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value){
        assert(width <= MAX_BITS);
        assert(width + lsb <= MAX_BITS);

        if (!Bitpack_fitsu(value, width)){
                RAISE(Bitpack_Overflow);
        }

        assert(width <= MAX_BITS);
        value = shift_u_left(value, lsb);
        uint64_t mask = external_ones_mask(width, lsb);
        word = word & mask;
        uint64_t result = word | value;
        return result;
}

/*
*       Description: A function that gets a sequence of bits, and imbedds 
*       them into a uint64_t. Where to imbed this sequence is found by 
*       starting at an specified index, and replacing a consecutive specfied 
*       number of bits after that index. 
*   
*       In/Out Expectations: Expects a larger uint64_t, a width value for 
*       number of bits we're replacing, and a index of the least significant
*       bit to start from. Expects a int64_t that holds the bit sequence to
*       imbed in the larger uint64_t. The width value should be less than 64 
*       (defined as MAX_BITS). Expected the bit sequence to imbed fits
*       in the width provided. Expected that the width + the lsb will not be
*       greater than MAX_BITS, as that would mean values of the imbedded bit 
*       sequence would be lost as they wouldn't fit between the lsb and the
*       end of the  larger uint64_t. Returns the updated larger word as a 
*       uint64_t with the given value imbedded in it.
*/
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                      int64_t value){
        assert(width <= MAX_BITS);
        assert(width + lsb <= MAX_BITS);

        if (!Bitpack_fitss(value, width)){
            RAISE(Bitpack_Overflow);
        }

        uint64_t mask = ~0;
        mask = shift_u_right(mask, MAX_BITS - width);
        value = value & mask;
        return Bitpack_newu(word, width, lsb, value);
}

/*
*       Description: Creates a mask that has all values of 1s 
*       except for a field specified by a width and index, which is all 0s. 
*   
*       In/Out Expectations: Expects a uint64_t, a width value for 
*       number of bits we will turn to 0s, and a index of the least 
*       significant bit to start from. Returns this mask as a uint64_t.
*/
uint64_t external_ones_mask(unsigned width, unsigned lsb) {
        uint64_t mask_one = ~0;
        mask_one = shift_u_left(mask_one, lsb + width);
        uint64_t mask_two = ~0;
        mask_two = shift_u_right(mask_two, MAX_BITS - lsb);
        uint64_t mask_result = mask_one | mask_two;
        return mask_result;
}

/*
*       Description: Checks to see if the first (farthest left)
*       bit value of a uint64_t is 1. 
*   
*       In/Out Expectations: Expects a uint64_t, a width value for 
*       how large it is. Returns a bool if the first (farthest
*       left) bit value is 1.
*/
bool first_idx_one(uint64_t word, unsigned width) {
        word = shift_u_right(word, width - 1);
        return word & 1;
}

/*
*       Description: Performs a shift right for a uint64_t.
*   
*       In/Out Expectations: Expects a uint64_t, a a value for 
*       how much to shift that isn't more than the size of this
*       uint64_t. Returns the result of the shift. 
*/
uint64_t shift_u_right(uint64_t word, unsigned shift_val) {
        if (shift_val >= MAX_BITS){
                return (uint64_t)0;
        }
        return (word >> shift_val);
}

/*
*       Description: Performs a shift left for a uint64_t.
*   
*       In/Out Expectations: Expects a uint64_t, a a value for 
*       how much to shift that isn't more than the size of this
*       uint64_t. Returns the result of the shift. 
*/
uint64_t shift_u_left(uint64_t word, unsigned shift_val) {
        if (shift_val >= MAX_BITS){
                return (uint64_t)0;
        }
        return (word << shift_val);
}

/*
*       Description: Performs a shift left for a int64_t.
*   
*       In/Out Expectations: Expects a int64_t, a a value for 
*       how much to shift that isn't more than the size of this
*       int64_t. Returns the result of the shift. 
*/
int64_t shift_s_left(int64_t word, unsigned shift_val) {
        if (shift_val >= MAX_BITS) {
                return (int64_t)0;
        }
        return (word << shift_val);
}

