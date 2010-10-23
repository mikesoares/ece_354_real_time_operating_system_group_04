#ifndef _RTX_STRESS_H_
#define _RTX_STRESS_H_

#define STRESS_QUEUE_MAX 30

extern void stress_a();
extern void stress_b();
extern void stress_c();

typedef struct num_message {
	UINT32 tx; 
	UINT32 rx; 
	int type; 
	int send_time;
	BYTE whitespace[48];
	UINT32 msg;
} num_message;

typedef struct msg_q {
	struct num_message * env;
	struct msg_q * next;
} msg_q;

struct msg_q * msgq_head;
struct msg_q * msgq_tail;

void enqueue_local_msg(num_message *);

#endif
