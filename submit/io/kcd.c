/**
 * @file: kcd.c
 * @brief: kcd process and related functions
 * @author: Michael A. Soares
 * @date: 2010/06/05
 */

#include "../shared/rtx_inc.h"
#include "../shared/rtx_io.h"
#include "../shared/rtx_mem.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_core.h"
#include "../shared/rtx_pids.h"
#include "../shared/dbug.h"
#include "../shared/rtx_util.h"

BYTE * kcd_buffer;

/*
 * kcd process
 * Takes in input and stores in buffer
 */
void kcd() {
	// initialize some variables
	kcd_buffer = (BYTE *)malloc(KCD_BUFFER_SIZE);
	int kbuf_size = 0;
	struct io_message * msg = NULL;
	BYTE input_char = '\0';

	// basic command to clear the screen
	register_command((BYTE *)"%clr", KCD_PID);

	while (1) {
		#ifdef _IO_DEBUG
			rtx_dbug_outs((CHAR *) "Enter: kcd - waiting for message\r\n");
		#endif
		
		msg = (io_message *)receive_message(NULL);
		
		#ifdef _IO_DEBUG
			rtx_dbug_outs((CHAR *) "Enter: kcd - received message\r\n");
		#endif
	
		if(msg->type == TYPE_REGISTER_CMD) {
			register_command(msg->msg, msg->tx);
			release_memory_block((void *)msg);
		} else if(msg->msg[1] == 'c' && msg->msg[2] == 'l' && msg->msg[3] == 'r') {	// clears the screen
			release_memory_block((void *)msg);
			
			struct io_message * msg = (io_message *)request_memory_block();
			msg->tx = KCD_PID;
			msg->rx = CRT_PID;
			str_copy((BYTE *)"\f", msg->msg);
			send_message(CRT_PID, msg);
		} else {
			// assign to temp var
			input_char = msg->msg[0];
			
			#ifdef _IO_DEBUG
				rtx_dbug_outs("Received character: ");
				rtx_dbug_out_char(input_char);
				rtx_dbug_outs("\r\n");
				rtx_dbug_outs((CHAR *) "Sending character to CRT proc...\r\n");
			#endif

			// change sender/receiver
			msg->tx = KCD_PID;
			msg->rx = CRT_PID;
			
			if(input_char == '\r') {			
				// add the newline char
				msg->msg[1] = '\n';
			}
			
			// send message to display on crt
			send_message(CRT_PID, msg);
			
			if(input_char == '%') {
				#ifdef _IO_DEBUG
					rtx_dbug_outs((CHAR *) "Detected start of command\r\n");
				#endif

				reset_kcd_buffer(kcd_buffer);
				kcd_buffer[0] = input_char;
				kbuf_size = 1;
			} else if(input_char == '\r') {
				#ifdef _IO_DEBUG
					rtx_dbug_outs((CHAR *) "Detected return key\r\n");
				#endif

				// check if it's part of command
				if(kcd_buffer[0] == '%') {
					#ifdef _IO_DEBUG
						rtx_dbug_outs((CHAR *) "Detected whole command\r\n");
						rtx_dbug_outs((CHAR *) "Checking command...\r\n");
					#endif
					
					// let's check our registered commands and decode what we got to make sure it's valid	
					reg_cmd * current = cmd_head;

					int i = 1;
					int check = 0;
					while(check == 0 && current != NULL) {
						if((kcd_buffer[i] == ' ' || kcd_buffer[i] == '\0') && current->cmd[i] == '\0') {	// end of command characters
							check = 1;
						} else if(kcd_buffer[i] == current->cmd[i]) {
							i++;
							continue;
						} else {
							i = 1;
							#ifdef _IO_DEBUG
								rtx_dbug_outs((CHAR *) "Command failed comparison with: ");
								rtx_dbug_outs((CHAR *) current->cmd);
								rtx_dbug_outs((CHAR *) "\r\n");
							#endif
							
							current = current->next;
						}
					}

					if(current == NULL) {
						// we didn't find the command
						#ifdef _IO_DEBUG
							rtx_dbug_outs((CHAR *) "Command entered was not valid.\r\n");
						#endif
					} else {
						#ifdef _IO_DEBUG
							rtx_dbug_outs((CHAR *) "Found a valid registered command: ");
							rtx_dbug_outs((CHAR *) current->cmd);
							rtx_dbug_outs_int((CHAR *) "\r\nSending message to process ", current->pid);
						#endif

						struct io_message * command = (io_message *)request_memory_block();
						command->tx = KCD_PID;
						command->rx = current->pid;
						str_copy((BYTE *)kcd_buffer, (BYTE *)command->msg);
						send_message(current->pid, command);
					}					

					reset_kcd_buffer(kcd_buffer);	// clear buffer since we have everything
					kbuf_size = 0;
				}
			} else {
				#ifdef _IO_DEBUG
					rtx_dbug_outs((CHAR *) "Detected new character\r\n");
				#endif

				if(kbuf_size >= KCD_BUFFER_SIZE) {
					// ignore it
				} else if(kcd_buffer[0] == '%') {
					kcd_buffer[kbuf_size] = input_char;
					kbuf_size++;
				}
			}
		}
	}
}

