/*
 * drivers/gpu/ion/ion_carveout_heap.c
 *
 * Copyright (C) 2011 Google, Inc.
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
#include <linux/spinlock.h>

#include <linux/err.h>
#include <linux/genalloc.h>
#include <linux/io.h>
#include <linux/ion.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "ion_priv.h"

#include <asm/mach/map.h>

#include "../arch/arm/mach-omap2/omap_ion.h"
#include <linux/sched.h>
#include <linux/signal.h>

struct ion_carveout_heap {
	struct ion_heap heap;
	struct gen_pool *pool;
	ion_phys_addr_t base;
};

// rt5604@mnbt.co.kr 2012.08.06 to prevent system reboot due to ION Memory short for camera & video application during ART test ->
long mnTotalIonMemory = OMAP3_ION_HEAP_CARVEOUT_INPUT_SIZE;
// rt5604@mnbt.co.kr 2012.08.06 to prevent system reboot due to ION Memory short for camera & video application during ART test <-

ion_phys_addr_t ion_carveout_allocate(struct ion_heap *heap,
				      unsigned long size,
				      unsigned long align)
{
	struct ion_carveout_heap *carveout_heap =
		container_of(heap, struct ion_carveout_heap, heap);
	unsigned long offset = gen_pool_alloc(carveout_heap->pool, size);


	// rt5604@mnbt.co.kr 2012.08.06 to prevent system reboot due to ION Memory short for camera & video application during ART test ->
	if (strncmp("omap3_carveout", heap->name, 14) == 0) {
		if (offset) mnTotalIonMemory -= size;
		printk(KERN_DEBUG "rt5604: ---> ion_carveout_allocate:offset=0x%08lx, size=%ld.%03ldMB, Total=%ld.%03ldMB\n", offset, 
			size/(1024*1024), (size%(1024*1024))/1024,
			mnTotalIonMemory/(1024*1024), (mnTotalIonMemory%(1024*1024))/1024);
		if (mnTotalIonMemory < OMAP3_ION_HEAP_CARVEOUT_INPUT_SIZE/10) {	// 10%
			// Kill mediaserver to prevent hang-up during ART test
			struct task_struct *p, *pmediaserver = NULL;
			for_each_process(p) {
				printk("rt5604: ---> ion_carveout_allocate: process:%d:%s\n",p->pid,p->comm);
				if(!strncmp(p->comm,"mediaserver", 11)) {
					pmediaserver = p;
				}
				if (pmediaserver) break;
			}
			
			if(pmediaserver) {
				printk("rt5604: ---> ion_carveout_allocate: KILL %s (pid=%d)!!!\n", pmediaserver->comm, pmediaserver->pid);
				force_sig(SIGKILL, pmediaserver);
			}
		}
	}
	// rt5604@mnbt.co.kr 2012.08.06 to prevent system reboot due to ION Memory short for camera & video application during ART test <-
	
	if (!offset) {
		printk(KERN_DEBUG ">>>:ion_carveout_allocate:Allocation FAILURE!!!\n");
		return ION_CARVEOUT_ALLOCATE_FAIL;
	}

	return offset;
}

void ion_carveout_free(struct ion_heap *heap, ion_phys_addr_t addr,
		       unsigned long size)
{
	struct ion_carveout_heap *carveout_heap =
		container_of(heap, struct ion_carveout_heap, heap);

	if (addr == ION_CARVEOUT_ALLOCATE_FAIL) {
		printk(KERN_DEBUG ">>>:ion_carveout_free: FREE ERROR because of wrong addr, 0x%08lx\n", addr);
		return;
	}

	gen_pool_free(carveout_heap->pool, addr, size);
	
	// rt5604@mnbt.co.kr 2012.08.06 to prevent system reboot due to ION Memory short for camera & video application during ART test ->
	if (strncmp("omap3_carveout", heap->name, 14) == 0) {
		mnTotalIonMemory += size;

		printk(KERN_DEBUG "ion_carveout_free    :addr  =0x%08lx, size=%ld.%03ldMB, Total=%ld.%03ldMB\n", addr, 
			size/(1024*1024), (size%(1024*1024))/1024,
			mnTotalIonMemory/(1024*1024), (mnTotalIonMemory%(1024*1024))/1024);
	}
	// rt5604@mnbt.co.kr 2012.08.06 to prevent system reboot due to ION Memory short for camera & video application during ART test <-
}

static int ion_carveout_heap_phys(struct ion_heap *heap,
				  struct ion_buffer *buffer,
				  ion_phys_addr_t *addr, size_t *len)
{
	*addr = buffer->priv_phys;
	*len = buffer->size;
	return 0;
}

static int ion_carveout_heap_allocate(struct ion_heap *heap,
				      struct ion_buffer *buffer,
				      unsigned long size, unsigned long align,
				      unsigned long flags)
{
	buffer->priv_phys = ion_carveout_allocate(heap, size, align);
	return buffer->priv_phys == ION_CARVEOUT_ALLOCATE_FAIL ? -ENOMEM : 0;
}

static void ion_carveout_heap_free(struct ion_buffer *buffer)
{
	struct ion_heap *heap = buffer->heap;

	ion_carveout_free(heap, buffer->priv_phys, buffer->size);
	buffer->priv_phys = ION_CARVEOUT_ALLOCATE_FAIL;
}

struct scatterlist *ion_carveout_heap_map_dma(struct ion_heap *heap,
					      struct ion_buffer *buffer)
{
	return ERR_PTR(-EINVAL);
}

void ion_carveout_heap_unmap_dma(struct ion_heap *heap,
				 struct ion_buffer *buffer)
{
	return;
}

void *ion_carveout_heap_map_kernel(struct ion_heap *heap,
				   struct ion_buffer *buffer)
{
	return __arch_ioremap(buffer->priv_phys, buffer->size,
			      MT_MEMORY_NONCACHED);
}

void ion_carveout_heap_unmap_kernel(struct ion_heap *heap,
				    struct ion_buffer *buffer)
{
	__arch_iounmap(buffer->vaddr);
	buffer->vaddr = NULL;
	return;
}

int ion_carveout_heap_map_user(struct ion_heap *heap, struct ion_buffer *buffer,
			       struct vm_area_struct *vma)
{
	return remap_pfn_range(vma, vma->vm_start,
			       __phys_to_pfn(buffer->priv_phys) + vma->vm_pgoff,
			       buffer->size,
			       pgprot_noncached(vma->vm_page_prot));
}

static struct ion_heap_ops carveout_heap_ops = {
	.allocate = ion_carveout_heap_allocate,
	.free = ion_carveout_heap_free,
	.phys = ion_carveout_heap_phys,
	.map_user = ion_carveout_heap_map_user,
	.map_kernel = ion_carveout_heap_map_kernel,
	.unmap_kernel = ion_carveout_heap_unmap_kernel,
};

struct ion_heap *ion_carveout_heap_create(struct ion_platform_heap *heap_data)
{
	struct ion_carveout_heap *carveout_heap;

	printk(">>>>:ion_carveout_heap_create\n");
	carveout_heap = kzalloc(sizeof(struct ion_carveout_heap), GFP_KERNEL);
	if (!carveout_heap)
		return ERR_PTR(-ENOMEM);

	carveout_heap->pool = gen_pool_create(12, -1);
	if (!carveout_heap->pool) {
		kfree(carveout_heap);
		return ERR_PTR(-ENOMEM);
	}
	carveout_heap->base = heap_data->base;
	gen_pool_add(carveout_heap->pool, carveout_heap->base, heap_data->size,
		     -1);
	carveout_heap->heap.ops = &carveout_heap_ops;
	carveout_heap->heap.type = ION_HEAP_TYPE_CARVEOUT;

	return &carveout_heap->heap;
}

void ion_carveout_heap_destroy(struct ion_heap *heap)
{
	struct ion_carveout_heap *carveout_heap =
	     container_of(heap, struct  ion_carveout_heap, heap);

	printk(">>>>:ion_carveout_heap_destroy\n");
	gen_pool_destroy(carveout_heap->pool);
	kfree(carveout_heap);
	carveout_heap = NULL;
}
