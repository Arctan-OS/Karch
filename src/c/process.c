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

struct ARC_Process *process_create(char *filepath) {
	if (filepath == NULL) {
		ARC_DEBUG(ERR, "Failed to create process, no file given\n");
		return NULL;
	}

	struct ARC_File *file = NULL;

	if (vfs_open(filepath, 0, ARC_STD_PERM, &file) != 0) {
		ARC_DEBUG(ERR, "Failed to create process, failed to open file\n");
		return NULL;
	}

	struct ARC_Process *process = (struct ARC_Process *)alloc(sizeof(*process));
	if (process == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate process\n");
		return  NULL;
	}

	memset(process, 0, sizeof(*process));

	void *page_tables = pmm_alloc(); // Allocate new top level page table
	void *entry = load_elf(page_tables, file);

	struct ARC_Thread *main = thread_create(page_tables, entry, DEFAULT_MEMSIZE);

	if (main == NULL) {
		free(process);
		ARC_DEBUG(ERR, "Failed to create main thread\n");
		return NULL;
	}

	pager_clone(page_tables, (uintptr_t)&__KERNEL_START__, (uintptr_t)&__KERNEL_START__,
		    ((uintptr_t)&__KERNEL_END__ - (uintptr_t)&__KERNEL_START__));

	process->threads = main;
	process->nextex = main;
	process->page_tables = page_tables;

	init_static_spinlock(&process->thread_lock);

	ARC_DEBUG(INFO, "Created process\n");

	return process;
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

struct ARC_Thread *process_get_next_thread(struct ARC_Process *process, struct ARC_Thread *current) {
	if (process == NULL) {
		ARC_DEBUG(ERR, "Cannot get next thread of NULL process\n");
		return NULL;
	}

	struct ARC_Thread *ret = NULL;
	spinlock_lock(&process->thread_lock);
	ret = process->nextex;

	if (ret == NULL || ret == current) {
		spinlock_unlock(&process->thread_lock);
		return NULL;
	}

	process->nextex = ret->next;
	spinlock_unlock(&process->thread_lock);

	return ret;
}