/*
 * Clears the given buffer of size KCD_BUFFER_SIZE
 */
void reset_kcd_buffer(BYTE * buffer) {
	#ifdef _IO_DEBUG
		rtx_dbug_outs((CHAR *) "Clearing buffer...\r\n");
	#endif
	
	int i;
	for(i = 0; i < KCD_BUFFER_SIZE; i++) {
		buffer[i] = '\0';
	}
}

/*
 * Outputs KCD buffer
 */
void output_kcd_buffer() {
	rtx_dbug_outs("------------------\n\r");
	rtx_dbug_outs((CHAR *) "Outputting KCD buffer...\r\n");
	
	int i;
	for(i = 0; i < KCD_BUFFER_SIZE; i++) {
		rtx_dbug_out_char(kcd_buffer[i]);
	}
	
	rtx_dbug_outs((CHAR *) "\r\n");
	rtx_dbug_outs("------------------\n\r");
}

/*
 * Initialize command registering
 */
void regcmd_init() {
	cmd_head = NULL;
	cmd_tail = NULL;

	// test commands that output to CRT
/*
	struct io_message * msg = (io_message *)request_memory_block();
	msg->tx = 0;
	msg->rx = KCD_PID;
	msg->type = TYPE_REGISTER_CMD;
	str_copy((BYTE *) "%hello", msg->msg);
	send_message(KCD_PID, msg); 

	struct io_message * msg1 = (io_message *)request_memory_block();
	msg1->tx = 0;
	msg1->rx = KCD_PID;
	msg1->type = TYPE_REGISTER_CMD;
	str_copy((BYTE *) "balls", msg1->msg);
	send_message(KCD_PID, msg1); 
*/
}

/*
 * Command registering
 */
void register_command(BYTE * cmd_str, UINT32 pid) {
	#ifdef _IO_DEBUG
		rtx_dbug_outs((CHAR *) "Registering command: ");
		rtx_dbug_outs((CHAR *) cmd_str);
		rtx_dbug_outs((CHAR *) "\r\n");
	#endif

	// ignore incorrectly formatted commands
	if(cmd_str[0] != '%') {
		return;
	}

	// create a new command and store it in our linked list
	reg_cmd * new_cmd = (reg_cmd *)malloc(sizeof(reg_cmd));

	// copy characters over to command struct
	str_copy(cmd_str, new_cmd->cmd);

	// assign pid and next pointer
	new_cmd->pid = pid;
	new_cmd->next = NULL;

	// if no command is stored yet
	if(cmd_head == NULL) {
		cmd_head = new_cmd;
		cmd_tail = new_cmd;
	} else {
		// point our tail to the new command
		cmd_tail->next = new_cmd;
		cmd_tail = new_cmd;
	}
        #ifdef _IO_DEBUG
                rtx_dbug_outs((CHAR *) "Registered command: ");
                rtx_dbug_outs((CHAR *) new_cmd->cmd);
                rtx_dbug_outs((CHAR *) "\r\n");
       		print_cmds(); 
	#endif
}

void print_cmds() {
    rtx_dbug_outs("------------------\n\r");
	rtx_dbug_outs("Valid commands:\n\r\n\r");

	reg_cmd * check_cmds = cmd_head;

	while(check_cmds != NULL) {
		rtx_dbug_outs((CHAR *) check_cmds->cmd);
		rtx_dbug_outs((CHAR *) "\n\r");
		check_cmds = check_cmds->next;
	}

	rtx_dbug_outs("------------------\n\r");
}
