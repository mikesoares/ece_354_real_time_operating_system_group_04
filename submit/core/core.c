#include "../shared/rtx_inc.h"
#include "../shared/rtx_io.h"
#include "../shared/rtx_mem.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_timer.h"
#include "../shared/rtx_pids.h"
#include "../shared/dbug.h"
#include "core.h"
#include "initialize.h"

VOID atomic_up()
{
	atomic(1);
}

VOID atomic_down()
{
	atomic(0);
}

VOID atomic(int n)
{
	if (n == 0) {
		n = -1;
	}

	if (n != 1 && n != -1) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs_int("Core Error: Invalid argument for atomic, ", n);
		#endif

		return;
	}

	atomic_status += n;

	// safety check
	if (atomic_status < 0) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Core Error: Atomic status is less than 0\r\n");
		#endif
		
		atomic_status = 0;
	}

	// enable interrupts
	if (atomic_status == 0) {
		asm("MOVE.W #0x2000,%sr");
	}

	// disable interrupts
	if (atomic_status == 1) {
		asm("MOVE.W #0x2700,%sr");
	}
}

VOID print_queues()
{
	rtx_dbug_outs("------------------\n\r");
	
	int i;
	for (i = 0; i < 5; i ++) {
		rtx_dbug_outs_int("Printing ready queue ", i);

		pcb * process;
		for (process = ready_queue[i]; process != NULL; process = process->next) {
			print_int(process->pid);
			rtx_dbug_outs(" -> ");
		}

		rtx_dbug_outs("\n\r");
	}
	rtx_dbug_outs("------------------\n\r");
}

VOID print_mem_blocked()
{
	rtx_dbug_outs("------------------\n\r");
	int i;
	for (i = 0; i < 5; i ++) {
		rtx_dbug_outs_int("Printing memory blocked queue ", i);
		
		pcb * process;
		for (process = mem_blocked_queue[i]; process != NULL; process = process->next) {
			print_int(process->pid);
			rtx_dbug_outs(" -> ");
		}
		
		rtx_dbug_outs("\n\r");
	}
	rtx_dbug_outs("------------------\n\r");
}

VOID print_msg_blocked()
{
	rtx_dbug_outs("------------------\n\r");
	int i;
	for (i = 0; i < 5; i ++) {
		rtx_dbug_outs_int("Printing message blocked queue ", i);
		
		pcb * process;
		for (process = msg_blocked_queue[i]; process != NULL; process = process->next) {
			print_int(process->pid);
			rtx_dbug_outs(" -> ");
		}
		
		rtx_dbug_outs("\n\r");
	}
	rtx_dbug_outs("------------------\n\r");
}

VOID print_process(pcb * process)
{
	#ifdef _CORE_DEBUG
	rtx_dbug_outs("------------------\n\r");

	if (process == NULL) {
		rtx_dbug_outs("Core Error: Invalid process!\r\n");
		return;
	}

	rtx_dbug_outs_int("Process ID: ", process->pid);
	rtx_dbug_outs_int("Priority: ", process->priority);
	
	if (process->next == NULL) {
		rtx_dbug_outs("Next: NULL\r\n");
	} else {
		rtx_dbug_outs_int("Next: ", process->next->pid);
	}
	
	rtx_dbug_outs("------------------\n\r");
	#endif
}

pcb * get_process_by_pid(UINT32 pid)
{
	// a process will not be in any queue if it's running, so check
	// that case first
	if (running_process != NULL && running_process->pid == pid) {
		return running_process;
	}

	//print_queues();
	//print_msg_blocked();
	//print_mem_blocked();

	// define all 3 possible queues
	pcb ** queues[3];
	queues[0] = ready_queue;
	queues[1] = mem_blocked_queue;
	queues[2] = msg_blocked_queue;

	int j;
	for (j = 0; j < 3; j ++) {
		int i;
		for (i = 0; i < 5; i ++) {
			pcb * block;
		
			for (block = queues[j][i]; block != NULL; block = block->next) {
				if (block->pid == pid) {
					return block;
				}
			}
		}
	}

	return NULL;
}

SINT32 get_current_pid()
{
	// if there's no running process, return -1
	if (running_process == NULL) {
		return -1;
	}

	return running_process->pid;
}

UINT32 is_pid_valid(UINT32 pid)
{
	pcb * process = get_process_by_pid(pid);

	if (process == NULL) {
		return 0;
	}

	return 1;
}

