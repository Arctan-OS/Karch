/**
 * @file process.h
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
#ifndef ARC_ARCH_PROCESS_H
#define ARC_ARCH_PROCESS_H

#include <stdint.h>
#include <stddef.h>
#include <arch/thread.h>

struct ARC_Process {
	struct ARC_Thread *threads;
	struct ARC_Thread *nextex;
	void *page_tables;
	void *base;
	size_t size;
	int priority;
};

struct ARC_Process *process_create(char *filepath);
int process_fork(struct ARC_Process *process);
int process_delete(struct ARC_Process *process);
int process_switch(struct ARC_Process *to);

#endif
