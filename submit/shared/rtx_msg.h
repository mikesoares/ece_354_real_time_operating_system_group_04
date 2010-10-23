#ifndef _RTX_MSG_H
#define _RTX_MSG_H

extern VOID init_messaging();
extern int send_message(int process_ID, VOID * envelope);
extern VOID * receive_message(int *tx); 
extern VOID printMailbox(); 
extern VOID printDelayedMailbox(); 
extern int delayed_send(int process_ID, VOID * envelope, int delay);
extern VOID check_existing_delayed_message();

//Create a struct, which contains the message related information. The mailbox stores the UINT32 to these structs 
struct MessageEnvelope {
	//The struct contains the following information 
	int tx; 
	int rx;
	int type; 
	int send_time;
	BYTE whitespace[48];
	CHAR msg [64];  
};

typedef struct MessageEnvelope MessageEnvelope;

//msg types
#define TYPE_WALLCLOCK_TICK 2
#define TYPE_REGISTER_CMD 3
#define TYPE_COUNT_REPORT 4
#define TYPE_WAKEUP10 5
#endif

