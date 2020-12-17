
/**********************************************************************

   File          : cmpsc473-mm.c

   Description   : Slab allocation and defenses

***********************************************************************/
/**********************************************************************
Copyright (c) 2019 The Pennsylvania State University
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of The Pennsylvania State University nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <time.h> 
#include "cmpsc473-format-13.h"   // TASK 1: student-specific
#include "cmpsc473-mm.h"
#include <malloc.h>

/* Globals */
heap_t *mmheap;
unsigned int canary;

/* Defines */
#define FREE_ADDR( slab ) ( (unsigned long)slab->start + ( slab->obj_size * slab->bitmap->free ))


/**********************************************************************

    Function    : mm_init
    Description : Initialize slab allocation
    Inputs      : void
    Outputs     : 0 if success, -1 on error

***********************************************************************/

int mm_init( void )
{
	mmheap = (heap_t *)malloc( sizeof(heap_t) );
	if ( !mmheap ) return -1;

	// TASK 2: Initialize heap memory (using regular 'malloc') and 
	//   heap data structures in prep for malloc/free
	mmheap->start = (void*) memalign(PAGE_SIZE, HEAP_SIZE);
	mmheap->size = (HEAP_SIZE/PAGE_SIZE);

	mmheap->bitmap = (bitmap_t*) malloc(sizeof(bitmap_t));
	mmheap->bitmap->free = 0;
	mmheap->bitmap->size = 256;
	mmheap->bitmap->map = (word_t*) calloc(32, sizeof(word_t));

	mmheap->slabA = (slab_cache_t*) malloc(sizeof(slab_cache_t));
	mmheap->slabA->obj_size = sizeof(allocA_t) + (sizeof(allocA_t) % 16);
	mmheap->slabA->ct = 0;
	mmheap->slabA->current = NULL;
	
	mmheap->slabB = (slab_cache_t*) malloc(sizeof(slab_cache_t));
	mmheap->slabB->obj_size = sizeof(allocB_t) + (sizeof(allocB_t) % 16);
	mmheap->slabB->ct = 0;
	mmheap->slabB->current = NULL;

	mmheap->slabC = (slab_cache_t*) malloc(sizeof(slab_cache_t));
	mmheap->slabC->obj_size = sizeof(allocC_t) + (sizeof(allocC_t) % 16); 
	mmheap->slabC->ct = 0;
	mmheap->slabC->current = NULL;

	// initialize canary
	canary_init();

	return 0;
}


/**********************************************************************

    Function    : my_malloc
    Description : Allocate from slabs
    Inputs      : size: amount of memory to allocate
    Outputs     : address if success, NULL on error

***********************************************************************/

