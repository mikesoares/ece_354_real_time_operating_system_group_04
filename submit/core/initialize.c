#include "../shared/rtx_inc.h"
#include "../test/rtx_test.h"
#include "../shared/rtx_io.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_mem.h"
#include "../shared/rtx_wallclock.h"
#include "../shared/rtx_stress.h"
#include "../shared/dbug.h"
#include "../shared/rtx_pids.h"
#include "../messaging/msg_test.h"
#include "../messaging/msg_test3.h"
#include "../mm/mem_test.h"
#include "core.h"
#include "core_test.h"
#include "priority_switch.h"

struct pib {
	UINT32 pid;
	UINT32 priority;
	VOID (*starting_addr)();
	UINT32 stack_size;
	struct pib * next;
};

struct pib * create_pib(UINT32 pid, UINT32 priority, VOID (*starting_addr)(), UINT32 stack_size)
{
	struct pib * init_block = (struct pib *)malloc(sizeof(struct pib));
	init_block->pid = pid;
	init_block->priority = priority;
	init_block->starting_addr = starting_addr;
	init_block->stack_size = stack_size;
	init_block->next = NULL;
	return init_block;
}

VOID null_process()
{
	while(1) {
	}
}

VOID enqueue_pib(struct pib * block, struct pib ** tail)
{
	(*tail)->next = block;
	*tail = block;
}


VOID initialize_processes()
{
	// get test proc info
	__REGISTER_TEST_PROCS_ENTRY__();
	
	struct pib * init_blocks_head = create_pib(NULL_PID, 4, &null_process, 1024);
	struct pib * init_blocks_tail = init_blocks_head;

/*	
	enqueue_pib(create_pib(MEM_TEST1_PID, 2, &mem_test1, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MEM_TEST2_PID, 2, &mem_test2, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MEM_TEST3_PID, 2, &mem_test3, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MEM_TEST4_PID, 3, &mem_test4, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MEM_TEST5_PID, 3, &mem_test5, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MEM_TEST6_PID, 3, &mem_test6, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MEM_TEST7_PID, 3, &mem_test7, 1024), &init_blocks_tail);
*/
/*
	enqueue_pib(create_pib(MSG_TEST_MASTER_PID, 3, &message_test_master, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MSG_TEST1_PID, 1, &message_test1, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MSG_TEST2_PID, 2, &message_test2, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MSG_TEST5_PID, 1, &message2_test1, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MSG_TEST6_PID, 2, &message2_test2, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MSG_TEST7_PID, 2, &message2_test3, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MSG_TEST8_PID, 2, &message2_test4, 1024), &init_blocks_tail);
*/
/*
	enqueue_pib(create_pib(MSG_TEST3_MASTER_PID, 3, &message_test3_master, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MSG_TEST9_PID, 2, &message3_test1, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MSG_TEST10_PID, 1, &message3_test2, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MSG_TEST11_PID, 2, &message3_test3, 1024), &init_blocks_tail);
*/
/*
	enqueue_pib(create_pib(MEM_TEST_MASTER_PID, 3, &mem_test_master, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MEM_TEST_BLOCKING1_PID, 1, &mem_test_blocking1, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(MEM_TEST_BLOCKING2_PID, 0, &mem_test_blocking2, 1024), &init_blocks_tail);
*/
	
	enqueue_pib(create_pib(STRESS_A_PID, 3, &stress_a, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(STRESS_B_PID, 2, &stress_b, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(STRESS_C_PID, 1, &stress_c, 1024), &init_blocks_tail);

	enqueue_pib(create_pib(KCD_PID, 0, &kcd, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(CRT_PID, 0, &crt, 1024), &init_blocks_tail);
	enqueue_pib(create_pib(UART_PID, 0, &uart, 1024), &init_blocks_tail);
	
	enqueue_pib(create_pib(WALLCLOCK_PID, 0, &wallclock, 1024), &init_blocks_tail);

	enqueue_pib(create_pib(PRIORITY_SWITCH_PID, 0, &priority_switch_command_handler, 1024), &init_blocks_tail);

	int i;
	for (i = 0; i < 6; i ++) {
		enqueue_pib(create_pib(i + 1, g_test_proc[i].priority, g_test_proc[i].entry, g_test_proc[i].sz_stack), &init_blocks_tail);
	}

	struct pib * init_block_cur;

	for (init_block_cur = init_blocks_head; init_block_cur != NULL; init_block_cur = init_block_cur->next) {
		
		#ifdef _CORE_DEBUG
			rtx_dbug_outs_int("intializing process: ", init_block_cur->pid);
			rtx_dbug_outs_int("priority: ", init_block_cur->priority);
		#endif
				
		pcb * block = (pcb *)malloc(sizeof(pcb));
		block->pid = init_block_cur->pid;
		block->priority = init_block_cur->priority;
		block->starting_addr = init_block_cur->starting_addr;
		block->next = NULL;
		block->state = new;

		#ifdef _CORE_DEBUG
			rtx_dbug_outs_int("starting address: ", (UINT32)block);
		#endif

		int i;
		for (i = 0; i < 8; i ++) {
			block->a[i] = 0x00000000;
			block->d[i] = 0x00000000;
		}

		void * stack = malloc(init_block_cur->stack_size);
		stack += init_block_cur->stack_size - 1;
		block->a[7] = (UINT32)stack;

		enqueue_process(ready_queue, block);
	}

	main_process = (pcb *)malloc(sizeof(pcb));
	main_process->next = NULL;
	main_process->state = ready;
	main_process->pid = -1;
	main_process->priority = -1;
}

void  __attribute__ ((section ("__REGISTER_RTX__"))) register_rtx()
{
	#ifdef _CORE_DEBUG
		rtx_dbug_outs("Starting RTX...\n\r");
	#endif
	
	g_test_fixture.release_processor = &release_processor;
	g_test_fixture.get_process_priority = &get_process_priority;
	g_test_fixture.set_process_priority = &set_process_priority;
	g_test_fixture.send_message = &send_message;
	g_test_fixture.receive_message = &receive_message;
	g_test_fixture.request_memory_block = &request_memory_block;
	g_test_fixture.release_memory_block = &release_memory_block;
	g_test_fixture.delayed_send = &delayed_send;
}

