#include "../test/rtx_test.h"
#include "../shared/dbug.h"

VOID * messagetoproc1;
VOID * proc2toproc1;
//Picking a PID that does not exist for testing 
#define INVALID_PID 28346

struct MessageEnvelope {
	//The struct contains the following information 
	int tx; 
	int rx;
	int type;  
};
typedef struct MessageEnvelope MessageEnvelope;

int fail_count;
int success_count;


void test_master() {
	fail_count = 0;
	success_count = 0;

	#ifdef _TEST_DEBUG
	rtx_dbug_outs((CHAR *)"G04_test: START\r\n");
	rtx_dbug_outs((CHAR *)"G04_test: total 16 tests\r\n");
	#endif
	
	//---------------------
	// MESSAGE TEST 1
	// Message blocking/priority test
	//---------------------
	
	//Send message to proc 1
	messagetoproc1 = g_test_fixture.request_memory_block();
	if (g_test_fixture.send_message(g_test_proc[1].pid, messagetoproc1) != 0) {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 3 FAIL\r\n");
		fail_count++;
		#endif
	}  else {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 3 OK\r\n");
		success_count++;
		#endif
	}

	//Send message to proc 2
	VOID * messagetoproc2 = g_test_fixture.request_memory_block();
	g_test_fixture.send_message(g_test_proc[2].pid, messagetoproc2); 
	
	//Receive message from proc 1
	MessageEnvelope * msg = (MessageEnvelope *)g_test_fixture.receive_message(NULL);

	if (msg->tx != g_test_proc[1].pid) {		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 8 FAIL\r\n");
		fail_count++;
		#endif	
	} else {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 8 OK\r\n");
		success_count++;
		#endif
	}
	
	g_test_fixture.release_memory_block(msg);

	//received message from proc2
	msg = (MessageEnvelope *)g_test_fixture.receive_message(NULL);
	
	if (msg->tx != g_test_proc[2].pid) {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 9 FAIL\r\n");
		fail_count++;
		#endif
	} else {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 9 OK\r\n");
		success_count++;
		#endif	
	}

	g_test_fixture.release_memory_block(msg);

	msg = g_test_fixture.request_memory_block();

	if (g_test_fixture.send_message(INVALID_PID, msg) == 0) {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 10 FAIL\r\n");
		fail_count++;
		#endif
	} else {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 10 OK\r\n");
		success_count++;
		#endif
	}

	g_test_fixture.release_memory_block(msg);

	if (g_test_fixture.send_message(g_test_proc[2].pid, NULL) == 0) {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 11 FAIL\r\n");
		fail_count++;
		#endif
	} else {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 11 OK\r\n");
		success_count++;
		#endif	
	}
	
	//---------------------
	// MESSAGE TEST 2
	// Delay messaging test
	//---------------------
	
	VOID * messagetoproc3 = g_test_fixture.request_memory_block();

	g_test_fixture.delayed_send(g_test_proc[3].pid, messagetoproc3, 1000); 
	
	//Delayed_send to process4
	VOID * messagetoproc4 = g_test_fixture.request_memory_block(); 
	g_test_fixture.delayed_send(g_test_proc[4].pid, messagetoproc4, 500); 
	
	//Receive message from process4
	
	MessageEnvelope * newmessage = (MessageEnvelope *)g_test_fixture.receive_message(NULL);

	if (newmessage->tx == g_test_proc[4].pid) { 
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 13 OK\n\r");
		success_count++;
		#endif
	} else {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 13 FAIL\r\n");
		fail_count++;
		#endif
	}
	
	g_test_fixture.release_memory_block(newmessage);
	
	//Receive message from process3
	MessageEnvelope * newmessage2 = (MessageEnvelope *)g_test_fixture.receive_message(NULL);

	if (newmessage2->tx == g_test_proc[3].pid) { 
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 15 OK\n\r");
		success_count++;
		#endif
	} else {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 15 FAIL\r\n");
		fail_count++;
		#endif
	}
	
	g_test_fixture.release_memory_block(newmessage2); 
	
	//---------------------
	// Delayed test
	// 
	// This test is designed to try to fill up the mailbox with delayed messages
	// coming in with arbitrary orders and delays. It will confirm that messages
	// are delivered in the proper order based on their delays and that the correct
	// messages are delivered.
	//
	// NOTE: This test is potentially fragile if other groups' delayed_send() function
	//       runs slower than ours does. Efforts were made to ensure that the times were
	//       sufficient to ensure that it doesn't need a super quick delayed_send(), but
	//       some of the times are fairly close together to test the desired functionality. 
	//---------------------
	
	VOID * delay_array[30];
	
	int i;
	for (i = 0; i < 30; i ++) {
		delay_array[i] = g_test_fixture.request_memory_block();
	}

	const int delays[30] = { 45, 47, 50, 75, 80, 85, 90, 110, 130, 150,
				 160, 165, 170, 185, 195, 215, 220, 230, 240, 255,
				 265, 280, 300, 350, 375, 380, 390, 395, 400, 407 };

	const int delay_order[30] = { 21, 20, 17, 28, 27, 19, 26, 13, 16, 18, 
				      0, 1, 2, 10, 15, 7, 9, 11, 24, 25, 
				      14, 23, 29, 8, 12, 22, 5, 3, 4, 6 };
	
	// set their types
	for (i = 0; i < 30; i ++) {
		((MessageEnvelope *)delay_array[i])->type = 42 + i;
	}

	// send all of the messages
	for (i = 0; i < 30; i ++) {
		g_test_fixture.delayed_send(g_test_proc[5].pid, delay_array[delay_order[i]], delays[delay_order[i]]);
	}

	// block until process5 is done
	g_test_fixture.release_memory_block(g_test_fixture.receive_message(NULL));


	#ifdef _TEST_DEBUG
	rtx_dbug_outs((CHAR *)"G04_test: ");
	print_int((UINT32)success_count);
	rtx_dbug_outs((CHAR *)"/16 tests OK\r\n");
	rtx_dbug_outs((CHAR *)"G04_test: ");
	print_int((UINT32)fail_count);
	rtx_dbug_outs((CHAR *)"/16 tests FAIL\r\n");
	rtx_dbug_outs((CHAR *)"G04_test: END\r\n");
	#endif
}

