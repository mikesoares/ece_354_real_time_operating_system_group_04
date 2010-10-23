#include "../shared/rtx_inc.h"
#include "../shared/dbug.h"
#include "../shared/rtx_core.h" 
#include "../shared/rtx_mem.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_timer.h"

//Add everything here to a header file

struct MessageEnvelope ** mailbox;
struct MessageEnvelope ** delayed_mailbox;  

// end here everything to a header file

VOID init_messaging()
{	
	//Initialize and give memory to the structs inside the mailbox 
	mailbox = (struct MessageEnvelope **)malloc(MEM_BLOCK_COUNT * sizeof(struct MessageEnvelope));
	delayed_mailbox = (struct MessageEnvelope **)malloc(MEM_BLOCK_COUNT * sizeof(struct MessageEnvelope));
	
	int i;
	for(i = 0; i < MEM_BLOCK_COUNT; i++) {
		mailbox[i] = NULL;
		delayed_mailbox[i] = NULL; 
	}
	
	//Set the ISR to the right mem address
	asm("move.l #asm_send_message,%d0");
	asm("move.l %d0,0x10000094");
	
	asm("move.l #asm_receive_message,%d0");
	asm("move.l %d0,0x10000098");
	
	asm("move.l #asm_delayed_send,%d0");
	asm("move.l %d0,0x1000009C");
}

int send_message(int process_ID, VOID * envelope) 
{
	//push data register to be used onto stack
	asm("MOVE.L %d5,-(%a7)");
	asm("MOVE.L %d6,-(%a7)");
	
	//move values into registers
	asm("MOVE.L %[pid],%%d6" : : [pid] "m" (process_ID));
	asm("MOVE.L %[env],%%d5" : : [env] "m" (envelope));
	
	//trap into supervisor mode
	asm("Trap #5");
	
	//move the value back from d6
	int value;
	asm("MOVE.L %%d6,%[val]" : [val] "=m" (value) );
	
	//restore registers from stack
	asm("MOVE.L (%a7)+,%d6");
	asm("MOVE.L (%a7)+,%d5");
	
	return value;
	
}
VOID send_message_handler() {
	
	atomic_up();
	
	//get values from data registers
	void * envelope;
	UINT32 process_ID;
	asm("MOVE.L %%d5,%[env]" : [env] "=m" (envelope));
	asm("MOVE.L %%d6,%[pid]" : [pid] "=m" (process_ID));
	
	#ifdef _MSG_DEBUG
	rtx_dbug_outs("Enter: send_message\n\r");
	#endif

	if (envelope == NULL) {
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("MSG ERROR: send_message was given null envelope\r\n");
		#endif

		atomic_down();
		asm("MOVE.L %[val],%%d6" : : [val] "i" (2));
		return;
	}

	//Cast the void pointer to the struct MessageEnvelope
	MessageEnvelope * newmessage = (MessageEnvelope *) envelope;
	
	//check if the process id is valid 
	if (!is_pid_valid(process_ID)) 
	{
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("MSG_ERROR: Process ID incorrect\n\r");
		#endif
		
		//trap out of supervisor mode
		atomic_down();
		asm("MOVE.L %[val],%%d6" : : [val] "i" (1));
		return; 
		
	} else {
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("Filling out message header...\n\r");
		#endif
		
		//Fill out the message header 
		newmessage->rx = process_ID; 
		
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("Reassigning memory block...\n\r");
		#endif
		
		reassign_mem_block((void *)newmessage, process_ID);
		
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("Calling scheduler...\n\r");
		#endif
		
		//Call the schedular to see whats running and fill in the tx of the header  
		newmessage->tx = (int)get_current_pid(); 
		
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("Storing message...\n\r");
		#endif
		
		//Store the new message by going through the array to find a null spot 
		UINT32 i; 
		for (i = 0; i < MEM_BLOCK_COUNT; i++)
		{
			if (mailbox[i] == NULL)
			{
				mailbox[i] = newmessage; 
				break; 
			}
		}
		
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("Unblocking process...\n\r");
		#endif
		
		atomic_down();

		//The kernel now has to now unblock the process 
		unblock_process_by_pid(process_ID, 1);
		
		#ifdef _MSG_DEBUG
		rtx_dbug_outs_int("Successfully unblocked process ", process_ID);
		#endif
		
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("Exit: send_message\n\r");
		#endif

	}
	
	asm("MOVE.L %[val],%%d6" : : [val] "i" (0));
	return;
	
}

VOID * receive_message(int *tx) {

	//push data register to be used onto stack
	asm("MOVE.L %d6,-(%a7)");
	
	//trap into supervisor mode
	asm("Trap #6");
	
	//move the value back from d6
	MessageEnvelope * msg;
	asm("MOVE.L %%d6,%[rVal]" : [rVal] "=m" (msg) );

	//restore registers from stack
	asm("MOVE.L (%a7)+,%d6");

	if (msg == NULL) {
		return receive_message(tx);
	}

	if (tx != NULL) {
		*tx = msg->tx;
	}
	
	return msg;
	
}

