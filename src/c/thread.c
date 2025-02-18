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
#include <arch/thread.h>
#include <arch/pager.h>
#include <mm/allocator.h>
#include <global.h>
#include <lib/util.h>

struct ARC_Thread *thread_create(void *page_tables, void *entry, size_t mem_size) {
	if (entry == NULL || mem_size == 0) {
		ARC_DEBUG(ERR, "Failed to create thread, improper parameters (%p %lu)\n", entry, mem_size);
		return NULL;
	}

	struct ARC_Thread *thread = (struct ARC_Thread *)alloc(sizeof(*thread));

	if (thread == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate thread\n");
		return NULL;
	}

	memset(thread, 0, sizeof(*thread));

	void *mem = (void *)0x1000; // TODO: Change this depending on thread

	pager_fly_map(page_tables, (uintptr_t)mem, mem_size, (1 << ARC_PAGER_RW) | (1 << ARC_PAGER_NX) | (1 << ARC_PAGER_US));

	if (mem == NULL) {
		free(thread);
		ARC_DEBUG(ERR, "Failed to allocate memory for thread\n");
		return NULL;
	}

	init_static_spinlock(&thread->lock);

#ifdef ARC_TARGET_ARCH_X86_64
	thread->ctx.rip = (uintptr_t)entry;
	thread->ctx.cs = 0x23;
	thread->ctx.ss = 0x1b;
	thread->ctx.rbp = (uintptr_t)mem + mem_size - 8;
	thread->ctx.rsp = thread->ctx.rbp;
	thread->ctx.r11 = 1 << 9 | 1 << 1;
	thread->ctx.rflags = 1 << 9 | 1 << 1;
#endif

	ARC_DEBUG(INFO, "Created thread\n");

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