//Higher priority 
void process1() {

	//Receive message from master
	VOID * receivemessage = g_test_fixture.receive_message(NULL);
	
	if (receivemessage != messagetoproc1)
	{
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 1 FAIL\n\r");
		fail_count++;
	#endif
		
	}  else {
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 1 OK\r\n");
		success_count++;
	#endif
		
	}
	
	
	MessageEnvelope * newmessage = (MessageEnvelope *) receivemessage;
	
	if(newmessage->tx != g_test_proc[0].pid) {
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 2 FAIL\n\r");
		fail_count++;
	#endif
		

	} else if(newmessage->rx != g_test_proc[1].pid) {
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 2 FAIL\n\r");
		fail_count++;
	#endif
		

	} else {
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 2 OK\n\r");
		success_count++;
	#endif
		
	}

	g_test_fixture.release_memory_block((void *)newmessage);
	
	int * tx = (int *)g_test_fixture.request_memory_block();
	
	//Block receive first from process2
	VOID * receivemessage2 = g_test_fixture.receive_message(tx);

	if (*tx != g_test_proc[2].pid) {
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 5 FAIL\r\n");
		fail_count++;
	#endif
		

	} else {
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 5 OK\r\n");
		success_count++;
	#endif
		
	}
	
	g_test_fixture.release_memory_block((void *)tx);

	if (receivemessage2 != proc2toproc1)
	{
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 6 FAIL\n\r");
		fail_count++;
	#endif
		
	}
	
	else {
	
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 6 OK\n\r");
		success_count++;
	#endif
	}
	
	MessageEnvelope * newmessage2 = (MessageEnvelope *) receivemessage2;
	
	if(newmessage2->tx!= g_test_proc[2].pid)
	{
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 7 FAIL\n\r");
		fail_count++;
	#endif
		

	} else if(newmessage2->rx != g_test_proc[1].pid) {
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 7 FAIL\n\r");
		fail_count++;
	#endif
		

	} else {
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 7 OK\n\r");
		success_count++;
	#endif
		
	}
	
	g_test_fixture.release_memory_block(newmessage2);
	
	//Sends message to master
	VOID * proc1tomaster = g_test_fixture.request_memory_block();
	g_test_fixture.send_message(g_test_proc[0].pid, proc1tomaster); 	
}