void *my_malloc( unsigned int size )
{
	void *addr = (void*) NULL; 
	int i;

	// TASK 2: implement malloc function for slab allocator
	if(size == 96){
		if(mmheap->slabA->current == NULL){
			slab_t *slab = (slab_t*) mmheap->start + (mmheap->bitmap->free * PAGE_SIZE) + (PAGE_SIZE - sizeof(slab_t));
			slab->start = mmheap->start + (mmheap->bitmap->free * PAGE_SIZE);
			slab->bitmap = (bitmap_t*) malloc(sizeof(bitmap_t));
			slab->bitmap->free = 0;
			slab->bitmap->size = 36;
			slab->bitmap->map = (word_t*) calloc(5, sizeof(word_t));
			slab->ct = 0;
			slab->real_size = 96;
			slab->obj_size = 112;
			slab->num_objs = 36;
			slab->state = SLAB_EMPTY;
			slab->next = slab;
			slab->prev = slab;
			mmheap->slabA->current = slab;
			mmheap->slabA->ct++;
			//for(i = 0; i < mmheap->bitmap->size; i++){
			//	if(!(get_bit(mmheap->bitmap->map, i))){
			//		mmheap->bitmap->free = i;
					//set_bit(mmheap->bitmap->map, i);
			//		break;
			//	}
			//}
			addr = slab->start;
		}
		else if(mmheap->slabA->current->state == SLAB_EMPTY){
			mmheap->slabA->current->state  = SLAB_PARTIAL;
			set_bit(mmheap->slabA->current->bitmap->map, mmheap->slabA->current->bitmap->free);
			//for(i = 0; i < mmheap->slabA->current->bitmap->size; i++){
				//if(!(get_bit(mmheap->slabA->current->bitmap->map, i))){
				//	mmheap->slabA->current->bitmap->free = i;
				//	set_bit(mmheap->bitmap->map, i);
				//	break;
				//}
			//}
			mmheap->slabA->current->ct++;
			addr = mmheap->slabA->current->start + (mmheap->slabA->current->bitmap->free * mmheap->slabA->current->obj_size);
		}
		else if(mmheap->slabA->current->state == SLAB_PARTIAL){
			//allocA_t *A = (allocA_t*) mmheap->slabA->current->start + (mmheap->slabA->current->bitmap->free * mmheap->slabA->current->obj_size);
			mmheap->slabA->current->ct++;
			if(mmheap->slabA->current->ct == mmheap->slabA->current->num_objs){
				mmheap->slabA->current->state  = SLAB_FULL;
				//set_bit(mmheap->bitmap->map, mmheap->bitmap->free);
				//mmheap->bitmap->free++;
			}
			set_bit(mmheap->slabA->current->bitmap->map, mmheap->slabA->current->bitmap->free);
			//for(i = 0; i < mmheap->slabA->current->bitmap->size; i++){
				//if(!(get_bit(mmheap->slabA->current->bitmap->map, i))){
					//mmheap->slabA->current->bitmap->free = i;
					//set_bit(mmheap->bitmap->map, i);
					//break;
				//}
			//}
			addr = mmheap->slabA->current->start + (mmheap->slabA->current->bitmap->free * mmheap->slabA->current->obj_size);
		}
		else if(mmheap->slabA->current->state == SLAB_FULL){
			slab_t *slab = (slab_t*) mmheap->start + (mmheap->bitmap->free * PAGE_SIZE) + (PAGE_SIZE - sizeof(slab_t));
			slab->start = mmheap->start + (mmheap->bitmap->free * PAGE_SIZE);
			slab->bitmap = (bitmap_t*) malloc(sizeof(bitmap_t));
			slab->bitmap->free = 0;
			slab->bitmap->size = 36;
			slab->bitmap->map = (word_t*) calloc(5, sizeof(word_t));
			slab->ct = 0;
			slab->real_size = 96;
			slab->obj_size = 112;
			slab->num_objs = 36;
			slab->state = SLAB_EMPTY;
			slab->prev = mmheap->slabA->current;
			slab->next = mmheap->slabA->current->next;
			mmheap->slabA->current->next = slab;
			mmheap->slabA->current->next->prev = slab;
			mmheap->slabA->ct++;
			//for(i = 0; i < mmheap->bitmap->size; i++){
			//	if(!(get_bit(mmheap->bitmap->map, i))){
			//		mmheap->bitmap->free = i;
			//		set_bit(mmheap->bitmap->map, i);
			//		break;
			//	}
			//}
			addr = mmheap->slabA->current->start + (mmheap->slabA->current->bitmap->free * mmheap->slabA->current->obj_size);
		}
	}
	else if(size == 72){
		if(mmheap->slabB->current == NULL){
			slab_t *slab = (slab_t*) mmheap->start + (mmheap->bitmap->free * PAGE_SIZE) + (PAGE_SIZE - sizeof(slab_t));
			slab->start = mmheap->start + (mmheap->bitmap->free * PAGE_SIZE);
			slab->bitmap = (bitmap_t*) malloc(sizeof(bitmap_t));
			slab->bitmap->free = 0;
			slab->bitmap->size = 50;
			slab->bitmap->map = (word_t*) calloc(7, sizeof(word_t));
			slab->ct = 0;
			slab->real_size = 72;
			slab->obj_size = 80;
			slab->num_objs = 50;
			slab->state = SLAB_EMPTY;
			slab->next = slab;
			slab->prev = slab;
			mmheap->slabB->current = slab;
			mmheap->slabB->ct++;
			addr = slab->start;
		}
		else if(mmheap->slabB->current->state == SLAB_EMPTY){
			mmheap->slabB->current->state  = SLAB_PARTIAL;
			set_bit(mmheap->slabB->current->bitmap->map, mmheap->slabB->current->bitmap->free);
			mmheap->slabB->current->bitmap->free++;
			mmheap->slabB->current->ct++;
			addr = mmheap->slabB->current->start + (mmheap->slabB->current->bitmap->free * mmheap->slabB->current->obj_size);
		}
		else if(mmheap->slabB->current->state == SLAB_PARTIAL){
			allocB_t *B = (allocB_t*) mmheap->slabB->current->start + (mmheap->slabB->current->bitmap->free * mmheap->slabB->current->obj_size);
			mmheap->slabB->current->ct++;
			if(mmheap->slabB->current->ct == mmheap->slabB->current->num_objs){
				mmheap->slabB->current->state  = SLAB_FULL;
				//set_bit(mmheap->bitmap->map, mmheap->bitmap->free);
				//mmheap->bitmap->free++;
			}
			set_bit(mmheap->slabB->current->bitmap->map, mmheap->slabB->current->bitmap->free);
			mmheap->slabB->current->bitmap->free++;
			addr = mmheap->slabB->current->start + (mmheap->slabB->current->bitmap->free * mmheap->slabB->current->obj_size);
		}
		else if(mmheap->slabB->current->state == SLAB_FULL){
			slab_t *slab = (slab_t*) mmheap->start + (mmheap->bitmap->free * PAGE_SIZE) + (PAGE_SIZE - sizeof(slab_t));
			slab->start = mmheap->start + (mmheap->bitmap->free * PAGE_SIZE);
			slab->bitmap = (bitmap_t*) malloc(sizeof(bitmap_t));
			slab->bitmap->free = 0;
			slab->bitmap->size = 50;
			slab->bitmap->map = (word_t*) calloc(7, sizeof(word_t));
			slab->ct = 0;
			slab->real_size = 72;
			slab->obj_size = 80;
			slab->num_objs = 50;
			slab->state = SLAB_EMPTY;
			slab->prev = mmheap->slabB->current;
			mmheap->slabB->current->next = slab;
			mmheap->slabB->current = slab;
			mmheap->slabB->ct++;
			addr = mmheap->slabB->current->start + (mmheap->slabB->current->bitmap->free * mmheap->slabB->current->obj_size);
		}
	}
	else if(size == 16){
		if(mmheap->slabC->current == NULL){
			slab_t *slab = (slab_t*) mmheap->start + (mmheap->bitmap->free * PAGE_SIZE) + (PAGE_SIZE - sizeof(slab_t));
			slab->start = mmheap->start + (mmheap->bitmap->free * PAGE_SIZE);
			slab->bitmap = (bitmap_t*) malloc(sizeof(bitmap_t));
			slab->bitmap->free = 0;
			slab->bitmap->size = 126;
			slab->bitmap->map = (word_t*) calloc(16, sizeof(word_t));
			slab->ct = 0;
			slab->real_size = 16;
			slab->obj_size = 32;
			slab->num_objs = 126;
			slab->state = SLAB_EMPTY;
			slab->next = slab;
			slab->prev = slab;
			mmheap->slabC->current = slab;
			mmheap->slabC->ct++;
			addr = slab->start;
		}
		else if(mmheap->slabC->current->state == SLAB_EMPTY){
			mmheap->slabC->current->state  = SLAB_PARTIAL;
			set_bit(mmheap->slabC->current->bitmap->map, mmheap->slabC->current->bitmap->free);
			mmheap->slabC->current->bitmap->free++;
			mmheap->slabC->current->ct++;
			addr = mmheap->slabC->current->start + (mmheap->slabC->current->bitmap->free * mmheap->slabC->current->obj_size);
		}
		else if(mmheap->slabC->current->state == SLAB_PARTIAL){
			allocC_t *C = (allocC_t*) mmheap->slabC->current->start + (mmheap->slabC->current->bitmap->free * mmheap->slabC->current->obj_size);
			mmheap->slabC->current->ct++;
			if(mmheap->slabC->current->ct == mmheap->slabC->current->num_objs){
				mmheap->slabC->current->state  = SLAB_FULL;
				//set_bit(mmheap->bitmap->map, mmheap->bitmap->free);
				//mmheap->bitmap->free++;
			}
			set_bit(mmheap->slabC->current->bitmap->map, mmheap->slabC->current->bitmap->free);
			mmheap->slabC->current->bitmap->free++;
			addr = mmheap->slabC->current->start + (mmheap->slabC->current->bitmap->free * mmheap->slabC->current->obj_size);
		}
		else if(mmheap->slabC->current->state == SLAB_FULL){
			slab_t *slab = (slab_t*) mmheap->start + (mmheap->bitmap->free * PAGE_SIZE) + (PAGE_SIZE - sizeof(slab_t));
			slab->start = mmheap->start + (mmheap->bitmap->free * PAGE_SIZE);
			slab->bitmap = (bitmap_t*) malloc(sizeof(bitmap_t));
			slab->bitmap->free = 0;
			slab->bitmap->size = 126;
			slab->bitmap->map = (word_t*) calloc(16, sizeof(word_t));
			slab->ct = 0;
			slab->real_size = 16;
			slab->obj_size = 32;
			slab->num_objs = 126;
			slab->state = SLAB_EMPTY;
			slab->prev = mmheap->slabC->current;
			mmheap->slabC->current->next = slab;
			mmheap->slabC->current = slab;
			mmheap->slabC->ct++;
			addr = mmheap->slabC->current->start + (mmheap->slabC->current->bitmap->free * mmheap->slabC->current->obj_size);
		}
	}
	else{
		return NULL;
	}
	

	return addr;	
}