/**
 * Basic malloc() functionality
 */
VOID * malloc(UINT32 size)
{
	VOID * oldHeap = heap;
	heap += size;
	
	int i;
	for(i = 0; i < size; i++) {
		*(BYTE *)(oldHeap+i) = 0x00;	
	}

	return oldHeap;
}

VOID remove_process(pcb * process)
{
	if (process == running_process) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Process is currently running... no need to remove\r\n");
		#endif
		return;
	}
	
	pcb ** queue;

	// figure out what queue the process is in
	if (process->state == blocked_msg) {
		queue = msg_blocked_queue;
	} else if (process->state == blocked_mem) {
		queue = mem_blocked_queue;
	} else {
		queue = ready_queue;
	}

	if (queue[process->priority] == process) {
		queue[process->priority] = process->next;
		process->next = NULL;
		return;
	}
	
	pcb * cur_proc;

	for(cur_proc = queue[process->priority]; cur_proc != NULL; cur_proc = cur_proc->next) {
		// if we find the process before the one we want, remove it
		if (cur_proc->next == process) {
			cur_proc->next = process->next;
			process->next = NULL;

			#ifdef _CORE_DEBUG
			rtx_dbug_outs("Removing process from queue\n\r");
			#endif
	
			return;
		}
	}

	#ifdef _CORE_DEBUG
	rtx_dbug_outs("Process was not found in queue\n\r");
	#endif
}

VOID enqueue_process(pcb * queue[], pcb * process)
{
	#ifdef _CORE_DEBUG
	rtx_dbug_outs("Enqueuing process\r\n");
	print_process(process);
	#endif

	if (process == NULL) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Core Error: Cannot enqueue invalid process\n\r");
		#endif

		return;
	}

	// if there's nothing in the queue, add the process to the head
	if (queue[process->priority] == NULL) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs_int("Empty queue for priority ", process->priority);
		#endif

		queue[process->priority] = process;
	} else {
		// otherwise, find the last process in the process queue
		pcb * proc;
		for (proc = queue[process->priority]; proc->next != NULL; proc = proc->next) {}

		// make the currently running process the new last process in the queue
		proc->next = process;
	}

	// since the process is now at the end of the queue, its next must be NULL
	process->next = NULL;


	#ifdef _CORE_DEBUG
	rtx_dbug_outs("Process enqueued!\r\n");

	// dump the appropriate queue
	if (queue == ready_queue) {
		print_queues();
	} else if (queue == msg_blocked_queue) {
		print_msg_blocked();
	} else if (queue == mem_blocked_queue) {
		print_mem_blocked();
	}
	#endif
}

pcb * dequeue_process(pcb * queue[])
{
	if (queue == NULL) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Core Error: Cannot dequeue from an empty queue!\n\r");
		#endif
		return NULL;
	}

	int i;
	for (i = 0; i < 5; i ++) {
		if (queue[i]) {
			break;
		}
	}

	if (i == 5) {	
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Core Error: No active processes.\n\r");
		#endif
		return NULL;
	}

	#ifdef _CORE_DEBUG
	rtx_dbug_outs_int("Dequeuing from priority level ", i);
	#endif

	pcb * process = queue[i]; 
	
	if (process->next == NULL) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Queue should become empty\n\r");
		#endif
	}
	
	queue[i] = process->next;
	process->next = NULL;

	#ifdef _CORE_DEBUG
	rtx_dbug_outs("Dequeuing process\r\n");
	print_process(process);
	
	// dump the appropriate queue
	if (queue == ready_queue) {
		print_queues();
	} else if (queue == msg_blocked_queue) {
		print_msg_blocked();
	} else if (queue == mem_blocked_queue) {
		print_mem_blocked();
	}
	#endif

	return process;
}

UINT16 is_process_blocked(UINT32 pid)
{
	pcb * process = get_process_by_pid(pid);

	if (process == NULL) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Core Error: Cannot find process to get blocked state of\n\r");
		#endif
	
		return 0;
	}

	return process->state == blocked_msg || process->state == blocked_mem;
}

