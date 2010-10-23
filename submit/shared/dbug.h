/**
 * @file: dbug.h
 * @author: ECE354 Lab Instructors and TAs
 * @author: Irene Huang  
 * @date: 2010/05/03
 * @brief: Header file of dbug.c.  
 */
#ifndef _DBUG_H_
#define _DBUG_H_

#include "rtx_inc.h"

VOID rtx_dbug_out_char( CHAR c );   // output a char to janusROM terminal
SINT32 rtx_dbug_outs( CHAR* s );    // output a string to janusROM terminal
VOID print_int(UINT32); // output a number in decimal to the janusROM terminal
VOID rtx_dbug_outs_int(CHAR *, UINT32);

#endif /* _DBUG_H_ */

//#define _CFSERVER_
//#define _MEM_DEBUG
//#define _MEM_TEST_DEBUG
//#define _IO_DEBUG
//#define _MSG_TEST_DEBUG
//#define _MSG_DEBUG
//#define _WALLCLOCK_DEBUG
//#define _CORE_DEBUG
//#define _TEST_DEBUG
//#define _TIMER_DEBUG
//#define _STRESS_DEBUG
//#define _HOTKEYS_DEBUG
