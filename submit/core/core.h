#ifndef _CORE_H_
#define _CORE_H_

/**
 * test proc initializaiton info. registration function provided by test suite.
 * test suite needs to register its test proc initilization info with rtx
 * The __REGISTER_TEST_PROCS_ENTRY__ symbol is in linker scripts
 */
extern void __REGISTER_TEST_PROCS_ENTRY__();

// ENUMs for process state and block types
typedef enum { new, ready, blocked_mem, blocked_msg } process_state;
typedef enum { mem, msg } block_type;

// PCB structure
struct pcb {
	UINT32 pid;
	process_state state;
	UINT32 priority;
	UINT32 d[8];
	UINT32 a[8];
	struct pcb * next;
	VOID (*starting_addr)();
};

// typedef to save typing
typedef struct pcb pcb;

pcb * main_process;

// process queues for ready, blocked mem, and blocked msg processes
pcb * ready_queue[5];
pcb * msg_blocked_queue[5];
pcb * mem_blocked_queue[5];

// currently running process
pcb * running_process;

// figure out the top of the heap from the linker script
extern VOID * _end;

// store that top as the heap pointer
VOID * heap;

SINT32 atomic_status;

VOID * malloc(UINT32 size);

VOID atomic(int);

// block and unblock processes
VOID block_process(UINT32 pid, block_type type);
VOID unblock_process_by_pid(UINT32 pid, UINT32 allow_release_processor);
VOID unblock_process_by_type(block_type);

// enqueue, dequeue, and remove PCBs
VOID enqueue_process(pcb * [], pcb *);
pcb * dequeue_process(pcb * []);
VOID remove_process(pcb *);

pcb * get_process_by_pid(UINT32 pid);

// core primatives
int release_processor();
int get_process_priority(int);
int set_process_priority(int, int);
#endif

