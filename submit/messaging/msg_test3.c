#include "../shared/rtx_inc.h"
#include "../shared/rtx_core.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_mem.h"
#include "../shared/dbug.h"
#include "../shared/rtx_pids.h"
#include "../shared/rtx_timer.h"
#include "msg_test3.h"

VOID * messagetomessage3test1;

VOID message_test3_master() { 
	// ---------------------------
	// Delay tests
	// ---------------------------
	
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs("MSG NOTICE: Entering message_test3_master\n\r");
	#endif
	
	messagetomessage3test1 = request_memory_block();
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs("Sending delayed messages to message3_test1\n\r");
	#endif
	
	delayed_send(MSG_TEST9_PID, messagetomessage3test1, 2000); 
	
	//receive message from message3_test1
	MessageEnvelope * newmessage = (MessageEnvelope *)receive_message(NULL);
	UINT32 receive_time_from_test1; 
	receive_time_from_test1 = get_time(); 
	
	if (newmessage->tx == MSG_TEST9_PID) { 
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: master successfully received a message from message3_test1\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: master did not get message from message3_test1\r\n");
		#endif	
	}
	
	if (receive_time_from_test1 >= 2000 &&  receive_time_from_test1 <= 2500)
	{	
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: the time for the delayed message sent to message3_test1 is a reasonable time\n\r");
		#endif
	}	
	else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: time taken too long to send delayed message to message3_test1\n\r");
		#endif
	}
	
	release_memory_block(newmessage); 
	
	// ---------------------------
	// Delay test with preemption 
	// ---------------------------
	
	//send delayed message to message3_test2 (higher priority than message3_test3
	VOID * messagetomessage3test2 = request_memory_block();
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs("Sending delayed messages to message3_test2\n\r");
	#endif
	
	delayed_send(MSG_TEST10_PID, messagetomessage3test2, 3000); 
	
	//send message to message3_test3 
	VOID * messagetomessage3test3 = request_memory_block();
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs("Sending delayed messages to message3_test3\n\r");
	#endif
	
	send_message(MSG_TEST11_PID, messagetomessage3test3); 
	
	//receive message and see if priority is correct
	MessageEnvelope * msg = (MessageEnvelope *)receive_message(NULL);

	if (msg->tx != MSG_TEST11_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: Higher-priority process was not unblocked on receive message\r\n");
		#endif	
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: Higher-priority process was unblocked on receive message\r\n");
		#endif
	}
	
	release_memory_block(msg);

	msg = (MessageEnvelope *)receive_message(NULL);
	
	if (msg->tx != MSG_TEST10_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: Message from unexpected PID... likely an unblocking order error\r\n");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCCESS: Message came from expected PID\r\n");
		#endif
	}

	release_memory_block(msg);
}

VOID message3_test1() {
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs("MSG NOTICE: Entering message3_test1\n\r");
	#endif
	
	MessageEnvelope * newmessage = (MessageEnvelope *) receive_message(NULL);
	
	if(newmessage->tx != MSG_TEST3_MASTER_PID) {
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
	
	release_memory_block(newmessage);
	
	//immediately send a msg back to the master to check time
	VOID * message3_test1_to_master = request_memory_block();
	send_message(MSG_TEST3_MASTER_PID, message3_test1_to_master);
	
	#ifdef _MSG_TEST_DEBUG
	rtx_dbug_outs("MSG NOTICE: Completed message3_test1\n\r");
	#endif 
}

//higher priority than 3
VOID message3_test2(){

MessageEnvelope * newmessage = (MessageEnvelope *) receive_message(NULL);
	
	if(newmessage->tx != MSG_TEST3_MASTER_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message3_test2 did not have the test master as the sender\n\r");
		#endif
	} else if(newmessage->rx != MSG_TEST10_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message3_test2 did not have itself as the receiver\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message3_test2 successfully received a message from the master\n\r");
		#endif
	}
	
	release_memory_block(newmessage);
	
	//Send message back to master, this should be received last even though its a higher priority since its on delayed send
	VOID * message3_test2_to_master = request_memory_block();
	send_message(MSG_TEST3_MASTER_PID, message3_test2_to_master); 
	
}

VOID message3_test3() {

MessageEnvelope * newmessage = (MessageEnvelope *) receive_message(NULL);
	
	if(newmessage->tx != MSG_TEST3_MASTER_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message3_test3 did not have the test master as the sender\n\r");
		#endif
	} else if(newmessage->rx != MSG_TEST11_PID) {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG ERROR: message3_test3 did not have itself as the receiver\n\r");
		#endif
	} else {
		#ifdef _MSG_TEST_DEBUG
		rtx_dbug_outs("MSG SUCESS: message3_test3 successfully received a message from the master\n\r");
		#endif
	}
	
	release_memory_block(newmessage);
	
	//Send message back to master, this should be received first
	VOID * message3_test3_to_master = request_memory_block();
	send_message(MSG_TEST3_MASTER_PID, message3_test3_to_master); 
	

}

