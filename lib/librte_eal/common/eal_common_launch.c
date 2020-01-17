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
rte_eal_wait_lcore(unsigned slave_id)
{
	if (lcore_config[slave_id].state == WAIT)
		return 0;

    // zhou: CPU nop, will block here until thread become state "WAIT" (never execute)
    //       function or "FINISHED" (execution completed).
	while (lcore_config[slave_id].state != WAIT &&
	       lcore_config[slave_id].state != FINISHED)
		rte_pause();

	rte_rmb();

    // zhou: when function executed completed, its state becomes "FINISHED".
    //       Just like pthread, when created with Joinable attribute, must need
    //       pthread_join() to free its resource.
    //

	/* we are in finished state, go to wait state */
	lcore_config[slave_id].state = WAIT;

	return lcore_config[slave_id].ret;
}

// zhou: because this function try to launch one same function on all lcores.
//       So we have to make sure all of cores stay in state "WAIT", otherwise
//       failure with EBUSY.
/*
 * Check that every SLAVE lcores are in WAIT state, then call
 * rte_eal_remote_launch() for all of them. If call_master is true
 * (set to CALL_MASTER), also call the function on the master lcore.
 */
int
rte_eal_mp_remote_launch(int (*f)(void *), void *arg,
			 enum rte_rmt_call_master_t call_master)
{
	int lcore_id;
	int master = rte_get_master_lcore();

    // zhou: check without block like rte_eal_wait_lcore().
	/* check state of lcores */
	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
		if (lcore_config[lcore_id].state != WAIT)
			return -EBUSY;
	}

	/* send messages to cores */
	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        // zhou: implemented in linuxapp/eal/eal_thread.c, it is platform related.
		rte_eal_remote_launch(f, arg, lcore_id);
	}

	if (call_master == CALL_MASTER) {
		lcore_config[master].ret = f(arg);
		lcore_config[master].state = FINISHED;
	}

	return 0;
}

// zhou: check thread state.
/*
 * Return the state of the lcore identified by slave_id.
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

	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
		rte_eal_wait_lcore(lcore_id);
	}
}
