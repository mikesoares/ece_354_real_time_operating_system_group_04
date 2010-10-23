#include "../shared/rtx_inc.h"
#include "../shared/rtx_core.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_mem.h"
#include "../shared/dbug.h"
#include "../shared/rtx_pids.h"
#include "../shared/rtx_timer.h"
#include "msg_test.h"

VOID * messagetoproc1;
VOID * proc2toproc1;
VOID * messagetomessage3test1;
VOID * proc1toproc2msg1;
VOID * proc1toproc3msg1;
VOID * proc1toproc4msg1;

VOID message_test_master() {
	//---------------------
	// Message blocking/priority test
	//---------------------

	//send to proc 1
	messagetoproc1 = request_memory_block();
	if (send_message(MSG_TEST1_PID, messagetoproc1) != 0) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: send_message did not return 0 on successful message send\r\n");
		#endif
	}

	//send to proc 2
	VOID * messagetoproc2 = request_memory_block();
	send_message(MSG_TEST2_PID, messagetoproc2); 
	
	//receive message from proc 1
	MessageEnvelope * msg = (MessageEnvelope *)receive_message(NULL);

	if (msg->tx != MSG_TEST1_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: Higher-priority process was not unblocked on receive message\r\n");
		#endif	
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: Higher-priority process was unblocked on receive message\r\n");
		#endif
	}
	
	release_memory_block(msg);

	//received message from proc2
	msg = (MessageEnvelope *)receive_message(NULL);
	
	if (msg->tx != MSG_TEST2_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: Message from unexpected PID... likely an unblocking order error\r\n");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: Message came from expected PID\r\n");
		#endif
	}

	release_memory_block(msg);

	msg = request_memory_block();

	if (send_message(INVALID_PID, msg) != 1) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: send_message did not return 1 on invalid PID\r\n");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: send_message returned 1 on invalid PID\r\n");
		#endif
	}

	release_memory_block(msg);

	if (send_message(MSG_TEST2_PID, NULL) != 2) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: send_message did not return 2 on NULL envelope\r\n");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: send_message returns 1 on NULL envelope\r\n");
		#endif
	}
	
	// --------------------------
	// message blocks stress test
	// --------------------------
	//send to proc 1
	VOID * mastertoproc1wakeup = request_memory_block();
	send_message(MSG_TEST5_PID, mastertoproc1wakeup); 

	// ---------------------------
	// Delay tests
	// ---------------------------
/*	
	//send delayed message to proc 1
	UINT32 current_time; 
	current_time = get_time();
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs_int("The current time is: ", current_time);
	#endif
	
	messagetomessage3test1 = request_memory_block();
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs("Sending delayed messages to proceeses/n/r");
	#endif
	delayed_send(MSG_TEST9_PID, messagetomessage3test1, 2000); 

	//send delayed message to proc 2
	VOID * messagetomessage3test2 = request_memory_block();
	delayed_send(MSG_TEST10_PID, messagetomessage3test2, 5000);  */
}

VOID message_test1() {
	//receive message
	VOID * receivemessage = receive_message(NULL);
	
	//blocks and unblocks 
	if (receivemessage != messagetoproc1)
	{
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message_test1 did not receive the correct message from master\n\r");
		#endif
	}
	
	MessageEnvelope * newmessage = (MessageEnvelope *) receivemessage;
	
	if(newmessage->tx != MSG_TEST_MASTER_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message_test1 did not have the test master as the sender\n\r");
		#endif
	} else if(newmessage->rx != MSG_TEST1_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message_test1 did not have itself as the receiver\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message_test1 successfully received a message from the master\n\r");
		#endif
	}

	int * tx = (int *)malloc(sizeof(int));
	
	//block receive first
	VOID * receivemessage2 = receive_message(tx);

	if (*tx != MSG_TEST2_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: TX pointer has incorrect value\r\n");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: TX pointer has correct value\r\n");
		#endif
	}

	//blocks and then unblocks to receive message from 2 
	if (receivemessage2 != proc2toproc1)
	{
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message_test1 did not receive the correct message from message_test2\n\r");
		#endif
	}
	
	MessageEnvelope * newmessage2 = (MessageEnvelope *) receivemessage2;
	
	if(newmessage2->tx!= MSG_TEST2_PID)
	{
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message_test1 did not have message message_test2 as the sender\n\r");
		#endif
	} else if(newmessage2->rx != MSG_TEST1_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message_test1 did not have itself as the recipient\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message_test1 successfully received a message from message message_test2\n\r");
		#endif
	}
	
	//sends message to test master
	VOID * proc1tomaster = request_memory_block();
	send_message(MSG_TEST_MASTER_PID, proc1tomaster); 
	
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs("MSG SUCCESS: message_test1 successfully sent a message to the master\n\r");
	#endif
	
	release_memory_block(proc1tomaster); 
}

