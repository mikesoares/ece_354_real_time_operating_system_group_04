#include "../shared/rtx_inc.h"
#include "../shared/dbug.h"
#include "../shared/rtx_core.h"
#include "../shared/rtx_mem.h"

struct memBlock {
	void * startBlock;
	struct memBlock * next;
	int pid;
};

int memoryBlockStart;

struct memBlock * void_to_block(void * voidPointer);
void zero_memory (void * block);

//pointers to the start of each queue
struct memBlock * availableBlocks;
struct memBlock * usedBlocks;

UINT32 get_block_number(struct memBlock * block)
{
	return ((UINT32)(block->startBlock) - memoryBlockStart) / MEM_BLOCK_SIZE;
}

//sets the number and size of the memory blocks and initilizes them
VOID init_memory() {
	
	//get memory of specified size for each memblock 
	VOID * blocks = malloc(MEM_BLOCK_SIZE * MEM_BLOCK_COUNT);
	memoryBlockStart = (int)blocks;
	
	//temp pointer
	struct memBlock * tempPoint;
	
	//set head for the availableBlock
	struct memBlock * head_block = malloc(sizeof(struct memBlock));
	availableBlocks = head_block;
	head_block->startBlock = blocks;
	tempPoint = head_block;
		
	int i;
	for (i = 1; i < MEM_BLOCK_COUNT; i++) {
		struct memBlock * mem_blocks = malloc(sizeof(struct memBlock));
		mem_blocks->startBlock = (void *)(blocks + i * MEM_BLOCK_SIZE);
		mem_blocks->pid = -1;
		zero_memory(mem_blocks->startBlock);
		tempPoint->next = mem_blocks;
		tempPoint = mem_blocks;
	}
	
	//set this to null (end of list)
	tempPoint->next = 0;
	
	//this starts as empty
	usedBlocks = NULL;
	
	//Set the ISR to the right mem address
	asm("move.l #asm_request_memory_block,%d0");
	asm("move.l %d0,0x1000008C");
	
	asm("move.l #asm_release_memory_block,%d0");
	asm("move.l %d0,0x10000090");
}

// determine if there are any blocks available
UINT32 are_blocks_available() {
	if (availableBlocks == NULL) {
		return 0;
	}

	return 1;
}

//takes in a pointer to a memory block and reassigns it to the given pid
VOID reassign_mem_block (void * memoryBlock, int newPid) {
	//convert the void pointer
	struct memBlock * block = void_to_block(memoryBlock);
	block->pid = newPid;
	return;
}

//add the provided memBlock struct to the end of the given queue
VOID enqueue_mem_block(struct memBlock ** queue, struct memBlock * block) {
	// if there's nothing in the queue, add the process to the head
	if (*queue == NULL) {
		*queue = block;
		block->next = NULL;
		return;
	}
	
	// otherwise, find the last process in the process queue
	struct memBlock * mem;
	for (mem = *queue; mem->next != NULL; mem = mem->next) {}
	
	// make the currently running process the new last process in the queue
	mem->next = block;
	
	// since process is now at the end of the queue, its next must be NULL
	block->next = NULL;
}

//remove the first memBlock from the specified queue
struct memBlock * dequeue_mem_block(struct memBlock ** queue) {
	if (*queue == NULL) {
		#ifdef _MEM_DEBUG
		rtx_dbug_outs("Cannot dequeue from an empty queue!\n\r");
		#endif
		
		return NULL;
	}
	
	struct memBlock * block = *queue;
	*queue = block->next;
	
	#ifdef _MEM_DEBUG
	rtx_dbug_outs_int("DEQUEUE: ", get_block_number(block));
	#endif
	
	return block;
}

struct memBlock * remove_mem_block(struct memBlock ** queue, struct memBlock * block) {
	//if the queue is empty cannot remove anything
	if (*queue == NULL) {
		#ifdef _MEM_DEBUG
		rtx_dbug_outs("Cannot remove block from an empty queue!\n\r");
		#endif
		
		return NULL;
	}

