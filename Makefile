# Makefile for Arith
# Last updated: March 7 2023


############## Variables ###############

CC = gcc # The compiler being used

IFLAGS = -I/comp/40/build/include -I/usr/sup/cii40/include/cii

CFLAGS = -g -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)

LDFLAGS = -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64 -larith40

LDLIBS = -l40locality -larith40 -lnetpbm -lcii40 -lm -lrt -lpnm

INCLUDES = $(shell echo *.h)

############### Rules ###############

all: ppm_diff 40image 

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@


ppm_diff: ppm_diff.o a2plain.o uarray2.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

40image: 40image.o compress40.o uarray2.o a2blocked.o uarray2b.o \
	cv_rgb.o unpacked_cv.o word_unpacked.o bitpack.o file_word.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f ppm_diff 40image *.o

