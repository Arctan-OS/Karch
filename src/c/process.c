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

/*
**
// NOTE: On x86-64 systems this is only called during an interrupt.
//       Interrupts are disabled on syscalls, therefore the worry of
//       returning to different segments is non-existent.
int process_switch(struct ARC_Process *to) {
	if (to == NULL) {
		ARC_DEBUG(ERR, "Cannot switch to NULL process\n");
		return -1;
	}

	struct ARC_ProcessorDescriptor *desc = smp_get_proc_desc();
	struct ARC_ProcessorDescriptor *current_desc = desc->generic.next;
	struct ARC_Thread *thread = to->nextex;
	struct ARC_Thread *current_thread = thread->next;

	for (uint32_t i = 0; (i < Arc_ProcessorCounter - 1 && thread != NULL); i++) {
		if (current_thread == NULL) {
			current_thread = to->threads;
		}

		smp_context_write(current_desc, &current_thread->ctx);
		current_thread = current_thread->next;
		current_desc = current_desc->generic.next;
	}

	if (thread != NULL) {
		smp_context_write(desc, &thread->ctx);
	}

	return 0;
}
 */