	if (block == NULL) {
		#ifdef _MEM_DEBUG
		rtx_dbug_outs("Null block pointer passed to remove_mem_block\r\n");
		#endif
	
		return NULL;
	}

	struct memBlock * curBlock;
	struct memBlock * prevBlock = NULL;

	for (curBlock = *queue; curBlock != NULL; curBlock = curBlock->next) {
		if (curBlock == block) {
			
			#ifdef _MEM_DEBUG
			rtx_dbug_outs_int("Current PID: ", get_current_pid());
			rtx_dbug_outs_int("Block owner PID: ", block->pid);
			rtx_dbug_outs_int("Block number: ", get_block_number(block));
			#endif
			
			if (curBlock->pid != get_current_pid()) {
				#ifdef _MEM_DEBUG
				rtx_dbug_outs("Process trying to remove memory block it does not own\r\n");
				#endif
				return NULL;
			}

			// if we're on not the first block, set the next of the previous
			if (prevBlock != NULL) {
				prevBlock->next = curBlock->next;
			} 
			// if we're on the first block, set the head pointer
			else {
				*queue = curBlock->next;
			}

			return curBlock;
		}

		prevBlock = curBlock;
	}

	#ifdef _MEM_DEBUG
	rtx_dbug_outs("Cannot find matching block\n\r");
	#endif
	
	return NULL;
}


void * request_memory_block() {
	
	//push data register to be used onto stack
	asm("MOVE.L %d6,-(%a7)");
	
	//move to supervisor mode
	asm("Trap #3");
	
	//retrieve pointer value from d6
	void * blockPointer;
	asm("MOVE.L %%d6,%[block]" : [block] "=m" (blockPointer));
	
	//restore registers from stack
	asm("MOVE.L (%a7)+,%d6");
	
	return blockPointer;
}

void request_memory_block_handler() {
	
	//turn on atomic mode so interupts won't, well, interupt!
	atomic_up();
	
	struct memBlock * block;
	
	//wait until there are available blocks
	while (availableBlocks == NULL) {
		atomic_down();
		block_process(get_current_pid(), mem);
		//using the asm version of trap up
		asm("Trap #1");
	}

	//get the first block in the available queue
	block = availableBlocks;
	block->pid = get_current_pid();
	
	#ifdef _MEM_DEBUG
	rtx_dbug_outs_int("Giving you memory block number: ", get_block_number(block));
	#endif
	
	//remove from available queue and add to used queue
	dequeue_mem_block(&availableBlocks);
	enqueue_mem_block(&usedBlocks, block);
	
	//move the pointer value onto d6
	asm("MOVE.L %[retBlock],%%d6" : : [retBlock] "m" (block->startBlock));
	
	//turn off atomic mode using the asm trap down
	asm("Trap #2");
	return;
	
}


int release_memory_block (void * MemoryBlock) {
	
	//push data register to be used onto stack
	asm("MOVE.L %d6,-(%a7)");
	
	//move the pointer into d6
	asm("MOVE.L %[block],%%d6" : : [block] "m" (MemoryBlock));

	//move to supervisor mode
	asm("Trap #4");

	//move the value back from d6
	int value;
	asm("MOVE.L %%d6,%[val]" : [val] "=m" (value) );
	
	//restore registers from stack
	asm("MOVE.L (%a7)+,%d6");

	return value;
}

