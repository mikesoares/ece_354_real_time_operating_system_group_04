/**
 * @file: uart.c
 * @brief: UART i-process and related functions
 * @author: Michael A. Soares
 * @date: 2010/06/04
 */

#include "../shared/rtx_inc.h"
#include "../shared/rtx_io.h"
#include "../shared/rtx_mem.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_core.h"
#include "../shared/rtx_pids.h"
#include "../shared/dbug.h"

volatile BYTE char_in;
volatile BYTE char_out;

/*
 * This function is called by the assembly STUB function
 */
void uart_handler() {
	
	// Disable all interupts
	atomic_up();
	
	#ifdef _IO_DEBUG
		rtx_dbug_outs((CHAR *) "Enter: uart_handler\r\n");
		rtx_dbug_outs((CHAR *) "Reading data...\r\n");
	#endif

	// irene said this should be enough - would be
	// very rare if it wasn't ready to read
	while (!(SERIAL1_UCSR & 1)) { }
	
	char_in = SERIAL1_RD;

	#ifdef _IO_DEBUG
		rtx_dbug_outs((CHAR *) "Determining what to do with char...\r\n");
	#endif

	struct io_message * msg;

	switch(char_in) {
		#ifdef _HOTKEYS_DEBUG
			case '!':
				rtx_dbug_outs((CHAR *) "'!' hotkey detected...\r\n");
				print_queues();	// print processes on ready queue and priorities
				break;
			case '@':
				rtx_dbug_outs((CHAR *) "'@' hotkey detected...\r\n");
				print_mem_blocked();	// print processes on memory blocked queue and priorities
				break;
			case '#':
				rtx_dbug_outs((CHAR *) "'#' hotkey detected...\r\n");
				print_msg_blocked();	// print processes on message blocked queue and priorities
				break;
			case '$':
				rtx_dbug_outs((CHAR *) "'$' hotkey detected...\r\n");
				print_availible_mem_queue();	// print available memory queue
				break;
			case '^':
				rtx_dbug_outs((CHAR *) "'^' hotkey detected...\r\n");
				print_used_mem_queue();	// print used memory queue
				break;
			case '&':
				rtx_dbug_outs((CHAR *) "'&' hotkey detected...\r\n");		
				output_kcd_buffer();	// print kcd buffer
				break;
			case '*':
				rtx_dbug_outs((CHAR *) "'*' hotkey detected...\r\n");
				print_cmds();	// print valid commands
				break;
		#endif
		
		default:
			#ifdef _IO_DEBUG
				rtx_dbug_outs((CHAR *) "Sending message to KCD proc...\r\n");
			#endif
			
			if (!are_blocks_available()) {
				atomic_down();
				return;
			}

			msg = (io_message *)request_memory_block();
			
			// reset tx/rx and send it to kcd
			msg->tx = UART_PID;
			msg->rx = KCD_PID;
			msg->msg[0] = char_in;
			send_message(KCD_PID, msg);
			break;
	}
	
	// Enable all interupts
	atomic_down();
}

/*
 * Initialize all interrupts associated with UART
 */
void uart_init() {
	#ifdef _IO_DEBUG
		rtx_dbug_outs((CHAR *) "Enter: uart_init\r\n");
	#endif
	
    UINT32 mask;

    // Disable all interupts
    atomic_up();

    // Store the serial ISR at user vector #64
    asm( "move.l #asm_serial_entry,%d0" );
    asm( "move.l %d0,0x10000100" );

    SERIAL1_UCR = 0x10;	// Reset the entire UART
    SERIAL1_UCR = 0x20;	// Reset the receiver
    SERIAL1_UCR = 0x30;	// Reset the transmitter
    SERIAL1_UCR = 0x40;	// Reset the error condition
    SERIAL1_ICR = 0x17;	// Install the interupt
    SERIAL1_IVR = 64;	// Install the interupt
    SERIAL1_IMR = 0x02;	// enable interrupts on rx only
    SERIAL1_UBG1 = 0x00;// Set the baud rate

	#ifdef _CFSERVER_           // add -D_CFSERVER_ for cf-server build
	    SERIAL1_UBG2 = 0x49;    // cf-server baud rate 19200 
	#else
	    SERIAL1_UBG2 = 0x92;    // lab board baud rate 9600
	#endif
    
    SERIAL1_UCSR = 0xDD;// Set clock mode
    SERIAL1_UMR = 0x13;	// Setup the UART (no parity, 8 bits )
    SERIAL1_UMR = 0x07;	// Setup the rest of the UART (noecho, 1 stop bit)
    SERIAL1_UCR = 0x05;	// Setup for transmit and receive

    // Enable interupts
    mask = SIM_IMR;
    mask &= 0x0003ddff;
    SIM_IMR = mask;

    // Enable all interupts
    atomic_down(); 

    char_in = '!';
    char_out = '\0';
}

void uart() {
	struct io_message * msg = NULL;
	
	while(1) {
		#ifdef _IO_DEBUG
			rtx_dbug_outs((CHAR *) "Enter: uart - waiting for message\r\n");
		#endif
		
		msg = (io_message *)receive_message(NULL);

		#ifdef _IO_DEBUG
			rtx_dbug_outs((CHAR *) "Enter: uart - received message\r\n");
		#endif
		
		char_out = msg->msg[0];
		release_memory_block((void *)msg);
		
		// irene said this is OK
		while( !(SERIAL1_UCSR & 4) ) { }

		// write data to port
		SERIAL1_WD = char_out;
		
		#ifdef _IO_DEBUG
			rtx_dbug_outs((CHAR *) "Writing data: ");
			rtx_dbug_out_char(char_out);
			rtx_dbug_outs((CHAR *) "\r\n");
		#endif
	}
}
