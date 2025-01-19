/**
 * @file thread.h
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
#ifndef ARC_ARCH_THREAD_H
#define ARC_ARCH_THREAD_H

#ifdef ARC_TARGET_ARCH_X86_64
#include <arch/x86-64/context.h>
#endif

#include <stdint.h>
#include <stddef.h>
#include <lib/atomics.h>

struct ARC_Thread {
	void *mem; // Heap begins at mem, stack begins at mem + mem_size - 8 bytes (on 64-bit machines)
	size_t mem_size;
	struct ARC_Thread *next;
	ARC_GenericSpinlock lock;
	struct ARC_Registers ctx;
};

struct ARC_Thread *thread_create(void *page_tables, void *entry, size_t mem_size);
int thread_delete(struct ARC_Thread *thread);

#endif
