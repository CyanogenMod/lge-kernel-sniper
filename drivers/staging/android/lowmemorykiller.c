/* drivers/misc/lowmemorykiller.c
 *
 * The lowmemorykiller driver lets user-space specify a set of memory thresholds
 * where processes with a range of oom_adj values will get killed. Specify the
 * minimum oom_adj values in /sys/module/lowmemorykiller/parameters/adj and the
 * number of free pages in /sys/module/lowmemorykiller/parameters/minfree. Both
 * files take a comma separated list of numbers in ascending order.
 *
 * For example, write "0,8" to /sys/module/lowmemorykiller/parameters/adj and
 * "1024,4096" to /sys/module/lowmemorykiller/parameters/minfree to kill processes
 * with a oom_adj value of 8 or higher when the free memory drops below 4096 pages
 * and kill processes with a oom_adj value of 0 or higher when the free memory
 * drops below 1024 pages.
 *
 * The driver considers memory used for caches to be free, but if a large
 * percentage of the cached memory is locked this can be very inaccurate
 * and processes may not get killed until the normal oom killer is triggered.
 *
 * Copyright (C) 2007-2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/sched.h>
#include <linux/notifier.h>
#include <linux/compaction.h>

//<!-- BEGIN: hyeongseok.kim@lge.com 2012-08-16 -->
//<!-- MOD : make LMK see swap condition 
//DEL : bs.lim@lge.com
/*#include <linux/swap.h>
#include <linux/fs.h>
#include <linux/slab.h>*/
//<!--  END: hyeongseok.kim@lge.com 2012-08-16 -->


static uint32_t lowmem_debug_level = 2;
static int lowmem_adj[6] = {
	0,
	1,
	6,
	12,
};
static int lowmem_adj_size = 4;
static size_t lowmem_minfree[6] = {
	3 * 512,	/* 6MB */
	2 * 1024,	/* 8MB */
	4 * 1024,	/* 16MB */
	16 * 1024,	/* 64MB */
};
static int lowmem_minfree_size = 4;

//<!-- BEGIN: hyeongseok.kim@lge.com 2012-08-16 -->
//<!-- MOD : make LMK see swap condition
//DEL : bs.lim@lge.com
/*#define LMK_SWAP_MINFREE_INIT (96 * 1024)
#define LMK_SWAP_MIN_KBYTES	(16*1024)
#define LMK_SWAP_DEC_KBYTES (8*1024)
unsigned long lmk_count = 0UL;
unsigned long min_free_swap = LMK_SWAP_MINFREE_INIT;
char *lmk_kill_info = 0;*/
//<!-- END: hyeongseok.kim@lge.com 2012-08-16 -->


static struct task_struct *lowmem_deathpending;
static unsigned long lowmem_deathpending_timeout;

extern int compact_nodes(bool sync);

#define lowmem_print(level, x...)			\
	do {						\
		if (lowmem_debug_level >= (level))	\
			printk(x);			\
	} while (0)

static int
task_notify_func(struct notifier_block *self, unsigned long val, void *data);

static struct notifier_block task_nb = {
	.notifier_call	= task_notify_func,
};

static int
task_notify_func(struct notifier_block *self, unsigned long val, void *data)
{
	struct task_struct *task = data;

	if (task == lowmem_deathpending)
		lowmem_deathpending = NULL;

	return NOTIFY_OK;
}

