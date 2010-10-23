#include "../shared/rtx_inc.h"
#include "../shared/dbug.h"
#include "../shared/rtx_core.h"
#include "../shared/rtx_pids.h"

void core_test1() {
	rtx_dbug_outs("Running core test 1\r\n");
	unblock_process_by_pid(CORE_TEST2_PID, 1);
}

void core_test2() {
	rtx_dbug_outs("Running core test 2\r\n");
	block_process(CORE_TEST2_PID, mem);
	rtx_dbug_outs("Core test 2 unblocked\r\n");
}
