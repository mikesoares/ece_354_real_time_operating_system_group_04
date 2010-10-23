#ifndef _RTX_CORE_H
#define _RTX_CORE_H

typedef enum { mem, msg } block_type;

extern SINT32 get_current_pid();
extern UINT32 is_pid_valid(UINT32 pid);
extern VOID * malloc(UINT32 size);

extern UINT16 is_process_blocked(UINT32 pid);

extern VOID block_process(UINT32 pid, block_type type);

extern VOID unblock_process_by_pid(UINT32, UINT32);
extern VOID unblock_process_by_type(block_type);

extern int release_processor();

extern int get_process_priority(int);
extern int set_process_priority(int, int);

// for printing debug info
extern VOID print_queues();
extern VOID print_blocked();
extern VOID print_mem_blocked();
extern VOID print_msg_blocked();

extern VOID atomic_up();
extern VOID atomic_down();
#endif
