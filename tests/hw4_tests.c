#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "polya.h"

#define TEST_TIMEOUT 400
/*
 * These tests just test the student worker using the demo master.
 * This should expose issues with the worker, since the demo master
 * will track whether the workers crash and whether they actually
 * solve any problems.
 *
 * Tests to be used in grading will track the behavior of the master
 * via the event functions that it is to call.  No such tests are
 * included here with the basecode.
 */

Test(demo_master_suite, startup_test, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, trivial_test, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -p 1 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_one_worker, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -p 5 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_three_workers, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -p 5 -t 2 -w 3";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

/////////////////////////////////////////////////////////////////////////////////////

Test(demo_master_suite, miner_test_6w_p10_t2_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 6 -p 10 -t 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_6w_p10_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 6 -p 10 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_6w_p10_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 6 -p 10 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_6w_p8_t2_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 6 -p 8 -t 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_6w_p8_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 6 -p 8 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_6w_p8_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 6 -p 8 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_4w_p10, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 4 -p 10";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_4w_p10_t2_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 4 -p 10 -t 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_4w_p10_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 4 -p 10 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_4w_p10_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 4 -p 10 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_4w_p6_t2_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 4 -p 6 -t 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_3w_p10_t2_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 3 -p 10 -t 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_3w_p10_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 3 -p 10 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_3w_p10_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 3 -p 10 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_3w_p6_t1_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 3 -p 6 -t 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_3w_p6_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 3 -p 6 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_3w_p6_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 3 -p 6 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_2w_p10_t2_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 2 -p 10 -t 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_2w_p10_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 2 -p 10 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_2w_p10_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 2 -p 10 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_2w_p6_t2_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 2 -p 6 -t 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_2w_p6_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 2 -p 6 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_2w_p6_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 2 -p 6 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_2w_p1_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 2 -p 1 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_2w_p1_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 2 -p 1 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_1w_p10_t1_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 1 -p 10 -t 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_1w_p10_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 1 -p 10 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_1w_p10_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 1 -p 10 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_1w_p2_t1_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 1 -p 2 -t 1 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_1w_p2_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 1 -p 2 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_1w_p2_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 1 -p 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_1w_p1_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 1 -p 1 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_1w_p0_t1_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 1 -p 0 -t 2 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_1w_p0_t2, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 1 -p 0 -t 2";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(demo_master_suite, miner_test_1w_p0_t1, .timeout = TEST_TIMEOUT) {
    char *cmd = "demo/polya -w 1 -p 0 -t 1";
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}