/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

/**
 * @file
 * Holds the structures for the eal internal configuration
 */

#ifndef EAL_INTERNAL_CFG_H
#define EAL_INTERNAL_CFG_H

#include <rte_eal.h>
#include <rte_pci_dev_feature_defs.h>

#include "eal_thread.h"

#if defined(RTE_ARCH_ARM) || defined(RTE_ARCH_ARM64)
#define MAX_HUGEPAGE_SIZES 4  /**< support up to 4 page sizes */
#else
#define MAX_HUGEPAGE_SIZES 3  /**< support up to 3 page sizes */
#endif

/*
 * internal configuration structure for the number, size and
 * mount points of hugepages
 */
// zhou: DPDK support several size Hugepage at the same time.
struct hugepage_info {
	uint64_t hugepage_sz;   /**< size of a huge page */
	char hugedir[PATH_MAX];    /**< dir where hugetlbfs is mounted */

	uint32_t num_pages[RTE_MAX_NUMA_NODES];
	/**< number of hugepages of that size on each socket */

	int lock_descriptor;    /**< file descriptor for hugepage dir */
};

/**
 * internal configuration
 */
// zhou: eal commandline.
struct internal_config {

    // zhou: "However if --legacy-mem is used, DPDK will attempt to preallocate
    //       all memory it can get to, and memory use must be explicitly limited.
    //       This is done by passing the flag to each process to specify how much
    //       -m hugepage memory, in megabytes, each process can use (or passing
    //       --socket-mem to specify how much hugepage memory on each socket each
    //       process can use)."
    //
    //       Once in Dynamic mode, and --no-huge speicied, DPDK will allocate
    //       MEMSIZE_IF_NO_HUGE_PAGE==64MB each time from OS.
	volatile size_t memory;           /**< amount of asked memory */

    // zhou: physical memory, why let user to specify it without any knowledge ???
	volatile unsigned force_nchannel; /**< force number of channels */
	volatile unsigned force_nrank;    /**< force number of ranks */

    // zhou: disable Hugepage by "--no-huge" option.
    //       Even if we have no Hugepage support, we still can use mmap to manage
    //       memory directly.
	volatile unsigned no_hugetlbfs;   /**< true to disable hugetlbfs */
    // zhou: "--huge-unlink"
	unsigned hugepage_unlink;         /**< true to unlink backing files */

	volatile unsigned no_pci;         /**< true to disable PCI */
	volatile unsigned no_hpet;        /**< true to disable HPET */
	volatile unsigned vmware_tsc_map; /**< true to use VMware TSC mapping
										* instead of native TSC */

    // zhou: "--no-shconf", avoid other process work as secondard process.
	volatile unsigned no_shconf;      /**< true if there is no shared config */

    // zhou: in-memory is a superset of noshconf and huge-unlink.
	volatile unsigned in_memory;
	/**< true if DPDK should operate entirely in-memory and not create any
	 * shared files or runtime data.
	 */

	volatile unsigned create_uio_dev; /**< true to create /dev/uioX devices */

    // zhou: "--proc-type"
	volatile enum rte_proc_type_t process_type; /**< multi-process proc type */

    // zhou: once "--socket-mem" is specified.
	/** true to try allocating memory on specific sockets */
	volatile unsigned force_sockets;
    // zhou: specified using "--socket-mem"
    //       pre-alloc memory per CPU socket, and it will not be released even
    //       not be used any more. More memory could be alloc even we have been
    //       set it. Should avoid use it in Linux normally.
	volatile uint64_t socket_mem[RTE_MAX_NUMA_NODES]; /**< amount of memory per socket */

    // zhou: once "--socket-limit" specified
	volatile unsigned force_socket_limits;
    // zhou: "--socket-limit", used to limit the total memory could be allocated
    //       in this CPU socket.
	volatile uint64_t socket_limit[RTE_MAX_NUMA_NODES]; /**< limit amount of memory per socket */

	uintptr_t base_virtaddr;          /**< base address to try and reserve memory from */

    // zhou: could be specified using "--legacy-mem"
	volatile unsigned legacy_mem;
	/**< true to enable legacy memory behavior (no dynamic allocation,
	 * IOVA-contiguous segments).
	 */
	volatile unsigned match_allocations;
	/**< true to free hugepages exactly as allocated */
    // zhou: could be specified using "--single-file-segments", used for vhost-user.
	volatile unsigned single_file_segments;
	/**< true if storing all pages within single files (per-page-size,
	 * per-node) non-legacy mode only.
	 */

    // zhou: could be specified using "--OPT_SYSLOG", used by openlog().
	volatile int syslog_facility;	  /**< facility passed to openlog() */

	/** default interrupt mode for VFIO */
	volatile enum rte_intr_mode vfio_intr_mode;
    // zhou: could be specified using "--file-prefix"
	char *hugefile_prefix;      /**< the base filename of hugetlbfs files */

    // zhou: user could specify Hugepage mount point by "--huge-dir" option, in
    //       order to specify which hugetlbfs will be used.
	char *hugepage_dir;         /**< specific hugetlbfs directory to use */
	char *user_mbuf_pool_ops_name;
			/**< user defined mbuf pool ops name */

    // zhou: support up to MAX_HUGEPAGE_SIZES.
	unsigned num_hugepage_sizes;      /**< how many sizes on this system */
    // zhou: sort by Hugepage Size in descend.
	struct hugepage_info hugepage_info[MAX_HUGEPAGE_SIZES];

    // zhou: "--iova-mode=<pa|va>"
	enum rte_iova_mode iova_mode ;    /**< Set IOVA mode on this system  */
	rte_cpuset_t ctrl_cpuset;         /**< cpuset for ctrl threads */
	volatile unsigned int init_complete;
	/**< indicates whether EAL has completed initialization */
};
extern struct internal_config internal_config; /**< Global EAL configuration. */

void eal_reset_internal_config(struct internal_config *internal_cfg);

#endif /* EAL_INTERNAL_CFG_H */