VOID receive_message_handler() {
	
	atomic_up();
	
	#ifdef _MSG_DEBUG
	rtx_dbug_outs((CHAR *) "Enter: receive_message\r\n");
	#endif
	
	//Create a local pointer variable 
	struct MessageEnvelope * receivedMessage; 	
	UINT32 i;
	
	UINT32 current_pid = get_current_pid();
	
	#ifdef _MSG_DEBUG
	rtx_dbug_outs_int((CHAR *) "Current PID is ", current_pid);
	rtx_dbug_outs((CHAR *) "Checking mailbox...\r\n");
	#endif
	
	//Go through mailbox and find the location where the destination ID equals to the current process running 
	for (i = 0; i < MEM_BLOCK_COUNT; i++) {
		receivedMessage = mailbox[i];
		
		#ifdef _MSG_DEBUG
		rtx_dbug_outs_int((CHAR *) "Checking message ", i);
		#endif
	
		// if there's nothing in this mailbox spot, break out
		if (receivedMessage == NULL) {
			break;
		}

		if (receivedMessage->rx == current_pid) {
			//Set that specific location of the mailbox to NULL	
			mailbox[i] = NULL;
			
			//Find the next spot after NULL where the mailbox has content
			for (i = i + 1; i < MEM_BLOCK_COUNT; i++){
				if (mailbox[i] != NULL) 
				{
					//Shift the content after mailbox[i]; search for if there are any content after mailbox[i] and shift it.				
					mailbox[i - 1] = mailbox[i]; 
					mailbox[i] = NULL; 
				} else {
					break;
				}
			}

			/*
			#ifdef _MSG_DEBUG
			rtx_dbug_outs("MSG NOTIFY: print the delayed mailbox after the process receives the delayed msg\n\r");
			#endif
			
			printDelayedMailbox(i); */
			
			//trap out of supervisor mode
			atomic_down();
			asm("MOVE.L %[val],%%d6" : : [val] "m" (receivedMessage));
			return;
		}
	}

	#ifdef _MSG_DEBUG
	rtx_dbug_outs((CHAR *) "Blocking process...\r\n");
	#endif
	
	//Block the process if the location is not found 
	atomic_down();
	block_process(current_pid, msg);
	
	#ifdef _MSG_DEBUG
	rtx_dbug_outs_int("Successfully blocked process ", current_pid);
	#endif
	
	#ifdef _MSG_DEBUG
	rtx_dbug_outs((CHAR *) "Exit: receive_message\r\n");
	#endif
	
	//trap out of supervisor mode
	asm("MOVE.L %[val],%%d6" : : [val] "i" (NULL));
	return;
}

int delayed_send(int process_ID, VOID * envelope, int delay) 
{
	//push data register to be used onto stack
	asm("MOVE.L %d4,-(%a7)");
	asm("MOVE.L %d5,-(%a7)");
	asm("MOVE.L %d6,-(%a7)");
	
	//move values into registers
	asm("MOVE.L %[pid],%%d6" : : [pid] "m" (process_ID));
	asm("MOVE.L %[env],%%d5" : : [env] "m" (envelope));
	asm("MOVE.L %[del],%%d4" : : [del] "m" (delay));
	
	//trap into supervisor modeea
	asm("Trap #7");
	
	//move the value back from d6
	int value;
	asm("MOVE.L %%d6,%[val]" : [val] "=m" (value) );
	
	//restore registers from stack
	asm("MOVE.L (%a7)+,%d6");
	asm("MOVE.L (%a7)+,%d5");
	asm("MOVE.L (%a7)+,%d4");
	
	return value;
	
}

VOID delayed_send_handler() {
	
	atomic_up();
	
	//get values from data registers
	void * envelope;
	UINT32 process_ID;
	UINT32 delay; 
	
	asm("MOVE.L %%d5,%[env]" : [env] "=m" (envelope));
	asm("MOVE.L %%d6,%[pid]" : [pid] "=m" (process_ID));
	asm("MOVE.L %%d4,%[del]" : [del] "=m" (delay)); 
	
	#ifdef _MSG_DEBUG
	rtx_dbug_outs("Enter: delayed_send function\n\r");
	#endif

	if (envelope == NULL) {
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("MSG ERROR: delayed_message was given null envelope\r\n");
		#endif

		atomic_down();
		asm("MOVE.L %[val],%%d6" : : [val] "i" (2));
		return;
	}

	//Cast the void pointer to the struct MessageEnvelope
	MessageEnvelope * new_delayedmessage = (MessageEnvelope *) envelope;
	 
	if (!is_pid_valid(process_ID)) 
	{
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("MSG_ERROR: Process ID incorrect\n\r");
		#endif
		
		//trap out of supervisor mode
		atomic_down();
		asm("MOVE.L %[val],%%d6" : : [val] "i" (1));
		return; 
		
	} else {
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("Starting to fill out all parts of the message envelope for delayed send\n\r");
		#endif
		
		new_delayedmessage->rx = process_ID; 
		
		reassign_mem_block((void *)new_delayedmessage, process_ID);
		
		new_delayedmessage->tx = (int)get_current_pid(); 
		
		//fill out send time - the delay time + the actual/current time 
		new_delayedmessage->send_time = (delay + get_time()); 
		
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("Finished filling out all parts of the message envelope for delayed send\n\r");
		#endif
		
		//Store the new message by going through the array to find a null spot 
		UINT32 i; 
		for (i = 0; i < MEM_BLOCK_COUNT; i++)
		{
			if (delayed_mailbox[i] == NULL)
			{
				delayed_mailbox[i] = new_delayedmessage; 
				break; 
			}
		}
		
		#ifdef _MSG_DEBUG
		rtx_dbug_outs("MSG NOTIFY: Finished putting the message in the delayed mailbox\n\r");
		#endif
		
		atomic_down();
	}
	
	asm("MOVE.L %[val],%%d6" : : [val] "i" (0));
	return;
	
}

