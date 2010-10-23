#ifndef _RTX_IO_H_
#define _RTX_IO_H_

extern void uart_init();
extern void uart();
extern void kcd();
extern void output_kcd_buffer();
extern void reset_kcd_buffer(BYTE * buffer);

#define KCD_BUFFER_SIZE 64

struct io_message {
	UINT32 tx; 
	UINT32 rx; 
	int type; 
	int send_time;
	BYTE whitespace[48];
	BYTE msg[KCD_BUFFER_SIZE];
};

typedef struct io_message io_message;

extern void crt();

struct reg_cmd {
	UINT32 pid;
	BYTE cmd[KCD_BUFFER_SIZE];
	struct reg_cmd * next;
};

typedef struct reg_cmd reg_cmd;

reg_cmd * cmd_head;
reg_cmd * cmd_tail;

extern void print_cmds();
extern void regcmd_init();
void register_command(BYTE *, UINT32);
#endif

