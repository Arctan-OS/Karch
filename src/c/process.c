/**
 * @file process.c
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
#include <arch/process.h>
#include <mm/allocator.h>
#include <global.h>
#include <lib/util.h>
#include <loaders/elf.h>
#include <lib/perms.h>
#include <arch/smp.h>
#include <arch/x86-64/ctrl_regs.h>
#include <mm/pmm.h>
#include <arch/pager.h>

#define DEFAULT_MEMSIZE 0x1000 * 64

struct ARC_Process *process_create_from_file(int userspace, char *filepath) {
	if (filepath == NULL) {
		ARC_DEBUG(ERR, "Failed to create process, no file given\n");
		return NULL;
	}

	struct ARC_File *file = NULL;

	if (vfs_open(filepath, 0, ARC_STD_PERM, &file) != 0) {
		ARC_DEBUG(ERR, "Failed to create process, failed to open file\n");
		return NULL;
	}

	struct ARC_Process *process = process_create(userspace, NULL);
	if (process == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate process\n");
		return  NULL;
	}

	void *entry = load_elf(process->page_tables, file);
	struct ARC_Thread *main = thread_create(process->page_tables, entry, DEFAULT_MEMSIZE);

	if (main == NULL) {
		process_delete(process);
		ARC_DEBUG(ERR, "Failed to create main thread\n");
		return NULL;
	}

	process_associate_thread(process, main);

	ARC_DEBUG(INFO, "Created process from file %s\n", filepath);

	return process;
}

struct ARC_Process *process_create(int userspace, void *page_tables) {
	struct ARC_Process *process = (struct ARC_Process *)alloc(sizeof(*process));

	if (process == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate process\n");
		return  NULL;
	}

	if (page_tables == NULL) {
		if (userspace == 0) {
			// Not a userspace process
			page_tables = (void *)ARC_PHYS_TO_HHDM(Arc_KernelPageTables);
			goto skip_page_tables;
		}

		page_tables = pager_create_page_tables();

		if (page_tables == NULL) {
			ARC_DEBUG(ERR, "Failed to allocate page tables\n");
			free(process);
			return NULL;
		}

		pager_clone(page_tables, (uintptr_t)&__KERNEL_START__, (uintptr_t)&__KERNEL_START__,
			    ((uintptr_t)&__KERNEL_END__ - (uintptr_t)&__KERNEL_START__));
	}

	skip_page_tables:;

	memset(process, 0, sizeof(*process));

	process->page_tables = page_tables;

	init_static_spinlock(&process->thread_lock);

	return process;
}

int process_associate_thread(struct ARC_Process *process, struct ARC_Thread *thread) {
	if (process == NULL || thread == NULL) {
		ARC_DEBUG(ERR, "Failed associate thread (%p) with process (%p)\n", thread, process);
		return -1;
	}

	spinlock_lock(&process->thread_lock);

	if (process->threads == NULL) {
		process->threads = thread;
		process->nextex = thread;
	} else {
		thread->next = process->threads;
		process->threads = thread;
	}

	spinlock_unlock(&process->thread_lock);

	return 0;
}

int process_disassociate_thread(struct ARC_Process *process, struct ARC_Thread *thread) {
	if (process == NULL || thread == NULL) {
		ARC_DEBUG(ERR, "Failed disassociate thread (%p) with process (%p)\n", thread, process);
		return -1;
	}

	// NOTE: This is terribly inefficient, have to come up with a better way to do this

	spinlock_lock(&process->thread_lock);
	struct ARC_Thread *current = process->threads;
	struct ARC_Thread *last = NULL;

	while (current != NULL) {
		if (current == thread) {
			break;
		}

		last = current;
		current = current->next;
	}

	if (current == NULL) {
		ARC_DEBUG(ERR, "Could not find thread (%p) in process (%p)\n", thread, process);
		spinlock_unlock(&process->thread_lock);
		return -1;
	}

	if (last == NULL) {
		process->threads = thread->next;
	} else {
		last->next = thread->next;
	}

	spinlock_unlock(&process->thread_lock);

	return 0;
}

int process_fork(struct ARC_Process *process) {
	if (process == NULL) {
		ARC_DEBUG(ERR, "Failed to fork process, given process is NULL\n");
		return -1;
	}

	ARC_DEBUG(ERR, "Implement\n");

	return 0;
}

int process_delete(struct ARC_Process *process) {
	if (process == NULL) {
		ARC_DEBUG(ERR, "Failed to delete process, given process is NULL\n");
		return -1;
	}

	ARC_DEBUG(ERR, "Implement\n");

	return 0;
}

struct ARC_Thread *process_get_next_thread(struct ARC_Process *process) {
	if (process == NULL) {
		ARC_DEBUG(ERR, "Cannot get next thread of NULL process\n");
		return NULL;
	}

	struct ARC_Thread *ret = NULL;
	struct ARC_ProcessorDescriptor *processor = smp_get_proc_desc();

	spinlock_lock(&process->thread_lock);
	ret = process->nextex;

	if (ret == NULL || ret == processor->current_thread) {
		spinlock_unlock(&process->thread_lock);
		return NULL;
	}

	process->nextex = ret->next;
	spinlock_unlock(&process->thread_lock);

	return ret;
}
