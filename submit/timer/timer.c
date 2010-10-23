#include "../shared/rtx_inc.h"
#include "../shared/dbug.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_mem.h"
#include "../shared/rtx_pids.h"
#include "../shared/rtx_core.h"

UINT32 ticks;

VOID init_timer() 
{
	#ifdef _TIMER_DEBUG
	rtx_dbug_outs("Initializing timer\n\r");
	#endif
	
	ticks = 0;

	// store the timer handler at autovector 6
	asm("MOVE.L #asm_timer_handler,%d0");
	asm("MOVE.L %d0, 0x10000078");

	// use autovector 6, priority 3
	TIMER0_ICR = 0x9B;

	// reset TRR
	TIMER0_TRR = 0x02B4;

	// turn the timer on and set the prescaler to tick once per millisecond
	TIMER0_TMR = 0x401B;
}

VOID timer_isr()
{
	// acknowledge the interrupt
	TIMER0_TER = 0x2;
	
	ticks ++;

	#ifdef _TIMER_DEBUG
	rtx_dbug_outs_int("Time: ", ticks / 1000);
	#endif

	// notify messaging system
	check_existing_delayed_message();
}

UINT32 get_time()
{
	return ticks;
}