void release_memory_handler() {	
	atomic_up();
	
	//get value from data register
	void * MemoryBlock;
	asm("MOVE.L %%d6,%[mBlock]" : [mBlock] "=m" (MemoryBlock));

	//the usedBlocks queue shouldn't be empty so throw error
	if (usedBlocks == NULL) {
		#ifdef _MEM_DEBUG
		rtx_dbug_outs("usedBlock queue is empty! Cannot remove block\n\r");
		#endif
		
		//put the return value on d6
		asm("MOVE.L %[val],%%d6" : : [val] "i" (1));
		atomic_down();
		return;
	}
	
	//remove the block from queue
	struct memBlock * removedBlock;
	struct memBlock * block = void_to_block(MemoryBlock);
	
	if (block == NULL) {
		#ifdef _MEM_DEBUG
		rtx_dbug_outs("Cannot find memory block\r\n");
		#endif
	
		//return
		asm("MOVE.L %[val],%%d6" : : [val] "i" (2));
		atomic_down();
		return;
	}
	
	removedBlock = remove_mem_block(&usedBlocks, block);
	
	if (removedBlock == NULL) {
		#ifdef _MEM_DEBUG
		rtx_dbug_outs("Removed block is null\r\n");
		#endif
	
		//put the return value on d6
		asm("MOVE.L %[val],%%d6" : : [val] "i" (3));
		atomic_down();
		return;

	}
	
	//zero the mem block and set the pid to 0
	zero_memory(removedBlock->startBlock);
	removedBlock->pid = -1;
	
	//add the removedBlock to the availilbe queue
	enqueue_mem_block(&availableBlocks, removedBlock);
	
	//tell the sceduler that memory is free
	atomic_down();
	unblock_process_by_type(mem);
	
	//put the return value on d6
	asm("MOVE.L %[val],%%d6" : : [val] "i" (0));
	return;	
}


//takes a void pointer and converts it to a memBlock pointer
struct memBlock * void_to_block(void * voidPointer) {
	//search through the available blocks first
	struct memBlock * block = availableBlocks;
	while (block != NULL) {
		if (block->startBlock == voidPointer) {
			return block;
		}
		block = block->next;
	}
	
	//do the same for the used blocks
	block = usedBlocks;
	while (block != NULL) {
		if (block->startBlock == voidPointer) {
			return block;
		}
		block = block->next;
	}

	#ifdef _MEM_DEBUG
	rtx_dbug_outs("Cannot find memBlock for the given void *\r\n");
	#endif
	
	//pointer not found
	return NULL;
	
}

//zero everything in a released block to make debugging cleaner/ easier
void zero_memory (void * block) {
	int i;
	
	for (i = 0; i < MEM_BLOCK_SIZE; i++) {
		*(BYTE *)(block + i) = 0x0000;
	}	
}
	
//prints out the memory blocks in the queues
void print_availible_mem_queue () {
	
	rtx_dbug_outs("------------------\n\r");
	
	//output the available queue first
	rtx_dbug_outs("Availible Queue\n\r");
	
	struct memBlock * queue = availableBlocks;
	while (queue != NULL) {
		//get the memory block number
		int blockNumber = (((int)queue->startBlock - memoryBlockStart) / MEM_BLOCK_SIZE); 
		
		if (queue->pid == -1) {
			rtx_dbug_outs("PID: -1\r\n");
		} else {
			rtx_dbug_outs_int("PID: ", queue->pid);			
		}

		rtx_dbug_outs_int("Block Start: ", blockNumber);
		rtx_dbug_outs(" -> ");

		queue = queue->next;
	}
	
	rtx_dbug_outs("0 \n\r");
	
	rtx_dbug_outs("------------------\n\r");
}

void print_used_mem_queue() {
	
	rtx_dbug_outs("------------------\n\r");
	
	//output the available queue first
	rtx_dbug_outs("Used Queue\n\r");
	
	struct memBlock * queue = usedBlocks;
	while (queue != NULL) {
		if (queue->pid == -1) {
			rtx_dbug_outs("PID: -1\r\n");
		} else {
			rtx_dbug_outs_int("PID: ", queue->pid);			
		}
		
		rtx_dbug_outs_int("Block Start: ", get_block_number(queue));
		rtx_dbug_outs(" -> ");
		queue = queue->next;
	}
	
	rtx_dbug_outs("0 \n\r");
	
	rtx_dbug_outs("------------------\n\r");
	
}

