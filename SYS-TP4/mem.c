#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>

#include "mem.h"

void * heap_base = NULL;// first byte of the heap
void * heap_end  = NULL;// first byte beyond the heap

int mem_initialized = 0;

// initialize the memory manager
void mem_init(void)
{
    assert(mem_initialized == 0);

    // request memory from the kernel
    heap_base = mmap(NULL, 800, PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    assert(heap_base != MAP_FAILED);

    heap_end = heap_base + 800 ;
    void *block_ptr;
    block_ptr = heap_base;
    *( (int64_t*)block_ptr ) = 800;
    int64_t header = *( (int64_t*)block_ptr );
    char flags  = 0b000;   
    int64_t size = 800;
    header = size | flags ;
    
    *( (int64_t*)block_ptr ) = header ;
    *( (int64_t*)(block_ptr+792) ) = header ;
    // create some free blocks: five of 80 bytes and one of 400 bytes
    /*void *block_ptr;
    for(int i=0; i<5;i++)
    {
        block_ptr = heap_base + 80*i;
        *( (int64_t*)block_ptr ) = 80;
    }
    block_ptr = heap_base;*/
    
    mem_initialized = 1;
}

void * mem_alloc(int64_t length)
{
    assert(mem_initialized == 1);
    
    // compute actual size of block
    length = (length+7)/8*8 ; // round up to nearest multiple of 8
    length += 16;              // add space for the header

    // heap traversal
    void *  block_ptr ;
    void *  newBlock_ptr ;
    int64_t header,footer ;
    int64_t size;
    int64_t newBlockLength;
    char    flags;
    
    block_ptr = heap_base;
    while(block_ptr < heap_end)
    {
        header = *( (int64_t*)block_ptr );
        flags  = header & 0b111;  // keep only three least significant bits
        size = header & ~0b111;   // discard the three least significant bits
        footer = *( (int64_t*)(block_ptr+size-8) );
        if( (flags == 0 ) &&      // if current block is free, and
            (size >= length))     // is also large enough, then we have a winner
            break;

        // otherwise we move on to the next block
        block_ptr += size;
    }

    // if the heap  traversal reached this far, then it  means we have
    // found no suitable block, so we should return NULL
    if(block_ptr >= heap_end)
    {
        return NULL;
    }
    if(length < size){
      printf("length : %ld, size : %ld",length,size);
      newBlockLength = size - length;
      size = length;
      newBlock_ptr = block_ptr + size;
      *( (int64_t*)newBlock_ptr ) = newBlockLength;
      //int64_t header = *( (int64_t*)newBlock_ptr );
      char flags  = 0b000;
      int64_t newHeader = newBlockLength | flags ;
      *( (int64_t*)newBlock_ptr ) = newHeader ;
      *( (int64_t*)(newBlock_ptr+newBlockLength - 8) ) = newHeader ;
    }
    flags = 0b001; // mark block as taken
    header = size | flags;
    footer = header;
    *( (int64_t*)block_ptr ) = header ; // write header back into the block
    *( (int64_t*)(block_ptr+size-8) ) = footer ; // write header back into the block
    return block_ptr + 8 ; // skip header
}

void mem_release(void *ptr)
{
    assert( mem_initialized == 1);
    assert( ((int64_t)ptr % 8) == 0 ); // sanity check
    
    int64_t header,footer,footerP,headerS;
    int64_t size,sizeP,sizeS;   
    char    flags,flagP,flagS;
    
    ptr -= 8;
    
    header = *( (int64_t*)ptr );
    size = header & ~0b111;
    bool base = 1;
    if((long int)ptr != (long int)heap_base){
      footerP = *( (int64_t*)(ptr-8) );
      flagP = footerP & 0b111;
      base = 0;
    } else {
      flagP = 0b001;
    }
    headerS = *( (int64_t*)(ptr+size));
    flagS = headerS & 0b111;
    flags = 0b000;
    printf("suivant : %p\n",ptr+size);
    printf("flag S : %d flag P : %d base : %d",flagS,flagP,base);
    
    if(!flagS && flagP ){
      sizeS = headerS & ~0b111;
      header = (sizeS + size) | flags;
      *( (int64_t*)ptr ) = header ; // write header back into the block
      *( (int64_t*)(ptr+sizeS+size-8)) = header ; // write footer back into the block
    } else if(flagS && !flagP && !base){
      sizeP = footerP & ~0b111;
      header = (size+sizeP) | flags;
      *( (int64_t*)(ptr-sizeP) ) = header ; // write header back into the block
      *( (int64_t*)(ptr+size-8)) = header ; // write footer back into the block
    } else if(!flagS && !flagP && !base){
      sizeS = headerS & ~0b111;
      sizeP = footerP & ~0b111;
      header = (size+sizeP+sizeS) | flags;
      *( (int64_t*)(ptr-sizeP) ) = header ;
      *( (int64_t*)(ptr+sizeS - 8) ) = header ;
    } else {
      footer = *( (int64_t*)(ptr+size-8));
      header = size | flags;
      footer = header;
      *( (int64_t*)ptr ) = header ; // write header back into the block
      *( (int64_t*)(ptr+size-8)) = footer ; // write footer back into the block
      
    }
    return;
}

void mem_show_heap(void)
{
    assert( mem_initialized == 1);
    
    void * block_ptr = heap_base;

    printf("heap_base = %p\n",heap_base);
    while(block_ptr < heap_end)
    {
        int64_t header = *( (int64_t*)block_ptr );
        char    flags  = header & 0b111;  //   keep only three least significant bits
        int64_t size   = header & ~0b111; // discard the three least significant bits
        if( (size < 8) ||
            (size%8 != 0) )
        {
            printf("error: block at %p has incorrect size %ld\n",block_ptr,size);
            exit(1);
        }
        int64_t footer = *( (int64_t*)(block_ptr+size-8) );
        printf("  block at %p: header=0x%08lx footer =0x%08lx size=%ld flags=%d (%s)\n",
               block_ptr,header,footer,size,flags,
               (flags ? "taken" : "free")
               );

        block_ptr += size; // move on to next block
    }

    printf("heap_end = %p\n",heap_end);

    //sanity check: a full heap traversal should reach *exactly* the end
    assert( block_ptr == heap_end); 
}