VOID check_existing_delayed_message()
{	
	//check to see if any entry in the delayed mailbox needs to be sent 
	SINT32 i; 

	UINT32 highest_priority = 4;

	for (i = 0; i < MEM_BLOCK_COUNT; i++)
	{
		//if such an entry exists, move the entry in the delayed_mailbox into the actual mailbox 
		if (delayed_mailbox[i] != NULL && delayed_mailbox[i]->send_time <= get_time()) 
		{
			//find an empty spot in the mailbox to store the entry from the delayed_mailbox 
			UINT32 j; 
			for (j = 0; j < MEM_BLOCK_COUNT; j++) 
			{
				if (mailbox[j] == NULL) {
					mailbox[j] = delayed_mailbox[i]; 
					delayed_mailbox[i] = NULL;
					break;
				}
			}
			
			if (j == MEM_BLOCK_COUNT) {
				#ifdef _MSG_ERROR
				rtx_dbug_outs("MSG ERROR: No space left in the mailbox for delayed message\r\n");
				#endif
			}
			
			if (mailbox[j] == NULL) {
				#ifdef _MSG_ERROR
				rtx_dbug_outs("MSG ERROR: Message was not successfully moved into real mailbox\r\n");
				#endif
			}

			UINT32 k;

			// shift everything in the delayed mailbox
			for (k = i + 1; k < MEM_BLOCK_COUNT; k ++) {
				if (delayed_mailbox[k] == NULL) {
					break;
				}

				delayed_mailbox[k - 1] = delayed_mailbox[k];
				delayed_mailbox[k] = NULL;
			}

			// if something was shifted back, we also need to shift i back
			if (k != MEM_BLOCK_COUNT && k != i + 1) {
				i --;
			}
			
			if (is_process_blocked(mailbox[j]->rx)) {
				unblock_process_by_pid(mailbox[j]->rx, 0);
				
				// update the highest priority process found if this one is higher than the previous
				// best
				if (get_process_priority(mailbox[j]->rx) < highest_priority) {
					highest_priority = get_process_priority(mailbox[j]->rx);
				}

				#ifdef _MSG_DEBUG
				rtx_dbug_outs_int("MSG Notice: delayed process sucessfully unblocked ", mailbox[j]->rx);
				#endif
			} else {
				#ifdef _MSG_DEBUG
				rtx_dbug_outs("MSG Notice: Process is not blocked\n\r");
				#endif
			}
		}
	}

	// release the processor if we've encountered a higher priority process
	if (highest_priority < get_process_priority(get_current_pid())) {
		release_processor();
	}
	/*
	#ifdef _MSG_DEBUG
	rtx_dbug_outs("MSG Notice: finished running check_delayed_message function\n\r");
	#endif */
	
	return;
}

VOID printMailbox(UINT32 i) {
	for (i = 0; i < MEM_BLOCK_COUNT; i++) {
	if (mailbox[i] != NULL)
	{
		rtx_dbug_outs_int("The sender is: ", mailbox[i]->tx);
		rtx_dbug_outs_int("The receiver is: ", mailbox[i]->rx);
		rtx_dbug_outs_int("The process type is: ", mailbox[i]->type);
	} else {
		rtx_dbug_outs("There is nothing at this entry\r\n");
		}
	}
}

VOID printDelayedMailbox(UINT32 i) {
	for (i = 0; i < MEM_BLOCK_COUNT; i++) {
		rtx_dbug_outs_int("The sender is: ", delayed_mailbox[i]->tx);
		rtx_dbug_outs_int("The receiver is: ",delayed_mailbox[i]->rx);
		rtx_dbug_outs_int("The process type is: ", delayed_mailbox[i]->type);
		
		if (delayed_mailbox[i] == NULL) {
		rtx_dbug_outs("NULL\n\r"); 
		}
	}
}
