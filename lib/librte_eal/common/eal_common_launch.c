/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/queue.h>

#include <rte_launch.h>
#include <rte_memory.h>
#include <rte_eal.h>
#include <rte_atomic.h>
#include <rte_pause.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>

#include "eal_private.h"

/*
 * Wait until a lcore finished its job.
 */
int
rte_eal_wait_lcore(unsigned worker_id)
{
	if (lcore_config[worker_id].state == WAIT)
		return 0;

    // zhou: CPU nop, will block here until thread become state "WAIT" (never execute)
    //       function or "FINISHED" (execution completed).

	while (lcore_config[worker_id].state != WAIT &&
	       lcore_config[worker_id].state != FINISHED)
		rte_pause();

	rte_rmb();

    // zhou: when function executed completed, its state becomes "FINISHED".
    //       Just like pthread, when created with Joinable attribute, must need
    //       pthread_join() to free its resource.
    //

	/* we are in finished state, go to wait state */
	lcore_config[worker_id].state = WAIT;
	return lcore_config[worker_id].ret;
}

// zhou: because this function try to launch one same function on all lcores.
//       So we have to make sure all of cores stay in state "WAIT", otherwise
//       failure with EBUSY.
/*
 * Check that every WORKER lcores are in WAIT state, then call
 * rte_eal_remote_launch() for all of them. If call_main is true
 * (set to CALL_MAIN), also call the function on the main lcore.
 */
int
rte_eal_mp_remote_launch(int (*f)(void *), void *arg,
			 enum rte_rmt_call_main_t call_main)
{
	int lcore_id;
	int main_lcore = rte_get_main_lcore();

    // zhou: check without block like rte_eal_wait_lcore().
	/* check state of lcores */
	RTE_LCORE_FOREACH_WORKER(lcore_id) {
		if (lcore_config[lcore_id].state != WAIT)
			return -EBUSY;
	}

	/* send messages to cores */
	RTE_LCORE_FOREACH_WORKER(lcore_id) {
        // zhou: implemented in linuxapp/eal/eal_thread.c, it is platform related.
		rte_eal_remote_launch(f, arg, lcore_id);
	}

	if (call_main == CALL_MAIN) {
		lcore_config[main_lcore].ret = f(arg);
		lcore_config[main_lcore].state = FINISHED;
	}

	return 0;
}

// zhou: check thread state.
/*
 * Return the state of the lcore identified by worker_id.
 */
enum rte_lcore_state_t
rte_eal_get_lcore_state(unsigned lcore_id)
{
	return lcore_config[lcore_id].state;
}

// zhou: wait for all threads stop working.
/*
 * Do a rte_eal_wait_lcore() for every lcore. The return values are
 * ignored.
 */
void
rte_eal_mp_wait_lcore(void)
{
	unsigned lcore_id;

	RTE_LCORE_FOREACH_WORKER(lcore_id) {
		rte_eal_wait_lcore(lcore_id);
	}
}