VOID message_test2() {
//receive message
	MessageEnvelope * newmessageproc2 = (MessageEnvelope *)receive_message(NULL);
	
	if (newmessageproc2->tx == MSG_TEST_MASTER_PID) { 
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: message_test2 successfully received a message from the master\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message_test2 did not get its wakeup message from the master\r\n");
		#endif	
	}

	release_memory_block(newmessageproc2);

	//sends message to proc 1
	proc2toproc1 = request_memory_block();
	send_message(MSG_TEST1_PID, proc2toproc1);
	
	//sends a message to master
	VOID * proc2tomaster = request_memory_block();
	send_message(MSG_TEST_MASTER_PID, proc2tomaster);
	
	release_memory_block(proc2toproc1); 
}

//high priority
VOID message2_test1() {
	//receive message
	VOID * receivemessage = receive_message(NULL);
	
	MessageEnvelope * newmessage = (MessageEnvelope *) receivemessage;
	
	if(newmessage->tx != MSG_TEST_MASTER_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test1 did not have the test master as the sender\n\r");
		#endif
	} else if(newmessage->rx != MSG_TEST5_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test1 did not have itself as the receiver\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message2_test1 successfully received a message from the master\n\r");
		#endif
	}
	
	release_memory_block(newmessage); 
	
	//send to proc 2 
	proc1toproc2msg1 = request_memory_block();
	send_message(MSG_TEST6_PID, proc1toproc2msg1); 
	
	//send to proc 2
	VOID * proc1toproc2msg2 = request_memory_block();
	send_message(MSG_TEST6_PID, proc1toproc2msg2);
	
	//send to proc 3 
	proc1toproc3msg1 = request_memory_block();
	send_message(MSG_TEST7_PID, proc1toproc3msg1);

	//send to proc 3
	VOID * proc1toproc3msg2 = request_memory_block();
	send_message(MSG_TEST7_PID, proc1toproc3msg2);
	
	//send to proc 4
	proc1toproc4msg1 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg1);

	VOID * proc1toproc4msg2 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg2);
	
	VOID * proc1toproc4msg3 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg3);
	
	VOID * proc1toproc4msg4 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg4);
	
	VOID * proc1toproc4msg5 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg5);
	
	VOID * proc1toproc4msg6 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg6);
	
	VOID * proc1toproc4msg7 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg7);
	
	VOID * proc1toproc4msg8 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg8);
	
	VOID * proc1toproc4msg9 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg9);
	
	VOID * proc1toproc4msg10 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg10);
	
	VOID * proc1toproc4msg11 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg11);
	
	VOID * proc1toproc4msg12 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg12);

	VOID * proc1toproc4msg13 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg13);
	
	VOID * proc1toproc4msg14 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg14);

	VOID * proc1toproc4msg15 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg15);

	VOID * proc1toproc4msg16 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg16);

	VOID * proc1toproc4msg17 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg17);

	VOID * proc1toproc4msg18 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg18);

	VOID * proc1toproc4msg19 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg19);

	VOID * proc1toproc4msg20 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg20);

	VOID * proc1toproc4msg21 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg21);
	
	VOID * proc1toproc4msg22 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg22);
	
	VOID * proc1toproc4msg23 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg23);
	
	VOID * proc1toproc4msg24 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg24);
	
	VOID * proc1toproc4msg25 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg25);
	
	VOID * proc1toproc4msg26 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg26);
	
	VOID * proc1toproc4msg27 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg27);

	VOID * proc1toproc4msg28 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg28);
	
	VOID * proc1toproc4msg29 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg29);
	
	VOID * proc1toproc4msg30 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg30);
	
	VOID * proc1toproc4msg31 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg31);
	
	VOID * proc1toproc4msg32 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg32);
	
	VOID * proc1toproc4msg33 = request_memory_block();
	send_message(MSG_TEST8_PID, proc1toproc4msg33);
}

