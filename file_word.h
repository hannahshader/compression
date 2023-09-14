#ifndef FILE_WORD_
#define FILE_WORD_

#include <stdio.h>
#include <a2methods.h>
#include "bitpack.h"
#include "word_unpacked.h"

/********** COMPRESSION **********/
void write_to_file(word_pixmap pixmap);

/********** DECOMPRESSION **********/
word_pixmap read_from_file(FILE *input);

#endif