static int lowmem_shrink(struct shrinker *s, struct shrink_control *sc)
{
	struct task_struct *p;
	struct task_struct *selected = NULL;
	int rem = 0;
	int tasksize;
	int i;
	int min_adj = OOM_ADJUST_MAX + 1;
	int selected_tasksize = 0;
	int selected_oom_adj;
	int array_size = ARRAY_SIZE(lowmem_adj);
	int other_free = global_page_state(NR_FREE_PAGES);
	int other_file = global_page_state(NR_FILE_PAGES) -
						global_page_state(NR_SHMEM);
	int other_file_pages = global_page_state(NR_FILE_PAGES);
	int other_file_shmem = global_page_state(NR_SHMEM);

//<!-- BEGIN: hyeongseok.kim@lge.com 2012-08-16 -->
//<!-- MOD : make LMK see swap condition 
//DEL : bs.lim@lge.com
/*	struct sysinfo sysi;
	si_swapinfo(&sysi);*/

	/* 
	 *	- increase min_free_swap progressively, 
	 *	   in case gap between free-swap and min_free_swap becomes bigger than 
	 *	   LMK_SWAP_DEC_KBYTES.
	 *	- must be considered initial value of min_free_swap.
	 */
	 
/*
	if( sysi.freeswap < (LMK_SWAP_MINFREE_INIT+LMK_SWAP_DEC_KBYTES)>>2 && 
		sysi.freeswap > (min_free_swap+LMK_SWAP_DEC_KBYTES)>>2)
		min_free_swap += LMK_SWAP_DEC_KBYTES;

	if(sysi.totalswap !=0 && sysi.freeswap < min_free_swap>>2) {
		other_file = 0;
	} else {
		other_file -= total_swapcache_pages;
		if(other_file < 0)
			other_file = 0;
	}*/
	//lowmem_print(1, "lmk min_free_swap=%dK, free_swap=%dK, RunLMK=%s\n", min_free_swap, sysi.freeswap*4, other_file==0?"TRUE":"FALSE");
//<!-- END: hyeongseok.kim@lge.com 2012-08-16 -->

	/*
	 * If we already have a death outstanding, then
	 * bail out right away; indicating to vmscan
	 * that we have nothing further to offer on
	 * this pass.
	 *
	 */
	if (lowmem_deathpending &&
	    time_before_eq(jiffies, lowmem_deathpending_timeout))
		return 0;

	if (lowmem_adj_size < array_size)
		array_size = lowmem_adj_size;
	if (lowmem_minfree_size < array_size)
		array_size = lowmem_minfree_size;
	for (i = 0; i < array_size; i++) {
		if (other_free < lowmem_minfree[i] &&
		    other_file < lowmem_minfree[i]) {
			min_adj = lowmem_adj[i];
			break;
		}
	}
	if (sc->nr_to_scan > 0)
		lowmem_print(3, "lowmem_shrink %lu, %x, ofree %d %d(=%d-%d), ma %d\n",
			     sc->nr_to_scan, sc->gfp_mask, other_free, other_file, 
			     other_file_pages, other_file_shmem, min_adj);
	rem = global_page_state(NR_ACTIVE_ANON) +
		global_page_state(NR_ACTIVE_FILE) +
		global_page_state(NR_INACTIVE_ANON) +
		global_page_state(NR_INACTIVE_FILE);
	if (sc->nr_to_scan <= 0 || min_adj == OOM_ADJUST_MAX + 1) {
		lowmem_print(5, "lowmem_shrink %lu, %x, return %d\n",
			     sc->nr_to_scan, sc->gfp_mask, rem);
		return rem;
	}
	selected_oom_adj = min_adj;
	
//<!-- BEGIN: hyeongseok.kim@lge.com 2012-08-16 -->
//<!-- MOD : make LMK see swap condition 
//DEL : bs.lim@lge.com
/*	if(other_file == 0 && min_free_swap > LMK_SWAP_MIN_KBYTES-1)
		min_free_swap -= LMK_SWAP_DEC_KBYTES;*/
//<!-- END: hyeongseok.kim@lge.com 2012-08-16 -->

	read_lock(&tasklist_lock);
	for_each_process(p) {
		struct mm_struct *mm;
		struct signal_struct *sig;
		int oom_adj;

		task_lock(p);
		mm = p->mm;
		sig = p->signal;
		if (!mm || !sig) {
			task_unlock(p);
			continue;
		}
		oom_adj = sig->oom_adj;
		if (oom_adj < min_adj) {
			task_unlock(p);
			continue;
		}
		tasksize = get_mm_rss(mm);
		task_unlock(p);
		if (tasksize <= 0)
			continue;
		if (selected) {
			if (oom_adj < selected_oom_adj)
				continue;
			if (oom_adj == selected_oom_adj &&
			    tasksize <= selected_tasksize)
				continue;
		}
		selected = p;
		selected_tasksize = tasksize;
		selected_oom_adj = oom_adj;
	//kiyong.choi@lge.com (+)
		if(lowmem_deathpending && selected != lowmem_deathpending)
		{
		   if(selected_oom_adj > 5){
				force_sig(SIGKILL, selected);
				lowmem_print(1, "time out send sigkill to %d (%s), adj %d, size %d ****\n",
					 selected->pid, selected->comm,
					 selected_oom_adj, selected_tasksize);
				selected=NULL;
				continue;
		   }
		}
    	//kiyong.choi@lge.com (-)
		lowmem_print(2, "select %d (%s), adj %d, size %d, to kill\n",
			     p->pid, p->comm, oom_adj, tasksize);
	}
	if (selected) {
		lowmem_print(1, "send sigkill to %d (%s), adj %d, size %d\n",
			     selected->pid, selected->comm,
			     selected_oom_adj, selected_tasksize);
//<!-- BEGIN: hyeongseok.kim@lge.com 2012-08-16 -->
//<!-- MOD : make LMK see swap condition 
//DEL : bs.lim@lge.com
/*	lmk_count++;
		if(lmk_kill_info)
			sprintf(lmk_kill_info, "%ul,%s,%d,%d\n",	lmk_count,
													selected->comm,
													selected_oom_adj,
													selected_tasksize);*/
//<!-- END: hyeongseok.kim@lge.com 2012-08-16 -->

		lowmem_deathpending = selected;
    //kiyong.choi@lge.com (+)
		lowmem_deathpending_timeout = jiffies + (3*HZ/10);
    //kiyong.choi@lge.com (-)
    
		//lowmem_deathpending_timeout = jiffies + HZ;		
		force_sig(SIGKILL, selected);
		rem -= selected_tasksize;
	}
	lowmem_print(4, "lowmem_shrink %lu, %x, return %d\n",
		     sc->nr_to_scan, sc->gfp_mask, rem);
	read_unlock(&tasklist_lock);
    if (selected)
        compact_nodes(false);
	return rem;
}

