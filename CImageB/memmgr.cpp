//////////////////////////////////////////////////////////////////////////
//	memmgr.cpp
//
//	Date	12/22/2014			Work New
//
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <malloc.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
#define ALLOCATOR_CHECK_ON	(1)

#if ALLOCATOR_CHECK_ON 

typedef struct tagST_MAllocInfo
{
	struct tagST_MAllocInfo* prev;
	void* buff;
	struct tagST_MAllocInfo* next;	
	size_t size;
	int number;
}ST_MAllocInfo;

ST_MAllocInfo* ghead = NULL;
ST_MAllocInfo* gtail = NULL;
int g_nALNum = 0;

//////////////////////////////////////////////////////////////////////////
void AllocatorInit()
{
	ghead = NULL;
	gtail = NULL;
	g_nALNum = 0;
	return;
}

//////////////////////////////////////////////////////////////////////////
void AllocatorTerm()
{
	ST_MAllocInfo* node;

	if (ghead == NULL){
		return;
	}
	printf("\nMemory leak detected!");
	while(ghead != NULL)
	{
		free(ghead->buff);
		printf("\nChain Num = %d", ghead->number);
		node = ghead;		
		ghead = ghead->next;
		free(node);
	}

	ghead = NULL;
	gtail = NULL;
	g_nALNum = 0;
	return;
}

//////////////////////////////////////////////////////////////////////////
void* alloc_mem(size_t size)
{
	size_t rn;
	unsigned char* w_pbBuff1 = NULL;
	ST_MAllocInfo* node = NULL;

	if (size < 1){
		return NULL;
	}

	rn = sizeof(ST_MAllocInfo*) + size;
	w_pbBuff1 = (unsigned char*)malloc(rn);
	if (w_pbBuff1 == NULL){
		return NULL;
	}
	node = (ST_MAllocInfo*)malloc(sizeof(ST_MAllocInfo));	
	if (node == NULL){
		return NULL;
	}
	memset(node, 0, sizeof(ST_MAllocInfo));
	node->size = size;

	*(ST_MAllocInfo**)w_pbBuff1 = node;
	node->number = g_nALNum;
	node->buff = w_pbBuff1;
	w_pbBuff1 += sizeof(ST_MAllocInfo*);

// 	if (g_nALNum == 9){
// 		rn ++;
// 		rn --;
// 	}

	if (ghead == NULL)
	{
		ghead = node;
		gtail = node;
	}
	else
	{
		node->prev = gtail;
		gtail->next = node;
		gtail = node;
	}	
	g_nALNum ++;
	return (void*)w_pbBuff1;
}

//////////////////////////////////////////////////////////////////////////
void free_mem(void* ptr)
{
	ST_MAllocInfo* node;
	ST_MAllocInfo* prev1;
	ST_MAllocInfo* next1;

	node = *(ST_MAllocInfo**)((unsigned char*)ptr - sizeof(ST_MAllocInfo*));
	if (node == NULL){
		printf("\nfree_mem :FAILED!");
		return;
	}
	prev1 = node->prev;
	next1 = node->next;
	if (prev1 == NULL)
	{
		if (node != ghead){
			printf("\nfree_mem :FAILED!");
			return;
		}
		ghead = next1;
	}
	else
	{
		prev1->next = next1;
	}
	if (next1 == NULL)
	{
		if (node != gtail){
			printf("\nfree_mem :FAILED!");
			return;
		}
		gtail = prev1;
	}
	else
	{
		next1->prev = prev1;
	}
	free(node->buff);
	free(node);
	return;
}

size_t GetSizeAAA(void* ptr)
{
	ST_MAllocInfo* node;
	node = *(ST_MAllocInfo**)((unsigned char*)ptr - sizeof(ST_MAllocInfo*));
	return node->size;
}

void* realloc_mem(void* ptr, size_t size)
{
	void* w_p1 = alloc_mem( size );
	size_t size0;

	size0 = GetSizeAAA(ptr);
	if (size0 > size){
		size0 = size;
	}
	memcpy(w_p1, ptr, size0);
	free_mem(ptr);
	return w_p1;
}

void* calloc_mem(size_t step, size_t size)
{
	void* w_p1 = alloc_mem( step * size );
	memset(w_p1, 0, step * size );
	return w_p1;
}

#else

void AllocatorInit()
{
	return;
}

void AllocatorTerm()
{
	return;
}

void* alloc_mem(size_t size)
{
	return malloc(size);
}

void free_mem(void* ptr)
{
	free(ptr);
}

void* realloc_mem(void* ptr, size_t size)
{
	return realloc(ptr, size);
}

void* calloc_mem(size_t step, size_t size)
{	
	return calloc(step, size);
}

#endif