/**********************************************************************

    Function    : my_free
    Description : deallocate from slabs
    Inputs      : buf: full pointer (with counter) to deallocate
    Outputs     : address if success, NULL on error

***********************************************************************/

void my_free( void *buf )
{
	// TASK 2: Implement free function for slab allocator

	return;
}


/**********************************************************************

    Function    : canary_init
    Description : Generate random number for canary - fresh each time 
    Inputs      : 
    Outputs     : void

***********************************************************************/

void canary_init( void )
{ 
	// This program will create different sequence of  
	// random numbers on every program run  
  
	canary = 0;   // fix this 
	printf("canary is %d\n", canary );
} 


/**********************************************************************

    Function    : check_canary
    Description : Find canary for obj and check against program canary
    Inputs      : addr: address of object
                  size: size of object to find cache
    Outputs     : 0 for success, -1 for failure

***********************************************************************/

int check_canary( void *addr)
{
	// TASK 3: Implement canary defense

	return 0;
}


/**********************************************************************

    Function    : check_type
    Description : Verify type requested complies with object 
    Inputs      : addr: address of object
                  type: type requested
    Outputs     : 0 on success, -1 on failure

***********************************************************************/

int check_type( void *addr, char type ) 
{
	// TASK 3: Implement type confusion defense

	return 0;
}