VOID block_process(UINT32 pid, block_type type)
{
	asm("TRAP #1");

	pcb * process = get_process_by_pid(pid);

	if (process == NULL) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Core Error: Cannot find process to block\n\r");
		#endif
		
		asm("TRAP #2");

		return;
	}
	
	#ifdef _CORE_DEBUG
	rtx_dbug_outs_int("Trying to block process ", process->pid);
	#endif
	
	if (process->state == blocked_msg || process->state == blocked_mem) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Process is already blocked!\n\r");
		#endif

		asm("TRAP #2");

		return;
	}

	// remove the process from the ready queue
	remove_process(process);

	pcb ** block_queue;

	// identify the correct block queue
	if (type == msg) {
		block_queue = msg_blocked_queue;
		process->state = blocked_msg;
	} else if (type == mem) {
		block_queue = mem_blocked_queue;
		process->state = blocked_mem;
	}

	enqueue_process(block_queue, process);

	#ifdef _CORE_DEBUG
	if (type == msg) {
		print_msg_blocked();
	} else {
		print_mem_blocked();
	}
	#endif
	
	if (process == running_process) {
		asm("TRAP #2");
		release_processor();

		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Back from a blocked process!\r\n");
		#endif
	} else {
		asm("TRAP #2");
	}
}

VOID unblock_process_by_pid(UINT32 pid, UINT32 allow_release_processor)
{
	asm("TRAP #1");
	
	pcb * process = get_process_by_pid(pid);

	if (process == NULL) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Cannot find process to unblock\n\r");
		#endif

		asm("TRAP #2");

		return;
	}

	if (process->state != blocked_msg && process->state != blocked_mem) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Cannot unblock a process that is not blocked\n\r");
		#endif

		asm("TRAP #2");

		return;
	}

	// remove process from the blocked queue
	remove_process(process);

	// put the process back into the ready state
	process->state = ready;
	
	#ifdef _CORE_DEBUG
	rtx_dbug_outs_int("Enquing formerly-blocked process ", process->pid);
	#endif

	// enqueue the process in the ready queue
	enqueue_process(ready_queue, process);
	
	// if we're unblocking a higher priority process and that's enabled, do that instead
	if (process->priority < running_process->priority && allow_release_processor) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Just unblocked higher priority process... releasing processor\r\n");
		#endif

		asm("TRAP #2");
		
		release_processor();
	} else {
		asm("TRAP #2");
	}
}

VOID unblock_process_by_type(block_type type)
{
	asm("TRAP #1");

	pcb ** queue;

	if (type == mem) {
		queue = mem_blocked_queue;
	} else if (type == msg) {
		queue = msg_blocked_queue;
	}
	
	UINT32 i;
	for (i = 0; i < 5; i ++) {
		if (queue[i]) {
			break;
		}
	}

	if (i == 5) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("No active processes to unblock\n\r");
		#endif

		asm("TRAP #2");

		return;
	}

	#ifdef _CORE_DEBUG
	rtx_dbug_outs_int("Unblocking from priority level ", i);
	#endif

	pcb * process = queue[i]; 
	queue[i] = process->next;
	process->next = NULL;
	process->state = ready;

	#ifdef _CORE_DEBUG
	rtx_dbug_outs_int("Unblocking process ", process->pid);
	#endif

	enqueue_process(ready_queue, process);

	// if we're unblocking a higher priority process, do that instead
	if (process->priority < running_process->priority) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Unblocking higher priority process... releasing processor\r\n");
		#endif
		
		asm("TRAP #2");

		release_processor();
	} else {
		asm("TRAP #2");
	}
}

int get_process_priority(int pid) {

	//push data register to be used onto stack
	asm("MOVE.L %d6,-(%a7)");
	
	//move values into registers
	asm("MOVE.L %[proc_id],%%d6" : : [proc_id] "m" (pid));
	
	//trap into supervisor mode
	asm("Trap #9");
	
	//move the value back from d6
	int value;
	asm("MOVE.L %%d6,%[val]" : [val] "=m" (value) );
	
	//restore registers from stack
	asm("MOVE.L (%a7)+,%d6");
	
	return value;
	
}

void get_process_priority_handler()
{
	atomic_up();
	
	//get values from data registers
	int pid;
	asm("MOVE.L %%d6,%[proc_id]" : [proc_id] "=m" (pid));
	
	pcb * process = get_process_by_pid(pid);

	if (process == NULL) {
		atomic_down();
		asm("MOVE.L %[val],%%d6" : : [val] "i" (-1));
		return;
	}

	atomic_down();
	asm("MOVE.L %[val],%%d6" : : [val] "m" (process->priority));
	return;
}


