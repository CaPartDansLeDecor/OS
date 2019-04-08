#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "mem.h"

int main()
{
    // initialize the allocator
    mem_init();
    mem_show_heap();
    char *p = mem_alloc( 42 ); 
    assert( p != NULL ); // check whether the allocation was successful
    printf("allocated 42 bytes at %p\n", p);
    mem_show_heap();
    mem_release(p);
    
    mem_show_heap();
    /*char *p6 = mem_alloc( 200 );
    assert( p6 != NULL );
    mem_show_heap();
    
    mem_release(p6);
    
    mem_show_heap();*/
    /*char *p3 = mem_alloc( 42 );
    assert( p3!= NULL );
    char *p4 = mem_alloc( 42 );
    assert( p4 != NULL );
    char *p5 = mem_alloc( 42 );
    assert( p5 != NULL );
    char *p7 = mem_alloc( 42 );
    assert( p7 != NULL );
    char *p9 = mem_alloc( 42 );
    assert( p9 != NULL );
    char *p10 = mem_alloc( 42 );
    assert( p10 != NULL );
    char *p11 = mem_alloc( 42 );
    assert( p11 != NULL );
    char *p12 = mem_alloc( 42 );
    assert( p12 != NULL );
    char *p13 = mem_alloc( 42 );
    assert( p13 != NULL );
    char *p14 = mem_alloc( 42 );
    assert( p14 != NULL );
    
    mem_show_heap();
    char *p8 = mem_alloc( 300 );
    assert( p8 != NULL );
    
    mem_show_heap();*/
}