static struct shrinker lowmem_shrinker = {
	.shrink = lowmem_shrink,
	.seeks = DEFAULT_SEEKS * 16
};

static int __init lowmem_init(void)
{
//<!-- BEGIN: hyeongseok.kim@lge.com 2012-08-16 -->
//<!-- MOD : make LMK see swap condition 
//DEL : bs.lim@lge.com
/*	lmk_kill_info = kmalloc(1024, GFP_KERNEL);*/
//<!-- END: hyeongseok.kim@lge.com 2012-08-16 -->

	task_free_register(&task_nb);
	register_shrinker(&lowmem_shrinker);
	return 0;
}

static void __exit lowmem_exit(void)
{
	unregister_shrinker(&lowmem_shrinker);
	task_free_unregister(&task_nb);
//<!-- BEGIN: hyeongseok.kim@lge.com 2012-08-16 -->
//<!-- MOD : make LMK see swap condition 
//DEL : bs.lim@lge.com
/*	if(lmk_kill_info)
		kfree(lmk_kill_info);*/
//<!-- END: hyeongseok.kim@lge.com 2012-08-16 -->

}

module_param_named(cost, lowmem_shrinker.seeks, int, S_IRUGO | S_IWUSR);
module_param_array_named(adj, lowmem_adj, int, &lowmem_adj_size,
			 S_IRUGO | S_IWUSR);
module_param_array_named(minfree, lowmem_minfree, uint, &lowmem_minfree_size,
			 S_IRUGO | S_IWUSR);
module_param_named(debug_level, lowmem_debug_level, uint, S_IRUGO | S_IWUSR);
//<!-- BEGIN: hyeongseok.kim@lge.com 2012-08-16 -->
//<!-- MOD : make LMK see swap condition 
//DEL : bs.lim@lge.com
/*module_param_named(lmksts, lmk_kill_info, charp, S_IRUGO | S_IWUSR);
module_param_named(min_free_swap, min_free_swap, ulong, S_IRUGO | S_IWUSR);*/
//<!-- END: hyeongseok.kim@lge.com 2012-08-16 -->

module_init(lowmem_init);
module_exit(lowmem_exit);

MODULE_LICENSE("GPL");

