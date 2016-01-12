#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define META_SIZE sizeof(struct s_block)
#define MAX_KVAL 24
#define VALUE_CHECK(x)   do{if(!x) return NULL;}while(0)

typedef struct 	s_block* t_block;

static char*		base_addr = NULL;
static t_block		freelist[MAX_KVAL+1] ={NULL};

struct s_block {
	unsigned 	reserved:1; 	/* one if reserved*/
	unsigned char	kval;		/* current value of k*/
	t_block 	succ;		/* successor block */
	t_block 	pred;		/* predessor block */
};

t_block free_node(t_block current)
{

	if(freelist[current->kval] == current){
		freelist[current->kval] = current->succ;
	}
		
	if(current->succ)
		current->succ->pred = current->pred;
		
	if(current->pred)
		current->pred->succ = current->succ;
		
	current->succ = current->pred = NULL;
/**	
	if(freelist[current->kval])
		freelist[current->kval]->pred = NULL;
*/	
	return current;

}

void push_node(t_block current, size_t kval){

	current->reserved = 0;
	current->pred = NULL;
	current->succ = NULL;
	current->kval = kval;
	

	if(freelist[kval]){	
		current->succ = freelist[kval];
		freelist[kval]->pred = current;
	}
	
	freelist[kval] = current;

}

void split_block(size_t kval)
{
	t_block child_1, child_2;	
	
	child_1 = free_node(freelist[kval]);
	kval--;

	child_2 =(t_block) ( ( (char*) child_1 ) + ( (unsigned) 1<<kval)) ;

	push_node(child_2,kval);
	push_node(child_1,kval);
}

size_t  find_block(size_t kval)
{
	size_t i;
	for( i = kval;  i<= MAX_KVAL; i++){
		if(freelist[i])
			return i;
	}
	return NULL;
}

size_t powertwo(size_t size)
{
	size_t i, j;
	i = 0;
	j = 1;
	while(j<size){
		j<<=1;
		i++;
	}
	return i;
}

void* malloc ( size_t size ) 
{
	t_block b;
	size_t k, j, i;	

	VALUE_CHECK(size);
	
	if(base_addr){
	
		k = powertwo(META_SIZE + size);
		
		j = find_block(k);
		VALUE_CHECK(j);
		
		for(i = j; i > k; --i){
			split_block(i);
		}

		b = free_node(freelist[k]);
		b->reserved = 1;

		return (b+1); 		
	}else{
	
		base_addr = sbrk(1<<MAX_KVAL);		
		VALUE_CHECK(base_addr);
		
		b =(t_block) base_addr;
		push_node(b,MAX_KVAL);

		return malloc(size);
	}
}

void merge(t_block current, t_block buddy){
	while(buddy->reserved == 0 && buddy->kval == current->kval){
		size_t kval = current->kval +1;
		free_node(buddy);
		free_node(current);
		
		if(current < buddy){
			push_node( current, kval);
			current  = current;

		} else {
			push_node( buddy, kval );
			current  = buddy;
		}
		buddy = (t_block) ((size_t)base_addr + (((size_t) (  (char*) current - base_addr))^(1 << current->kval)));
		if (kval == MAX_KVAL)
			return;
	}

}


void free(void* ptr)
{

	t_block current,buddy;
	if(!ptr)
		return;

	current  = (t_block) ( (char*) ptr - META_SIZE);
	
	if(current->reserved == 0)
		return;
		
	if(current->kval == MAX_KVAL){
		push_node(current, MAX_KVAL);
		return;
	}
			
	//buddy = (t_block)  base_addr + (((size_t) (  (char*) current - base_addr))^(1 << current->kval));
	buddy = (t_block) ((size_t)base_addr + (((size_t) (  (char*) current - base_addr))^(1 << current->kval)));
	if(buddy->reserved == 0 && buddy->kval == current->kval)
		merge(current,buddy);
	else
		push_node(current,current->kval);
	
}
void* calloc(size_t number, size_t size)
{
	void* new;
	new = malloc(number*size);
	VALUE_CHECK(new);
	memset(new,0,number*size);
	return new;
}

void* realloc(void* src, size_t size)
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

	if( (((size_t) (1<<b->kval)) - META_SIZE) >= size )
		return src;
	
	dst = malloc(size);
	VALUE_CHECK(dst);
	
	free(src);
	memcpy( dst, src, (size_t) (1<<b->kval) - META_SIZE )  ;
		
	return dst;
}

