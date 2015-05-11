#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


// (x-1)/8  * 8 + 8-> give the nearest greater or equal multiple of 8
#define align8 (x) ( ( ( ( ( x ) - 1 ) >> 3 ) << 3 ) + 8 ) 

# define BLOCK_SIZE sizeof(struct s_block)

// a minimum data size to malloc
# define MIN_SIZE 8


typedef struct s_block* t_block;

// base pointer to the starting point of heap
void* 	base;

/**
 *struct -- concatenation of its fields
 * s_block should be 12 bytes (with 32 bit integer)
 * first four - size 
 * next four - pointer to next block
 * last four - integer
 */
struct s_block {
	size_t		size;
	t_block		next;
	t_block 	prev;
	int		free;
};

t_block find_block ( t_block* last, size_t size ) 
{
	t_block 		b = base;

	while ( b && ! ( b->free && b->size >=size)) {

		*last = b;

		b = b->next;
	}
	
	return b;
}

t_block extend_heap ( t_block last, size_t size )
{
	t_block		b;
	b = sbrk(0) ;

	if ( (void* -1 == sbrk ( BLOCK_SIZE + size ) ) ) 
		return NULL;

	b->size 	= s;
	b->next 	= NULL;

	if ( last )
		last->next	= b;

	b->free 	= 0;

	return b;
}

void split_block ( t_block b , size_t size ) 
{
	t_block		new;

	new			= b->data + size ;
	new->size	= b->size - s - BLOCK_SIZE ;
	new->next 	= b->next ;
	new->free 	= 1 ;

	b->size 		= size;
	b->next 		= new;
}

t_block fusion ( t_block b )
{
	if ( b->next &&  b->next->free ) {
		
		b->size 	+= BLOCK_SIZE + b->next->size;
		b->next		 = b->next->next;
		
		if ( b->next )
			b->next->prev = b;
	}
	return b;
}				

void* malloc ( size_t size ) 
{
	t_block		b;
	t_block		last;
	size_t		s;
	
	s 	= align8(size);

	/* There is a base */
	if ( base ) {
		/* Find a block start from base */
		last = base ;
		b = find_block( &last , s );

		/* any block that work ? */
		if ( b )  {
			/* split ? */
			if ( ( BLOCK_SIZE + MIN_SIZE ) <= ( b->size - s ) )
				split_block( b , s );

			b->free = 0;
		} else {  
			/* no block that work, we extend */
			b = extend_heap ( last, s );

			if ( !b )
				return NULL
		}
	} else {
		/* first time */
		b = extend_heap ( NULL , s );
		if ( !b ) 
			return NULL;
		base = b;
	}
	return b->data;
}

void* calloc ( size_t number, size_t size ) 
{
	size_t*	new;
	size_t		s8;
	size_t		i;
	
	new = malloc ( number * size );
	if ( new ) {
		s8 = align8(number * size) << 3
	}
}
			
