#include "../shared/rtx_inc.h"
#include "../shared/rtx_io.h"
#include "../shared/rtx_mem.h"
#include "../shared/rtx_timer.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_core.h"
#include "../shared/rtx_pids.h"
#include "../shared/dbug.h"
#include "../shared/rtx_util.h"

int clock_on = 0; //0 if the clock is disabled, 1 if clock is enabled
UINT32 hour;
UINT32 min;
UINT32 sec;

UINT32 out_hour;
UINT32 out_min;
UINT32 out_sec;

UINT32 base_time;
UINT32 cur_time;

CHAR out_time[11];

int convert_to_dec(CHAR msg []);
void convert_to_ascii();
int error_check(CHAR msg []);
int get_msg_type(CHAR msg[]);

void wallclock() {
	clock_on = 0;
	hour = 0;
	min = 0;
	sec = 0;
	
	out_hour = 0;
	out_min = 0;
	out_sec = 0;
	
	int mode;

	#ifdef _WALLCLOCK_DEBUG
	rtx_dbug_outs("Registering wall clock\r\n");
	#endif
	
	struct MessageEnvelope * msg1 = (MessageEnvelope *)request_memory_block();
	msg1->type = TYPE_REGISTER_CMD;
	str_copy((BYTE *) "%WS", (BYTE *)msg1->msg);
	send_message(KCD_PID, msg1);
	
	struct MessageEnvelope * msg2 = (MessageEnvelope *)request_memory_block();
	msg2->type = TYPE_REGISTER_CMD;
	str_copy((BYTE *) "%WT", (BYTE *)msg2->msg);
	send_message(KCD_PID, msg2);
	
	// for receiving messages
	struct MessageEnvelope * msg;
	
	// for sending messages
	struct MessageEnvelope * send_msg;
	
	#ifdef _WALLCLOCK_DEBUG
	rtx_dbug_outs("Registed wall clock\r\n");
	#endif
	
	while (1) {
		#ifdef _WALLCLOCK_DEBUG
		rtx_dbug_outs("In wallclock loop\r\n");
		#endif
		
		//receive message
		msg = (MessageEnvelope *)receive_message(NULL);
		
		#ifdef _WALLCLOCK_DEBUG
		rtx_dbug_outs("After receive message\r\n");
		#endif
		
		if (msg == NULL) {
			#ifdef _WALLCLOCK_DEBUG
			rtx_dbug_outs("WALLCLOCK ERROR: Invalid message received\r\n");
			#endif
			
			release_memory_block((void *)msg);
			continue;
		}
		
		#ifdef _WALLCLOCK_DEBUG
		rtx_dbug_outs_int("Recieved Message Type: ", msg->type);
		rtx_dbug_outs_int("Clock status: ", clock_on);
		#endif
		
		mode = get_msg_type(msg->msg);
		
		//if the message is telling the clock to start or stop then the int keeping track of that needs
		//to be changed
		if (mode == 1) { //clock on
			#ifdef _WALLCLOCK_DEBUG
			rtx_dbug_outs("Wallclock on\r\n");
			#endif
			
			//check to make sure string is in %WS HH:MM:SS format
			if (error_check(msg->msg) == 1) {
				#ifdef _WALLCLOCK_DEBUG
				rtx_dbug_outs("Invalid in clock format\r\n");
				#endif
				
				release_memory_block((void *)msg);
				continue;
			}

			//convert values from chars to decimal values (and check that they are valid options eg HH is less then 24)
			if (convert_to_dec(msg->msg) == 1) {
				#ifdef _WALLCLOCK_DEBUG
				rtx_dbug_outs("Invalid in clock values\r\n");
				#endif
				
				release_memory_block((void *)msg);
				continue;
			}
						
			//if the clock is already on, set the new time and continue (so time not updated twice a second)
			if (clock_on) {
				base_time = get_time();
				
				//release a memory block
				release_memory_block((void *)msg);
				continue;
			}
			
			base_time = get_time();
			clock_on = 1;
		} else if (mode == 2) { //clock off
			
			#ifdef _WALLCLOCK_DEBUG
			rtx_dbug_outs("Wallclock off\r\n");
			#endif
			
			clock_on = 0;	
			release_memory_block((void *)msg);
			continue;
			
		} else if (msg->type == TYPE_WALLCLOCK_TICK && clock_on == 1) {
			#ifdef _WALLCLOCK_DEBUG
			rtx_dbug_outs("Tick...\r\n");
			#endif
			
			// get a memory block to send to CRT (this may block, so we should
			// do this *before* getting the time)
			send_msg = (MessageEnvelope *)request_memory_block();
			
			//get the time relative to when the clock was started for this tick
			cur_time = (get_time() - base_time) / 1000;
			
			//calculate the hour, min and sec
			out_sec = (sec + cur_time) % 60;
			out_min = (min + (sec + cur_time) / 60) % 60;
			out_hour = (hour + (min + (sec + cur_time) / 60) / 60) % 24;
			
			//print to CRT
			convert_to_ascii(out_hour, out_min, out_sec);
			
			//sent msg values
			str_copy((BYTE *)out_time, (BYTE *)send_msg->msg);
			
			#ifdef _WALLCLOCK_DEBUG
			rtx_dbug_outs("Timer Message: ");
			rtx_dbug_outs(send_msg->msg);
			rtx_dbug_outs("\r\n");
			#endif
			
			//send message
			send_message(CRT_PID, send_msg);
		}
		
		#ifdef _WALLCLOCK_DEBUG
		rtx_dbug_outs("release block 1\r\n");
		#endif
		
		release_memory_block((void *)msg);
		
		if(clock_on == 1) {
			//send wallclock a delayed message of 1 second
			struct MessageEnvelope * delay_msg;
			delay_msg = (MessageEnvelope *)request_memory_block();
			delay_msg->type = TYPE_WALLCLOCK_TICK;
		
			delayed_send(WALLCLOCK_PID, delay_msg, 1000);
		}
	}
}

