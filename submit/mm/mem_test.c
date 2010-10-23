#include "../shared/rtx_inc.h"
#include "../shared/dbug.h"
#include "../shared/rtx_core.h"
#include "../shared/rtx_mem.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_pids.h"

void mem_test1 () {
	rtx_dbug_outs("Entering mem_test1\r\n");

	//print_availible_mem_queue();
	//print_used_mem_queue();
	
	void * block = request_memory_block();
	
	//print_availible_mem_queue();
	//print_used_mem_queue();
	
	rtx_dbug_outs("Requested Block\r\n");
	
	int success = release_memory_block(block);
	
	//print_availible_mem_queue();
	//print_used_mem_queue();
	
	if (success == 0) {
		rtx_dbug_outs("Release Block Failed\r\n");
	} else {
		rtx_dbug_outs("Release Block Success!\r\n");
	}
}


//This function requests all blocks, realeases all of them (in order requested),
//and tries to request another block (should succeed)
void mem_test2 () {
	rtx_dbug_outs("Starting Memory Test 2\r\n");

	void * testArray[32];
	
	rtx_dbug_outs("Requesting all memory blocks\r\n");
	
	//request all memory blocks
	int i;
	for (i = 0; i < 32; i++) {
		testArray[i] = request_memory_block();
	}	
	
	rtx_dbug_outs("All memory blocks requested\r\n");
	
	
	//print_availible_mem_queue();
	//print_used_mem_queue();
	
	
	//release all memory blocks
	for (i = 0; i < 32; i++) {
		release_memory_block(testArray[i]);
	}
		
	rtx_dbug_outs("All blocks released\r\n");
	
	//print_availible_mem_queue();
	//print_used_mem_queue();	
	
	//request one more block
	void * oneMore;
	oneMore = request_memory_block();
	
	rtx_dbug_outs("One more block given\r\n");
	
	//release last one
	release_memory_block(oneMore);
	
	rtx_dbug_outs("Finished Memory Test 2\r\n");

}

//This function requests all blocks, realeases all of them (in backwards order),
//and tries to request another block (should succeed)
void mem_test3 () {
	rtx_dbug_outs("Starting Memory Test 3\r\n");
	
	void * testArray[32];
	
	rtx_dbug_outs("Requesting all memory blocks\r\n");
	
	//print_availible_mem_queue();
	//print_used_mem_queue();	
	
	//request all memory blocks
	int i;
	for (i = 0; i < 32; i++) {
		testArray[i] = request_memory_block();
	}
	
	rtx_dbug_outs("All memory blocks requested\r\n");
	
	
	//print_availible_mem_queue();
	//print_used_mem_queue();		
	
	//release all memory blocks
	for (i = 31; i >= 0; i--) {
		release_memory_block(testArray[i]);
	}
	
	rtx_dbug_outs("All blocks released\r\n");
	
	
	//print_availible_mem_queue();
	//print_used_mem_queue();	
	
	
	//request one more block
	void * oneMore;
	oneMore = request_memory_block();
	
	rtx_dbug_outs("One more block given\r\n");
	
	//release last one
	release_memory_block(oneMore);
	
	rtx_dbug_outs("Finished Memory Test 3\r\n");
	
}

//Proc 1 of a two processor test
//These tests will check to see if a block due to memory is handled right
void mem_test4() {
	rtx_dbug_outs("Starting Memory Test 4\r\n");
	
	rtx_dbug_outs("Requesting one block\r\n");
	
	//print_availible_mem_queue();
	//print_used_mem_queue();	
	
	//request 1 memory block
	void * oneBlock = request_memory_block();
	
	rtx_dbug_outs_int("Block recieved. Block address: \r\n", (int)oneBlock);
	rtx_dbug_outs("Releasing proccessor in Memory Test 4\r\n");
	
	//release the processor (to the other test)
	release_processor();
	
	rtx_dbug_outs("Restarting Memory Test 4\r\n");
	
	//print_availible_mem_queue();
	//print_used_mem_queue();	
	
	//release the memory block
	release_memory_block(oneBlock);
	
	rtx_dbug_outs("Memory Block Got Released\r\n");
	rtx_dbug_outs("Finished Memory Test 4\r\n");

}

//Second part to the test (Test 4)
void mem_test5() {
	rtx_dbug_outs("Starting Memory Test 5\r\n");
	
	//request 31 blocks of mem (Mem should now fill up)
	void * testArray[32];
	
	//request 31 memory blocks
	int i;
	for (i = 0; i < 31; i++) {
		testArray[i] = request_memory_block();
	}
	
	//print_availible_mem_queue();
	//print_used_mem_queue();	
	
	rtx_dbug_outs("Requested 31 blocks. Requesting one more\r\n");
	
	//this should get blocked
	testArray[31] = request_memory_block();
	
	//PROC 4 SHOULD HAPPEN BEFORE THIS AND SHOULD RELEASE IT'S MEMORY
	rtx_dbug_outs_int("Got Block! Address is: \r\n", (int)testArray[31]);
	
	//release all memory blocks
	for (i = 0; i < 32; i++) {
		release_memory_block(testArray[i]);
	}
	
	//print_availible_mem_queue();
	//print_used_mem_queue();	
	
	rtx_dbug_outs("Finished Memory Test 5\r\n");
	
}