int set_process_priority(int pid, int priority) 
{
	//push data register to be used onto stack
	asm("MOVE.L %d5,-(%a7)");
	asm("MOVE.L %d6,-(%a7)");
	
	//move values into registers
	asm("MOVE.L %[proc_id],%%d6" : : [proc_id] "m" (pid));
	asm("MOVE.L %[prior],%%d5" : : [prior] "m" (priority));
	
	//trap into supervisor mode
	asm("Trap #8");
	
	//move the value back from d6
	int value;
	asm("MOVE.L %%d6,%[val]" : [val] "=m" (value) );
	
	//restore registers from stack
	asm("MOVE.L (%a7)+,%d6");
	asm("MOVE.L (%a7)+,%d5");
	
	return value;

	
}

void set_process_priority_handler()
{
	atomic_up();
	
	//get values from data registers
	int priority;
	int pid;
	asm("MOVE.L %%d5,%[prior]" : [prior] "=m" (priority));
	asm("MOVE.L %%d6,%[proc_id]" : [proc_id] "=m" (pid));

	if (pid <= 0) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Core Notice: Trying to change the priority of an invalid PID\r\n");
		#endif

		atomic_down();
		asm("MOVE.L %[val],%%d6" : : [val] "i" (2));
		return;
	}

	if (priority < 0 || priority > 3) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Core Notice: Trying to change to an invalid priority\r\n");
		#endif

		atomic_down();
		asm("MOVE.L %[val],%%d6" : : [val] "i" (3));
		return;
	}

	pcb * block = get_process_by_pid(pid);

	// confirm that the block isn't null
	if (block == NULL) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs_int("Core Notice: Cannot find PID to change priority ", pid);
		#endif

		atomic_down();
		asm("MOVE.L %[val],%%d6" : : [val] "i" (4));
		return;
	}

	// save the old priority
	UINT32 oldPriority = block->priority;

	// if we're not changing the priority, do no work
	if (priority == block->priority) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Core Notice: Priority unchanged\r\n");
		#endif
		
		atomic_down();
		asm("MOVE.L %[val],%%d6" : : [val] "i" (0));
		return;
	}

	// if we're changing the priority on a process other than the one running, move it in its queue
	if (block != running_process) {
		remove_process(block);
		
		// change the priority
		block->priority = priority;

		pcb ** queue;

		if (block->state == blocked_msg) {
			queue = msg_blocked_queue;
		} else if (block->state == blocked_mem) {
			queue = mem_blocked_queue;
		} else {
			queue = ready_queue;
		}

		// put it in the right queue
		enqueue_process(queue, block);
		
		// if the new priority of a non-running process is higher than that of the
		// currently running process, and the process is in the ready queue... context switch
		if (priority < running_process->priority && queue == ready_queue) {
			atomic_down();

			release_processor();
			asm("MOVE.L %[val],%%d6" : : [val] "i" (0));
			return;
		}

	} 
	// if we're changing the running process
	else {
		// change the priority
		block->priority = priority;

		// if the priority is going lower on the currently running process,
		// do a context switch to ensure that there're no higher priority processes
		// that want to run instead
		if (priority > oldPriority) {
			atomic_down();

			release_processor();
			asm("MOVE.L %[val],%%d6" : : [val] "i" (0));
			return;
		}
	}
	
	// no need to context switch, return success
	atomic_down();
	asm("MOVE.L %[val],%%d6" : : [val] "i" (0));
	return;
}

VOID process_bootstrap(pcb * block)
{
	// disable supervisor mode
	asm("MOVE.W #0x0000,%sr");

	block->starting_addr();

	#ifdef _CORE_DEBUG
	rtx_dbug_outs_int("Terminating process ", block->pid);
	print_queues();
	#endif

	// remove the process from the ready queues
	// todo: remove from blocked queues?
	remove_process(block);

	// set the currently running process to nothing
	running_process = NULL;

	#ifdef _CORE_DEBUG
	rtx_dbug_outs("Process terminated\n\r");
	print_queues();
	#endif

	// jump to a new process
	release_processor();
}

/**
 * Switch to another process
 */
int release_processor() 
{
	asm("TRAP #0");

	return 0;
}

/**
 * ISR for process switch
 * 
 * Restores all registers and starts proc1() or proc2() if they're not 
 * already running.
 *
 * Doesn't properly handle pid = 0 right now
 */
