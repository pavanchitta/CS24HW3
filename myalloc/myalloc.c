/*! \file
 * Implementation of a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004-2010.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "myalloc.h"


/*!
 * These variables are used to specify the size and address of the memory pool
 * that the simple allocator works against.  The memory pool is allocated within
 * init_myalloc(), and then myalloc() and free() work against this pool of
 * memory that mem points to.
 */
int MEMORY_SIZE;
unsigned char *mem;


/* TODO:  The unacceptable allocator uses an external "free-pointer" to track
 *        where free memory starts.  If your allocator doesn't use this
 *        variable, get rid of it.
 *
 *        You can declare data types, constants, and statically declared
 *        variables for managing your memory pool in this section too.
 */
static unsigned char *freeptr;
static unsigned char *startptr;
typedef struct header {
         /* Negative size means the block is allocated, */
         /* positive size means the block is available. */
         int size;
}header;
typedef struct footer {
         /* Negative size means the block is allocated, */
         /* positive size means the block is available. */
         int size;
}footer;
int num_blocks;


/*!
 * This function initializes both the allocator state, and the memory pool.  It
 * must be called before myalloc() or myfree() will work at all.
 *
 * Note that we allocate the entire memory pool using malloc().  This is so we
 * can create different memory-pool sizes for testing.  Obviously, in a real
 * allocator, this memory pool would either be a fixed memory region, or the
 * allocator would request a memory region from the operating system (see the
 * C standard function sbrk(), for example).
 */
void init_myalloc() {

    /*
     * Allocate the entire memory pool, from which our simple allocator will
     * serve allocation requests.
     */
    mem = (unsigned char *) malloc(MEMORY_SIZE);
    if (mem == 0) {
        fprintf(stderr,
                "init_myalloc: could not get %d bytes from the system\n",
		MEMORY_SIZE);
        abort();
    }

    /* initialize the initial state of memory pool. */

    freeptr = mem;
    startptr = mem;
    header *h = (header *) mem; // Cast the mem pointer to header * to set size
    h->size = MEMORY_SIZE - 2*sizeof(header); //size only holds the size of
                                            //the payload
    footer *f = (footer *) (mem + MEMORY_SIZE - sizeof(footer));
    f->size = MEMORY_SIZE - 2*sizeof(header);
    num_blocks = 1;
}


/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 */
unsigned char *myalloc(int size) {

    /* TODO:  The unacceptable allocator simply checks to see if there are at
     *        least "size" bytes left in the pool, and if so, the caller gets
     *        the current "free-pointer" value, and then freeptr is incremented
     *        by size bytes.
     *
     *        Your allocator will be more sophisticated!
     */

    printf("Sanity Check Result: %d\n", sanity_check());
    header *h = (header *) startptr;
    header *best_header = h;
    int min_diff = INT_MAX;
    /* Implement best-fit strategy for finding available space */
    while ((unsigned char *) h < startptr + MEMORY_SIZE) {
        if ( h->size > 0 && (h->size > size)) {
            int diff = h->size - size;
            if (diff < min_diff) {
                min_diff = diff;
                best_header = h;
            }

        }
        h = (void *) h + 2*sizeof(header) + abs(h->size);
    }

    if ( best_header->size > 0 && (best_header->size > size)) {

        unsigned char *resultptr = (void *) best_header + sizeof(header) + size;
        footer *f = (footer *) resultptr;
        f->size = -1*(size);
        resultptr += sizeof(footer);
        int temp = best_header->size;
        best_header->size = -1*(size);
        header *h2 = (header *) resultptr;
        h2->size = temp - (size) - sizeof(footer) - sizeof(header);
        footer *old_footer = (void *) best_header + size + 3*sizeof(header) + h2->size;
        old_footer->size = h2->size;
        num_blocks++;
        printf("Sanity Check Result: %d\n", sanity_check());
        resultptr = (unsigned char *) best_header + sizeof(header);
        return resultptr;
    }
    else {
        fprintf(stderr, "myalloc: cannot service request of size %d with"
                " %lx bytes allocated\n", size, (freeptr - mem));
        return (unsigned char *) 0;
    }
}


/*!
 * Free a previously allocated pointer.  oldptr should be an address returned by
 * myalloc().
 */
void myfree(unsigned char *oldptr) {
    /* TODO:
     *
     * The unacceptable allocator does nothing -- that's part of why this is
     * unacceptable!
     *
     * Allocations will succeed for a little while...
     */
}

/*!
 * Clean up the allocator state.
 * All this really has to do is free the user memory pool. This function mostly
 * ensures that the test program doesn't leak memory, so it's easy to check
 * if the allocator does.
 */
void close_myalloc() {
    free(mem);
}

int sanity_check() {
    int count = 0;
    int size = 0;
    int old_size = 0;
    //unsigned char *p = startptr;
    header *h = (header *) startptr;
    while (count < num_blocks) {
        old_size = size;
        size += abs(h->size) + 2*sizeof(header); //account for the footer

        h = (void *) h + size - old_size;
        count++;

    }

    if (size == MEMORY_SIZE && (unsigned char *) h == startptr + MEMORY_SIZE) {
        return 1;
    } else {
        return 0;
    }
}
