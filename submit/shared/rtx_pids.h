#ifndef _RTX_PIDS_H_
#define _RTX_PIDS_H_

#define NULL_PID 0

// stress tests
#define STRESS_A_PID 7
#define STRESS_B_PID 8
#define STRESS_C_PID 9

// io
#define KCD_PID 10
#define	CRT_PID 11
#define UART_PID 12

// mem tests
#define MEM_TEST1_PID 20
#define MEM_TEST2_PID 21
#define MEM_TEST3_PID 22
#define MEM_TEST4_PID 23
#define MEM_TEST5_PID 24
#define MEM_TEST6_PID 25
#define MEM_TEST7_PID 26

#define MEM_TEST_MASTER_PID 27
#define MEM_TEST_BLOCKING1_PID 28
#define MEM_TEST_BLOCKING2_PID 29

// msgs tests (30-series and 40-series allocated)
#define MSG_TEST_MASTER_PID 30 
#define MSG_TEST1_PID 31
#define MSG_TEST2_PID 32
#define MSG_TEST5_PID 36
#define MSG_TEST6_PID 37
#define MSG_TEST7_PID 38
#define MSG_TEST8_PID 39
#define MSG_TEST3_MASTER_PID 35 
#define MSG_TEST9_PID 33
#define MSG_TEST10_PID 34
#define MSG_TEST11_PID 40

// a PID that can be used for testing an invalid PID
#define INVALID_PID 50

// PIDs for various user-level processes
#define WALLCLOCK_PID 60

// core tests
#define CORE_TEST1_PID 70
#define CORE_TEST2_PID 71

// priority switch command handler
#define PRIORITY_SWITCH_PID 80

#endif
