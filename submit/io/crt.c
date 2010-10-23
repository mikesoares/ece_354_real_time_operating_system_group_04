/**
 * @file: crt.c
 * @brief: CRT process
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

/*
 * CRT process
 * Sends data to UART to write
 */
void crt() {
	struct io_message * msg = NULL;
	
	while(1) {
		#ifdef _IO_DEBUG
			rtx_dbug_outs((CHAR *) "Enter: crt - waiting for message\r\n");
		#endif
		
		msg = (io_message *)receive_message(NULL);
		
		#ifdef _IO_DEBUG
			rtx_dbug_outs("Enter: crt - received message\r\n");
		#endif
		
		if(msg->rx != CRT_PID) {
			#ifdef _IO_DEBUG
				rtx_dbug_outs("CRT received an invalid request - this should never happen\r\n");
			#endif
		}
		
		#ifdef _IO_DEBUG
			rtx_dbug_outs("Sending character to UART...\r\n");
		#endif
		
		// we'll go through the string and output every character on its own		
		int i;
		for(i = 0; msg->msg[i] != '\0'; i++) {
			struct io_message * uart_msg = (io_message *)request_memory_block();
			uart_msg->tx = CRT_PID;
			uart_msg->rx = UART_PID;
			uart_msg->msg[0] = msg->msg[i];
			send_message(UART_PID, uart_msg);
		}
		
		//reset_kcd_buffer(msg->msg);	// todo: remove this - it is only temporary until memory zeroing gets properly implemented
		release_memory_block((void *)msg);
	}
}
