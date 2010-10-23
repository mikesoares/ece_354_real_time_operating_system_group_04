/*--------------------------------------------------------------------------
 *					  RTX and Test Suite Loader
 *--------------------------------------------------------------------------
 */
/**
 * @file:   rtx_loader.c   
 * @author: Thomas Reidemeister
 * @author: Irene Huang
 * @date:   2010.05.15
 *  @brief:  To register third party test processes with rtx and load rtx 
 */

#include "../test/rtx_test.h"
#include "../shared/dbug.h"

/* the RTX entry point, see rtx_loader.ld */
extern void __RTX_ENTRY__();	

/* rtx primitive registration function provided by the rtx 
 * rtx needs to register its primitives with test suite
 * The __REGISTER_RTX_ENTRY__ symbol is defined in 
 * linker scripts
 */
extern void __REGISTER_RTX_ENTRY__();

/**
 * Main entry point for this program.
 */
int main(void)
{
	#ifdef _CORE_DEBUG			  
		rtx_dbug_outs("rtx_loader: started\r\n");
		rtx_dbug_outs("rtx_loader: calling __REGISTER_RTX_ENTRY__()...\r\n");
	#endif

	/* register rtx primitives in the test data structure */
	__REGISTER_RTX_ENTRY__();
	
	#ifdef _CORE_DEBUG
		rtx_dbug_outs("rtx_loader: calling __RTX_ENTRY__()...\r\n");
	#endif
	
	/* start the rtx */
	__RTX_ENTRY__();

	/* should never reach here */
	#ifdef _CORE_DEBUG
		rtx_dbug_outs("rtx_loader: out of rtx, not good!!!\r\n");
	#endif

	while(1) {}
	
	return 0;
}

