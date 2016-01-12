#define _BSD_SOURCE 
#include <sys/types.h> 
#include <unistd.h> 
#include <string.h>

#define META_SIZE sizeof(struct s_block)
#define MAX_KVAL 24
#define VALUE_CHECK(x) do{if(!x) return NULL;}while(0)

typedef struct 	s_block* t_block;

static t_block		free_list[MAX_KVAL];
static char*		base_addr = NULL;

struct s_block {
	unsigned 	reserved:1; 	/* one if reserved*/
	unsigned char	kval;		/* current value of k*/
	t_block 	succ;		/* successor block */
	t_block 	pred;		/* predessor block */
};

void* get_buddy(void* ptr, size_t kval)
{
	 return base_addr + (((unsigned) (  (char*) ptr -(char*) base_addr))^(1 << kval));
}

void split_block(size_t kval)
{
	t_block child_1, child_2;	
	/* handle split mess */	
	child_1 = free_list[kval];
	free_list[kval] = child_1->succ;
	
	kval--;
	
	child_2 = (t_block) get_buddy(child_1, kval);
	
	child_1->reserved = child_2->reserved = 0;
	child_1->kval = child_2->kval = kval;

	child_1->succ = child_2;
	child_2->pred = child_1;
	
	free_list[kval] = child_1;
	
}

size_t  find_block(size_t kval)
{
	size_t i;
	for( i = kval;  i<= MAX_KVAL; i++){
		
		if(free_list[i])
			return i;
			
	}
	return 0;
}

size_t logtwo(size_t size)
{
	size_t kval = 0;
	while( size >>= 1) {
		kval++;
	}
	return kval;
}
	
void* malloc ( size_t size ) 
{
	t_block b;
	size_t K, J, I;	

	if(base_addr){
		/* increase size to be the smallest power of two */
		K = logtwo(META_SIZE + size);
		
		/* find suitable block */
		J = find_block(K);
		VALUE_CHECK(J);
		
		/* split all the block */
		for(I = J; I > K; --I){
			split_block(I);
		}
		
		b = free_list[K];
		b->reserved = 1;
		free_list[K] = b->succ;		
		return ( b + 1); 		
	}else{
	/* first time execute */
		base_addr = sbrk(1<<MAX_KVAL);
		VALUE_CHECK(base_addr);

		b =(t_block) base_addr;
		b->kval = MAX_KVAL;
		b->succ = NULL;
		b->pred = NULL;
		b->reserved=0;
		free_list[MAX_KVAL] = b;
		return malloc(size);
	}
}

void add_block(t_block block){
	if(free_list[block->kval]){
		block->succ = free_list[block->kval];
		free_list[block->kval]->pred = block;
	}
	free_list[block->kval] = block;
		
}
void free_link(t_block current)
{
	if(free_list[current->kval] == current)
		free_list[current->kval] = current->succ;
	if(current->succ)
		current->succ->pred = current->pred;
	if(current->pred)
		current->pred->succ = current->succ;
}

void free(void* ptr)
{

	t_block current,buddy;
	if(!ptr)
		return;
	current  = (t_block) ( ((char*) ptr) - META_SIZE );
	buddy = get_buddy(current, current->kval);
	current->reserved = 0;

	if(buddy->reserved == 0 && buddy->kval == current->kval){
		free_link(buddy);
		
		current->kval+=1;
		current->pred = NULL;

		add_block(current);
		free(current +1 );
	}else{
		add_block(current);
	}

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
	b = (t_block) ((char*)src - META_SIZE);
	if(size == 0){
		free(src);
		return NULL;
	}
	if(  (1<<b->kval) >= size )
		return src;
	
	dst = malloc(size);
	VALUE_CHECK(dst);
	free(src);
	return memcpy(dst,src,1<<b->kval);
}
