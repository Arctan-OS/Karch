/**
 * @file thread.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan.
 *
 * Arctan is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @DESCRIPTION
*/
#include <arctan.h>
#include <mm/vmm.h>
#include <lib/convention/sysv.h>
#include <arch/x86-64/apic/lapic.h>
#include <mm/pmm.h>
#include <arch/thread.h>
#include <arch/pager.h>
#include <mm/allocator.h>
#include <global.h>
#include <lib/util.h>

struct ARC_Thread *thread_create(struct ARC_VMMMeta *allocator, void *page_tables, void *entry, size_t stack_size) {
	if (entry == NULL) {
		ARC_DEBUG(ERR, "Failed to create thread, improper parameters (%p %lu)\n", entry, stack_size);
		return NULL;
	}

	struct ARC_Thread *thread = (struct ARC_Thread *)alloc(sizeof(*thread));

	if (thread == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate thread\n");
		return NULL;
	}

	memset(thread, 0, sizeof(*thread));

	void *vstack = (void *)vmm_alloc(allocator, stack_size);
	void *pstack = pmm_alloc(stack_size);

	if (pstack == NULL || vstack == NULL ||
		pager_map(page_tables, (uintptr_t)vstack, ARC_HHDM_TO_PHYS(pstack), stack_size, (1 << ARC_PAGER_RW) | (1 << ARC_PAGER_NX) | (1 << ARC_PAGER_US)) != 0) {
			free(thread);
			ARC_DEBUG(ERR, "Failed to allocate memory for thread\n");
			return NULL;
	}
	

	init_static_spinlock(&thread->lock);
		
#ifdef ARC_TARGET_ARCH_X86_64
		thread->ctx.rip = (uintptr_t)entry;
		thread->ctx.cs = page_tables == (void *)ARC_PHYS_TO_HHDM(Arc_KernelPageTables) ? 0x8 : 0x23;
		thread->ctx.ss = page_tables == (void *)ARC_PHYS_TO_HHDM(Arc_KernelPageTables) ? 0x10 : 0x1b;
		thread->ctx.rbp = (uintptr_t)vstack + stack_size - 16;
		thread->ctx.rsp = thread->ctx.rbp;
		thread->ctx.r11 = (1 << 9) | (1 << 1) | (0b11 << 12);
		thread->ctx.rflags = (1 << 9) | (1 << 1) | (0b11 << 12);
#endif
		
	thread->state = ARC_THREAD_READY;
	thread->pstack = pstack;
	thread->vstack = vstack;
	thread->stack_size = stack_size;

	ARC_DEBUG(INFO, "Created thread (%p)\n", entry);

	return thread;
}

int thread_delete(struct ARC_Thread *thread) {
	if (thread == NULL) {
		ARC_DEBUG(ERR, "Failed to delete thread, given thread is NULL\n");
		return -1;
	}

	// TODO: Signal an exit to the thread?

	spinlock_lock(&thread->lock);

	free(thread);

	return 0;
}