//medium priority
VOID message2_test2() {
	//receive message & its blocked  
	
	//Unblocks and compare envelope to proc1toproc2msg1
	VOID * msg = receive_message(NULL);
	if (msg != proc1toproc2msg1){
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test2 did not receive the message envelope in the correct order from message2_test1\n\r");
		#endif
	} else {
			#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message2_test2 successfully received the message envelope in the correct order from proc1\n\r");
		#endif
	}
	
	MessageEnvelope * newmessageproc2 = (MessageEnvelope *) msg;
	
	if(newmessageproc2->tx != MSG_TEST5_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test2 did not have proc1 as the sender\n\r");
		#endif
	} else if(newmessageproc2->rx != MSG_TEST6_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test2 did not have itself as the receiver\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message2_test2 successfully received a message from message2_test1\n\r");
		#endif
	}
	
	release_memory_block(newmessageproc2);
	
	//block
	receive_message(NULL);
}

//same priority as 2
VOID message2_test3() {
	//receive message & its blocked  
	
	//Unblocks and compare envelope to proc1toproc3msg1
	VOID * msg = receive_message(NULL);
	if (msg != proc1toproc3msg1){
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test3 did not receive the message envelope in the correct order from message2_test2\n\r");
		#endif
	} else {
			#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message2_test3 successfully received the message envelope in the correct order from message2_test2\n\r");
		#endif
	}
	
	MessageEnvelope * newmessageproc3 = (MessageEnvelope *) msg;
	
	if(newmessageproc3->tx != MSG_TEST5_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test3 did not have message2_test1 as the sender\n\r");
		#endif
	} else if(newmessageproc3->rx != MSG_TEST7_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test3 did not have itself as the receiver\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message2_test3 successfully received a message from proc1\n\r");
		#endif
	}
	
	release_memory_block(newmessageproc3);
	
	receive_message(NULL);
	
}

VOID message2_test4() {
	//receive message & its blocked  
	//Unblocks and compare envelope to proc1toproc4msg1
	VOID * msg = receive_message(NULL);
	if (msg != proc1toproc4msg1){
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test4 did not receive the message envelope in the correct order from message2_test1\n\r");
		#endif
	} else {
			#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message2_test4 successfully received the message envelope in the correct order from message2_test1\n\r");
		#endif
	}
	
	MessageEnvelope * newmessageproc4 = (MessageEnvelope *) msg;
	
	if(newmessageproc4->tx != MSG_TEST5_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test4 did not have message2_test1 as the sender\n\r");
		#endif
	} else if(newmessageproc4->rx != MSG_TEST8_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message2_test4 did not have itself as the receiver\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message2_test4 successfully received a message from message2_test1\n\r");
		#endif
	}
	
	release_memory_block(newmessageproc4);
	
}

/*
VOID message3_test1() {
	//receive delayed message
	VOID * receivemessage = receive_message(NULL);
	
	//blocks and unblocks 
	if (receivemessage != messagetomessage3test1)
	{
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message3_test1 did not receive the correct message from master\n\r");
		#endif
	}
	
	MessageEnvelope * newmessage = (MessageEnvelope *) receivemessage;
	
	if(newmessage->tx != MSG_TEST_MASTER_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message3_test1 did not have the test master as the sender\n\r");
		#endif
	} else if(newmessage->rx != MSG_TEST9_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message3_test1 did not have itself as the receiver\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message3_test1 successfully received a message from the master\n\r");
		#endif
	}
	UINT32 received_time1; 
	received_time1 = get_time(); 
	
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs_int("The received time for message3_test1 is: ", received_time1);
	#endif
	
	release_memory_block(newmessage);
}

VOID message3_test2() {
	//receive delayed message
	MessageEnvelope * newmessageproc2 = (MessageEnvelope *)receive_message(NULL);
	
	if (newmessageproc2->tx == MSG_TEST_MASTER_PID) { 
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: message3_test2 successfully received a message from the master\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message3_test2 did not get its wakeup message from the master\r\n");
		#endif	
	}
	
	UINT32 received_time2;
	received_time2 = get_time(); 
	
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs_int("The received time for test2 is: ", received_time2);
	#endif	
	
	release_memory_block(newmessageproc2);
} */