//%WS HH:MM:SSNULL
//return 0 if no errors and 1 if there are errors
int error_check(CHAR msg []) {
	
	//check that the 7th and 10th characters are colons
	if (msg[6] != ':' || msg[9] != ':' ) {
		return 1;
	}
	
	//check that the 4th position is a space
	if (msg[3] != ' ') {
		return 1;
	}
	
	//check that the 13th position is NULL
	if (msg[12] != NULL) {
		return 1;
	}
	
	//check that HH MM SS are all numeric values
	if (msg[4] < 0x30 || msg[4] > 0x39 || msg[5] < 0x30 || msg[5] > 0x39 || msg[7] < 0x30 || msg[7] > 0x39 || msg[8] < 0x30 || 
		msg[8] > 0x39 || msg[10] < 0x30 || msg[10] > 0x39 || msg[11] < 0x30 || msg[11] > 0x39) {
		return 1;
	}
	
	return 0;
	
}

//checks if this is starting or stopping the clock (not sent in msg type, need to check msg itself)
int get_msg_type(CHAR msg[]) {
	
	if (msg[2] == 'S') {
		return 1;
	} else if (msg[2] == 'T') {
		return 2;
	}
	
	return 0;
}

//converts the HH MM SS Char into decimal values
int convert_to_dec(CHAR msg[]) {
	//convert to decimal - these should NOT be stored in the global variables until the validity checks pass below
	int tmp_hour = (((int)msg[4] - 0x30) * 10) + ((int)msg[5] - 0x30);
	int tmp_min = (((int)msg[7] - 0x30) * 10) + ((int)msg[8] - 0x30);
	int tmp_sec = (((int)msg[10] - 0x30) * 10) + ((int)msg[11] - 0x30);
	
	//check that hour is less then 24
	if (tmp_hour > 23) {
		return 1;
	}
	
	//check that min and sec are less then 60
	if (tmp_min > 59 || tmp_sec > 59) {
		return 1;
	}
	
	hour = tmp_hour;
	min = tmp_min;
	sec = tmp_sec;

	return 0;
}

//converts out_hour, out_min, out_sec to a string value (with the colons)
void convert_to_ascii (UINT32 hour, UINT32 min, UINT32 sec) {
	
	out_time[0] = (CHAR)((hour / 10) + 0x30);
	out_time[1] = (CHAR)((hour % 10) + 0x30);
	out_time[2] = ':';
	out_time[3] = (CHAR)((min / 10) + 0x30);
	out_time[4] = (CHAR)((min % 10) + 0x30);
	out_time[5] = ':';
	out_time[6] = (CHAR)((sec / 10) + 0x30);
	out_time[7] = (CHAR)((sec % 10) + 0x30);
	out_time[8] = '\r';
	out_time[9] = '\n';
	out_time[10] = NULL;
	
	return;
}

