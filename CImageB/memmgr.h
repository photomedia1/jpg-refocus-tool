#ifndef __MEM_MGR_H__
#define __MEM_MGR_H__

void AllocatorInit();
void AllocatorTerm();

void* alloc_mem(size_t size);
void free_mem(void* ptr);
void* realloc_mem(void* ptr, size_t size);
void* calloc_mem(size_t step, size_t size);

#endif//..__MEM_MGR_H__