VOID proc_switch_handler()
{
	atomic_up();
	asm("MOVE.L %a7,-(%a7)");

	asm("MOVE.L %d0,-(%a7)");
	asm("MOVE.L %d1,-(%a7)");
	asm("MOVE.L %d2,-(%a7)");
	asm("MOVE.L %d3,-(%a7)");
	asm("MOVE.L %d4,-(%a7)");
	asm("MOVE.L %d5,-(%a7)");
	asm("MOVE.L %d6,-(%a7)");
	asm("MOVE.L %d7,-(%a7)");

	asm("MOVE.L %a0,-(%a7)");
	asm("MOVE.L %a1,-(%a7)");
	asm("MOVE.L %a2,-(%a7)");
	asm("MOVE.L %a3,-(%a7)");
	asm("MOVE.L %a4,-(%a7)");
	asm("MOVE.L %a5,-(%a7)");
	asm("MOVE.L %a6,-(%a7)");


	// save registers in PCB if we're not on process 0
	if (running_process != NULL) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Running process:\r\n");
		print_process(running_process);
		#endif

		UINT32 d0;
		UINT32 d1;
		UINT32 d2;
		UINT32 d3;
		UINT32 d4;
		UINT32 d5;
		UINT32 d6;
		UINT32 d7;

		UINT32 a0;
		UINT32 a1;
		UINT32 a2;
		UINT32 a3;
		UINT32 a4;
		UINT32 a5;
		UINT32 a6;
		UINT32 a7;

		asm("MOVE.L (%%a7)+,%[a6]" : [a6] "=g" (a6));
		asm("MOVE.L (%%a7)+,%[a5]" : [a5] "=g" (a5));
		asm("MOVE.L (%%a7)+,%[a4]" : [a4] "=g" (a4));
		asm("MOVE.L (%%a7)+,%[a3]" : [a3] "=g" (a3));
		asm("MOVE.L (%%a7)+,%[a2]" : [a2] "=g" (a2));
		asm("MOVE.L (%%a7)+,%[a1]" : [a1] "=g" (a1));
		asm("MOVE.L (%%a7)+,%[a0]" : [a0] "=g" (a0));

		asm("MOVE.L (%%a7)+,%[d7]" : [d7] "=g" (d7));
		asm("MOVE.L (%%a7)+,%[d6]" : [d6] "=g" (d6));
		asm("MOVE.L (%%a7)+,%[d5]" : [d5] "=g" (d5));
		asm("MOVE.L (%%a7)+,%[d4]" : [d4] "=g" (d4));
		asm("MOVE.L (%%a7)+,%[d3]" : [d3] "=g" (d3));
		asm("MOVE.L (%%a7)+,%[d2]" : [d2] "=g" (d2));
		asm("MOVE.L (%%a7)+,%[d1]" : [d1] "=g" (d1));
		asm("MOVE.L (%%a7)+,%[d0]" : [d0] "=g" (d0));
	
		asm("MOVE.L (%%a7)+,%[a7]" : [a7] "=g" (a7));

		running_process->a[0] = a0;
		running_process->a[1] = a1;
		running_process->a[2] = a2;
		running_process->a[3] = a3;
		running_process->a[4] = a4;
		running_process->a[5] = a5;
		running_process->a[6] = a6;
		running_process->a[7] = a7;

		running_process->d[0] = d0;
		running_process->d[1] = d1;
		running_process->d[2] = d2;
		running_process->d[3] = d3;
		running_process->d[4] = d4;
		running_process->d[5] = d5;
		running_process->d[6] = d6;
		running_process->d[7] = d7;

		if (running_process != main_process) {
			if (running_process->state == ready) {
				// enqueue the currently running process
				enqueue_process(ready_queue, running_process);
			}
		} else {
			#ifdef _CORE_DEBUG
			rtx_dbug_outs("Running process IS main\r\n");
			#endif
		}
	} else {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Running process is null\r\n");
		#endif
	}
	
	// dequeue the process at the top of the ready queue
	running_process = dequeue_process(ready_queue);

	#ifdef _CORE_DEBUG
	rtx_dbug_outs("New running process:\r\n");
	print_process(running_process);

	print_queues();
	print_msg_blocked();
	print_mem_blocked();
	#endif

	if (running_process == NULL) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Running process is null\r\n");
		#endif
		
		running_process = main_process;
	
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Returning to main process...\r\n");
		#endif
	}

	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->a[7]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->a[6]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->a[5]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->a[4]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->a[3]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->a[2]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->a[1]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->a[0]));

	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->d[7]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->d[6]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->d[5]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->d[4]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->d[3]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->d[2]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->d[1]));
	asm("MOVE.L %[reg],-(%%a7)" : : [reg] "m" (running_process->d[0]));
	
	// if there's nothing on the process's stack (i.e. the process has not
	// yet been started), we really should start it
	if (running_process->state == new) {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Current process is in new state\n\r");
		#endif

		running_process->state = ready;
			
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Starting the process...\n\r");
		print_process(running_process);
		#endif
		
		// disable supervisor mode
		atomic_down();

		asm("MOVE.L (%a7)+,%d0");
		asm("MOVE.L (%a7)+,%d1");
		asm("MOVE.L (%a7)+,%d2");
		asm("MOVE.L (%a7)+,%d3");
		asm("MOVE.L (%a7)+,%d4");
		asm("MOVE.L (%a7)+,%d5");
		asm("MOVE.L (%a7)+,%d6");
		asm("MOVE.L (%a7)+,%d7");

		asm("MOVE.L (%a7)+,%a0");
		asm("MOVE.L (%a7)+,%a1");
		asm("MOVE.L (%a7)+,%a2");
		asm("MOVE.L (%a7)+,%a3");
		asm("MOVE.L (%a7)+,%a4");
		asm("MOVE.L (%a7)+,%a5");
		asm("MOVE.L (%a7)+,%a6");
		asm("MOVE.L (%a7)+,%a7");
		
		// run the process
		process_bootstrap(running_process);

		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Process finished...\n\r");
		#endif
	}
	// if the process is already running, just let it continue
	else {
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Current process is not in the new state\n\r");
		#endif
		
		asm("MOVE.L (%a7)+,%d0");
		asm("MOVE.L (%a7)+,%d1");
		asm("MOVE.L (%a7)+,%d2");
		asm("MOVE.L (%a7)+,%d3");
		asm("MOVE.L (%a7)+,%d4");
		asm("MOVE.L (%a7)+,%d5");
		asm("MOVE.L (%a7)+,%d6");
		asm("MOVE.L (%a7)+,%d7");

		asm("MOVE.L (%a7)+,%a0");
		asm("MOVE.L (%a7)+,%a1");
		asm("MOVE.L (%a7)+,%a2");
		asm("MOVE.L (%a7)+,%a3");
		asm("MOVE.L (%a7)+,%a4");
		asm("MOVE.L (%a7)+,%a5");
		asm("MOVE.L (%a7)+,%a6");
		asm("MOVE.L (%a7)+,%a7");
		
		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Registers dumped\r\n");
		#endif
	
		atomic_down();

		#ifdef _CORE_DEBUG
		rtx_dbug_outs("Left supervisor mode\r\n");
		#endif
	}
}
/**
 * Initialize the VBR so that we know where to put the ISR
 */