/**********************************************************************

    Function    : check_count
    Description : Verify that pointer count equals object count
    Inputs      : addr: address of pointer (must include metadata in pointer)
    Outputs     : 0 on success, or -1 on failure

***********************************************************************/

int check_count( void *addr ) 
{
	// TASK 3: Implement free count defense

	return 0;
}



/**********************************************************************

    Function    : set/clear/get_bit
    Description : Bit manipulation functions
    Inputs      : words: bitmap 
                  n: index in bitmap
    Outputs     : cache if success, or NULL on failure

***********************************************************************/

void set_bit(word_t *words, int n) {
	words[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void clear_bit(word_t *words, int n) {
	words[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

int get_bit(word_t *words, int n) {
	word_t bit = words[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
	return bit != 0;
}


/**********************************************************************

    Function    : print_cache_slabs
    Description : Print current slab list of cache
    Inputs      : cache: slab cache
    Outputs     : void

***********************************************************************/

int print_cache_slabs( slab_cache_t *cache )
{
	slab_t *slab = cache->current;
	int count=0;
	printf("Cache %p has %d slabs\n", cache, cache->ct );
	do {
		printf("slab: %p; prev: %p; next: %p\n", slab, slab->prev, slab->next );
		count+=1;
		slab = slab->next;
	} while ( slab != cache->current );
	return count;
}


/**********************************************************************

    Function    : get_stats/slab_counts
    Description : Print stats on slab page and object allocations 
    Outputs     : void

***********************************************************************/

void slab_counts( slab_cache_t *cache, unsigned int *slab_count, unsigned int *object_count ){
	slab_t *slab = cache->current;
	int i;
	unsigned int orig_count;
	
	*slab_count = 0;
	*object_count = 0;

	if ( slab == NULL ) {
                return;
        }

	do {
		(*slab_count)++;

		// set orig to test objects per slab
		orig_count = *object_count;

		// count objects in slab
		for ( i = 0; i < slab->bitmap->size ; i++ ) {
			if ( get_bit( slab->bitmap->map, i )) {
				(*object_count)++;
			}
		}

		if (( *object_count - orig_count ) != slab->ct ) {
			printf("*** Discrepancy in object count in slab %p: %d:%d\n", 
			       slab, *object_count - orig_count, slab->ct);
		}
			

		slab = slab->next;
	} while ( slab != cache->current );

	if ( *slab_count != cache->ct ) {
		printf("*** Discrepancy in slab page count in cache %p: %d:%d\n", cache, *slab_count, cache->ct);
	}
}

void get_stats(){
	unsigned int slab_count, object_count;

	printf("--- Cache A ---\n");
	slab_counts( mmheap->slabA, &slab_count, &object_count );
	printf("Number of slab pages:objects in Cache A: %d:%d\n", slab_count, object_count );
	printf("--- Cache B ---\n");
	slab_counts( mmheap->slabB, &slab_count, &object_count );
	printf("Number of slab pages:objects in Cache B: %d:%d\n", slab_count, object_count );
	printf("--- Cache C ---\n");
	slab_counts( mmheap->slabC, &slab_count, &object_count );
	printf("Number of slab pages:objects in Cache C: %d:%d\n", slab_count, object_count );
}
