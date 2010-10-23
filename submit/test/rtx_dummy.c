/*--------------------------------------------------------------------------
 *                      RTX Test Suite 
 *--------------------------------------------------------------------------
 */
/**
 * @file:   rtx_test_dummy.c   
 * @author: Thomas Reidemeister
 * @author: Irene Huang
 * @date:   2010.02.11
 * @brief:  rtx test suite 
 */

#include "rtx_test.h"
#include "../shared/dbug.h"

/* third party dummy test process 1 */ 
void test1()
{
    int i;
    for (i = 0; i < 10; i ++) 
    {
#ifdef _TEST_DEBUG
    	rtx_dbug_outs((CHAR *)"rtx_test: test1\r\n");
#endif	
	/* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 2 */ 
void test2()
{
    int i;
    for (i = 0; i < 10; i ++) 
    {
#ifdef _TEST_DEBUG
    	rtx_dbug_outs((CHAR *)"rtx_test: test2\r\n");
#endif
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 3 */ 
void test3()
{
    int i;
    for (i = 0; i < 10; i ++) 
    {
#ifdef _TEST_DEBUG
    	rtx_dbug_outs((CHAR *)"rtx_test: test3\r\n");
#endif
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}

/* third party dummy test process 4 */ 
void test4()
{
    int i;
    for (i = 0; i < 10; i ++) 
    {
#ifdef _TEST_DEBUG
    	rtx_dbug_outs((CHAR *)"rtx_test: test4\r\n");
#endif
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 5 */ 
void test5()
{
    int i;
    for (i = 0; i < 10; i ++) 
    {
#ifdef _TEST_DEBUG
    	rtx_dbug_outs((CHAR *)"rtx_test: test5\r\n");
#endif
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}
/* third party dummy test process 6 */ 
void test6()
{
    int i;
    for (i = 0; i < 10; i ++) 
    {
#ifdef _TEST_DEBUG
    	rtx_dbug_outs((CHAR *)"rtx_test: test6\r\n");
#endif
        /* execute a rtx primitive to test */
        g_test_fixture.release_processor();
    }
}

/* register the third party test processes with RTX */
void __attribute__ ((section ("__REGISTER_TEST_PROCS__")))register_test_proc()
{
    int i;

#ifdef _TEST_DEBUG
    rtx_dbug_outs((CHAR *)"rtx_test: register_test_proc()\r\n");
#endif

    for (i =0; i< NUM_TEST_PROCS; i++ ) {
        g_test_proc[i].pid = i + 1;
        g_test_proc[i].priority = 2;
        g_test_proc[i].sz_stack = 2048;
    }

    g_test_proc[0].entry = test1;
    g_test_proc[1].entry = test2;
    g_test_proc[2].entry = test3;
    g_test_proc[3].entry = test4;
    g_test_proc[4].entry = test5;
    g_test_proc[5].entry = test6;
}

/**
 * Main entry point for this program.
 * never get invoked
 */
int main(void)
{
#ifdef _TEST_DEBUG
    rtx_dbug_outs((CHAR *)"rtx_test: started\r\n");
#endif

    return 0;
}