//this tests removing arbitrary blocks from used mem queue
void mem_test6() {
	rtx_dbug_outs("Starting Memory Test 6\r\n");
	
	//request half of the blocks
	void * testArray[16];
	
	//print_availible_mem_queue();
	//print_used_mem_queue();	
	
	int i;
	for (i = 0; i < 16; i++) {
		testArray[i] = request_memory_block();
	}

	rtx_dbug_outs("Blocks requested\r\n");
	
	//remove some from the middle
	release_memory_block(testArray[4]);
	release_memory_block(testArray[10]);
	
	//print_availible_mem_queue();
	//print_used_mem_queue();	
	
	//release all memory blocks
	for (i = 0; i < 16; i++) {
		if (i != 4 && i != 10) {
			release_memory_block(testArray[i]);
		}
	}
	
	rtx_dbug_outs("Finished Memory Test 6\r\n");
	
}

//this function tests the memory zeroing funcution
void mem_test7() {
	rtx_dbug_outs("Starting Memory Test 7\r\n");
	
	//request a memory block
	void * block = request_memory_block();
	
	rtx_dbug_outs("Writing one's to the block\r\n");
	
	//write ones to all mem in the block
	BYTE * start = (BYTE *)(block);
	int i;
	for (i = 0; i < MEM_BLOCK_SIZE; i++) {
		*(start + i) = 0x0001;
	}
	
	rtx_dbug_outs("Wrote one's\r\n");
	
	//print out values
	for (i = 0; i < MEM_BLOCK_SIZE; i++) {
		rtx_dbug_outs_int("BLOCK VALUE: ", *(int*)(start + i));
	}
	
	//release memory block
	release_memory_block(block);
	
	
	rtx_dbug_outs("Writing zeros\r\n");

	
	//print out values
	for (i = 0; i < MEM_BLOCK_SIZE; i++) {
		rtx_dbug_outs_int("BLOCK VALUE: ", *(int*)(start + i));
	}
	
	rtx_dbug_outs("Finished Memory Test 7\r\n");
	
}

// lowest priority
void mem_test_master()
{
	/**
	 * Memory blocking test
	 * 
	 * This test consumes all of the memory, then wakes up 2 processes of different
	 * priorities which both request_memory_block(), blocking them both. When this
	 * process returns, it frees a single memory block and verifies that it was given
	 * to the right process.
	 */
	void * msg1 = request_memory_block();
	void * msg2 = request_memory_block();
	
	void * blocks[30];

	// starve the system of memory
	int i;
	for (i = 0; are_blocks_available(); i ++) {
		blocks[i] = request_memory_block();
	}

	for (; i < 30; i ++) {
		blocks[i] = NULL;
	}

	#ifdef _MEM_TEST_DEBUG
	if (!is_process_blocked(MEM_TEST_BLOCKING1_PID)) {
		rtx_dbug_outs("MEM TEST ERROR: Blocking1 is not blocked for msg\r\n");
	}

	if (!is_process_blocked(MEM_TEST_BLOCKING2_PID)) {
		rtx_dbug_outs("MEM TEST ERROR: Blocking2 is not blocked for msg\r\n");
	}
	#endif


	// wake up blocking1 and blocking2
	send_message(MEM_TEST_BLOCKING1_PID, msg1);
	send_message(MEM_TEST_BLOCKING2_PID, msg2);

	#ifdef _MEM_TEST_DEBUG
	if (!is_process_blocked(MEM_TEST_BLOCKING1_PID)) {
		rtx_dbug_outs("MEM TEST ERROR: Blocking1 is not blocked for mem\r\n");
	}

	if (!is_process_blocked(MEM_TEST_BLOCKING2_PID)) {
		rtx_dbug_outs("MEM TEST ERROR: Blocking2 is not blocked for mem\r\n");
	}
	#endif

	// both will block and we'll end up back here
	release_memory_block(blocks[0]);

	MessageEnvelope * msgFromB2 = (MessageEnvelope *)receive_message(NULL);

	if (msgFromB2->tx != MEM_TEST_BLOCKING2_PID) {
		#ifdef _MEM_TEST_DEBUG
		rtx_dbug_outs_int("MEM TEST ERROR: Incorrect process was unblocked on memory (1) ", msgFromB2->tx);
		#endif
	} else {
		#ifdef _MEM_TEST_DEBUG
		rtx_dbug_outs("MEM TEST SUCCESS: Correct process unblocked on memory (1)\r\n");
		#endif
	}

	release_memory_block(msgFromB2);

	MessageEnvelope * msgFromB1 = (MessageEnvelope *)receive_message(NULL);

	if (msgFromB1->tx != MEM_TEST_BLOCKING1_PID) {
		#ifdef _MEM_TEST_DEBUG
		rtx_dbug_outs_int("MEM TEST ERROR: Incorrect process was unblocked on memory (2) ", msgFromB1->tx);
		#endif
	} else {
		#ifdef _MEM_TEST_DEBUG
		rtx_dbug_outs("MEM TEST SUCCESS: Correct process unblocked on memory (2)\r\n");
		#endif
	}
	// cleanup
	release_memory_block(msgFromB1);

	for (i = 1; blocks[i] != NULL; i ++) {
		release_memory_block(blocks[i]);
	}
}

// medium priority
void mem_test_blocking1()
{
	// block this process
	void * msg = receive_message(NULL);

	void * msgBack = request_memory_block();

	send_message(MEM_TEST_MASTER_PID, msgBack);

	// cleanup
	release_memory_block(msg);
}

// highest priority
void mem_test_blocking2()
{
	// block this process
	void * msg = receive_message(NULL);

	void * msgBack = request_memory_block();

	send_message(MEM_TEST_MASTER_PID, msgBack);

	release_memory_block(msg);
}
