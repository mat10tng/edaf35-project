#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define META_SIZE sizeof(struct s_block)
#define MIN_SIZE 8
#define VALUE_CHECK(x)    do {if(!x) return NULL;} while(0)
typedef struct s_block* t_block;

// base pointer to the starting point of heap
void* 	base;

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

	if ( (b = sbrk(META_SIZE + size)) == (void*) -1   ) 
		return NULL;

	b->size 	= size;
	b->next 	= NULL;

	if ( last )
		last->next	= b;

	b->free 	= 0;
	return b;
}
void* malloc ( size_t size ) 
{
	t_block		b;
	t_block		last;

	if ( base ) {
		last = base ;
		b = find_block( &last , size );
		if ( b )  {
			b->free = 0;
		} else {  
			b = extend_heap ( last, size );
			VALUE_CHECK(b);
		}
	} else {
		b = extend_heap ( NULL , size );
		VALUE_CHECK(b);
		base = b;
	}
	return b+1;
}

void* calloc ( size_t number, size_t size ) 
{
	void* new;
	new = malloc(number*size);	
	VALUE_CHECK(new);
	memset(new,0,number*size);
	return new;
}
void free( void* ptr)
{
	t_block b;
	if(!ptr)
		return;
	b =(char*) ptr - META_SIZE;
	b->free = 1;

}
void* realloc( void* src, size_t size)
{
	void* dst;
	t_block b;
	
	if(!src)
		return malloc(size);

	b = src - META_SIZE;
	 
	if(size == 0){
		free(src);
		return NULL;
	}
	
	if( b->size  >= size)
		return src;

	dst = malloc(size);
	VALUE_CHECK(dst);
	free(src);
	return memcpy(dst,src,b->size);

}