SINT8 coldfire_vbr_init()
{
	asm( "move.l %a0, -(%a7)" );
	asm( "move.l #0x10000000, %a0 " );
	asm( "movec.l %a0, %vbr" );
	asm( "move.l (%a7)+, %a0" );
	
	return RTX_SUCCESS;
}

/* gcc expects this function to exist */
int __main( VOID )
{
	return 0;
}

int main( VOID )
{
	#ifdef _CORE_DEBUG
	rtx_dbug_outs("Entering main()\n\r");
	#endif

	coldfire_vbr_init();	

	/*  
	 * Move asm_process_switch asm IRR into the right memory address 
	 */
	asm("move.l #asm_process_switch,%d0");
	asm("move.l %d0,0x10000080");

	asm("move.l #asm_atomic_up,%d0");
	asm("move.l %d0,0x10000084");

	asm("move.l #asm_atomic_down,%d0");
	asm("move.l %d0,0x10000088");
	
	asm("move.l #asm_set_priority,%d0");
	asm("move.l %d0,0x100000A0");
	
	asm("move.l #asm_get_priority,%d0");
	asm("move.l %d0,0x100000A4");

	// initialize pointers
	int i;
	for (i = 0; i < 5; i ++) {
		ready_queue[i] = NULL;
		msg_blocked_queue[i] = NULL;
		mem_blocked_queue[i] = NULL;
	}
	
	running_process = NULL;
	heap = &_end;
	atomic_status = 0;

	initialize_processes();
	init_memory();
	init_messaging();
	uart_init();
	regcmd_init();
	init_timer();

	// start in the main process
	running_process = main_process;

	release_processor();
	
	#ifdef _CORE_DEBUG
	rtx_dbug_outs("RTX terminating...\r\n");
	#endif

	return 0;
}

