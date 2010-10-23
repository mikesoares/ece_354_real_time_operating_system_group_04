#ifndef _RTX_MEM_H
#define _RTX_MEM_H

#define MEM_BLOCK_SIZE 128
#define MEM_BLOCK_COUNT 30

extern void init_memory();
extern UINT32 get_block_count();
extern VOID reassign_mem_block (void * block, int newPid);
extern int release_memory_block (void * MemoryBlock);
extern void * request_memory_block();
extern UINT32 are_blocks_available();

//debug print queues
void print_availible_mem_queue ();
void print_used_mem_queue();

#endif