void process2() {

	MessageEnvelope * newmessageproc2 = (MessageEnvelope *)g_test_fixture.receive_message(NULL);

	if (newmessageproc2->tx == g_test_proc[0].pid) { 
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 4 OK\n\r");
		success_count++;
	#endif
		

	} else {
		
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 4 FAIL\r\n");
		fail_count++;
	#endif
		
	}

	g_test_fixture.release_memory_block(newmessageproc2);

	//sends message to proc 1
	proc2toproc1 = g_test_fixture.request_memory_block();
	g_test_fixture.send_message(g_test_proc[1].pid, proc2toproc1);
	
	//sends a message to master
	VOID * proc2tomaster = g_test_fixture.request_memory_block();
	g_test_fixture.send_message(g_test_proc[0].pid, proc2tomaster);
	
}

void process3() {
	
	MessageEnvelope * newmessage = (MessageEnvelope *)g_test_fixture.receive_message(NULL);
	
	if(newmessage->tx != g_test_proc[0].pid) {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 14 FAIL\n\r");
		fail_count++;
		#endif	
	} else if(newmessage->rx != g_test_proc[3].pid) {	
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 14 FAIL\n\r");
		fail_count++;
		#endif
	} else {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 14 OK\n\r");
		success_count++;
		#endif
	}
	
	g_test_fixture.release_memory_block(newmessage);
	
	//Immediately send a msg back to the master
	VOID * proc3_to_master = g_test_fixture.request_memory_block();
	g_test_fixture.send_message(g_test_proc[0].pid, proc3_to_master);
}

void process4() {
	MessageEnvelope * newmessage = (MessageEnvelope *)g_test_fixture.receive_message(NULL);
	
	if(newmessage->tx != g_test_proc[0].pid) {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 12 FAIL\n\r");
		fail_count++;
		#endif
	} else if(newmessage->rx != g_test_proc[4].pid) {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 12 FAIL\n\r");
		fail_count++;
		#endif
	} else {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs((CHAR *)"G04_test: test 12 OK\n\r");
		success_count++;
		#endif
	}
	
	g_test_fixture.release_memory_block(newmessage);
	
	//Immediately send a msg back to the master
	VOID * proc4_to_master = g_test_fixture.request_memory_block();
	g_test_fixture.send_message(g_test_proc[0].pid, proc4_to_master);

}

void process5() {
	int failed = 0;
	
	int i;
	for (i = 42; i < 72; i ++) {
		MessageEnvelope * newmessage = (MessageEnvelope *)g_test_fixture.receive_message(NULL);
		
		if (newmessage->type != i) {
			failed = 1;
			//rtx_dbug_outs_int("test 16 - expected: ", i - 42);
			//rtx_dbug_outs_int("test 16 - got: ", newmessage->type - 42);
		}

		g_test_fixture.release_memory_block(newmessage);
	}

	if (failed) {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs("G04_test: test 16 FAIL\r\n");
		#endif

		fail_count ++;
	} else {
		#ifdef _TEST_DEBUG
		rtx_dbug_outs("G04_test: test 16 OK\n\r");
		#endif

		success_count++;
	}

	g_test_fixture.delayed_send(g_test_proc[0].pid, g_test_fixture.request_memory_block(), 50);
}

void __attribute__ ((section ("__REGISTER_TEST_PROCS__")))register_test_proc()
{
	int i;

	for (i =0; i< NUM_TEST_PROCS; i++ ) {
		g_test_proc[i].pid = 1 + i;
		g_test_proc[i].sz_stack = 1024;
	}

	g_test_proc[0].priority = 3;
	g_test_proc[1].priority = 1;
	g_test_proc[2].priority = 2;
	g_test_proc[3].priority = 2;
	g_test_proc[4].priority = 2;
	g_test_proc[5].priority = 3;

	g_test_proc[0].entry = &test_master;
	g_test_proc[1].entry = &process1;
	g_test_proc[2].entry = &process2;
	g_test_proc[3].entry = &process3;
	g_test_proc[4].entry = &process4;
	g_test_proc[5].entry = &process5;
}

int main(void)
{
	return 0;
}

