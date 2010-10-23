/**
 * @file: stress.c
 * @brief: stress tests
 * @author: Michael A. Soares
 * @date: 2010/07/05
 */

#include "../shared/rtx_inc.h"
#include "../shared/rtx_io.h"
#include "../shared/rtx_mem.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_core.h"
#include "../shared/rtx_pids.h"
#include "../shared/rtx_stress.h"
#include "../shared/dbug.h"
#include "../shared/rtx_util.h"

void stress_a() {
	#ifdef _STRESS_DEBUG
	rtx_dbug_outs("Registering Stress Test A...\r\n");
	#endif
	
	struct MessageEnvelope * msg_rx;
	struct num_message * num_msg;
	struct MessageEnvelope * reg_cmd = (MessageEnvelope *)request_memory_block();
	reg_cmd->type = TYPE_REGISTER_CMD;
	str_copy((BYTE *) "%Z", (BYTE *)reg_cmd->msg);
	send_message(KCD_PID, reg_cmd);
		
	UINT32 num;
	
	while(1) {
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs("Enter: Stress Test A - Loop 1\r\n");
		#endif
		
		msg_rx = (MessageEnvelope *)receive_message(NULL);
		
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs("Stress Test A received a message in Loop 1\r\n");
		#endif		
		
		if(msg_rx->msg[0] == '%' && msg_rx->msg[1] == 'Z') {
			#ifdef _STRESS_DEBUG
			rtx_dbug_outs("Stress Test A received valid message - breaking out of loop\r\n");
			#endif
			release_memory_block((void *)msg_rx);
			break;
		}
		
		release_memory_block((void *)msg_rx);
	}
	
	num = 0;
	
	while(1) {
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs("Enter: Stress Test A - Loop 2\r\n");
		#endif
		
		num_msg = (num_message *)request_memory_block();
		
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs_int("Just got memory block for message num ", num);
		#endif
		
		num_msg->type = TYPE_COUNT_REPORT;
		num_msg->msg = num;
		send_message(STRESS_B_PID, num_msg);
		
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs("Stress Test A sent message to Stress Test B\r\n");
		#endif
		
		num = num + 1;
		release_processor();
	
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs("Stress Test A back from release processor\r\n");
		#endif
	}
}

void stress_b() {
	void * msg_rx;
	
	while(1) {
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs("Enter: Stress Test B\r\n");
		#endif
		
		msg_rx = receive_message(NULL);
		
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs("Stress Test B received a message\r\n");
		#endif
		
		send_message(STRESS_C_PID, msg_rx);
		
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs("Stress Test B sent message to Stress Test C\r\n");
		#endif
	}
}

void stress_c() {
	#ifdef _STRESS_DEBUG
	rtx_dbug_outs("Initializing: Stress Test C\r\n");
	#endif
	
	msgq_head = NULL;
	msgq_tail = NULL;
	
	struct num_message * msg_rx;
	struct num_message * msg_wake;
	struct MessageEnvelope * proc_c;
	struct num_message * hibernate;
		
	while(1) {
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs("Enter: Stress Test C\r\n");
		#endif
		
		if(msgq_head == NULL) {
			// receive message
			msg_rx = (num_message *)receive_message(NULL);

			#ifdef _STRESS_DEBUG
			rtx_dbug_outs("Stress Test C received a message\r\n");
			#endif
		} else {
			// dequeue first message from local message queue
			msg_rx = msgq_head->env;
			msgq_head = msgq_head->next;

			if(msgq_head == NULL) {
				msgq_tail = NULL;
			}
			
			#ifdef _STRESS_DEBUG
			rtx_dbug_outs("Stress Test C dequeued a message\r\n");
			#endif
		}
		
		UINT32 type = msg_rx->type;
		UINT32 msg_data = msg_rx->msg;
		release_memory_block((void *)msg_rx);
		
		if(type == TYPE_COUNT_REPORT) {
			#ifdef _STRESS_DEBUG
			rtx_dbug_outs_int("Stress Test C got message ", msg_rx->msg);
			#endif
			
			if(msg_data%20 == 0) {
				proc_c = (MessageEnvelope *)request_memory_block();
				str_copy((BYTE *) "Process C\r\n", (BYTE *)proc_c->msg);
				send_message(CRT_PID, proc_c);

				#ifdef _STRESS_DEBUG
				rtx_dbug_outs("Stress Test C sent a message to CRT\r\n");
				#endif

				// hibernate for 10 seconds
				hibernate = (num_message *)request_memory_block();
				hibernate->type = TYPE_WAKEUP10;
				delayed_send(STRESS_C_PID, hibernate, 10000);
				
				while(1) {
					msg_wake = (num_message *)receive_message(NULL);
					if(msg_wake->type == TYPE_WAKEUP10) {
						break;
					} else {
						// enqueue the message into local queue
						enqueue_local_msg(msg_wake);
					}
				}
			}
		}
		
		release_memory_block((void *)hibernate);
		release_processor();
	}
}

void enqueue_local_msg(num_message * msg) {
		msg_q * new_item = (msg_q *)malloc(sizeof(msg_q));
	
		new_item->env = msg;
		new_item->next = NULL;
	
		if(msgq_head == NULL) {
			msgq_head = new_item;
			msgq_tail = new_item;
		} else {
			msgq_tail->next = new_item;
			msgq_tail = new_item;
		}
			
		#ifdef _STRESS_DEBUG
		rtx_dbug_outs("Stress Test message enqueued\r\n");
		#endif
}
