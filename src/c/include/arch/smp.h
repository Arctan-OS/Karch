/**
 * @file smp.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/Karch - Abstract Definition, Declaration of Architecture Functions
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan-OS/Karch.
 *
 * Arctan-OS/Karch is free software; you can redistribute it and/or
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
 * Abstract the initialization and management of symmetric multi-processing on different
 * architectures.
*/
#ifndef ARC_ARCH_SMP_H
#define ARC_ARCH_SMP_H

#include "arch/context.h"

#include <stdint.h>

// Offsets into flags attribute
enum {
        ARC_SMP_FLAGS_RESV0  = 0,
        ARC_SMP_FLAGS_RESV1  = 1,
        ARC_SMP_FLAGS_WTIMER = 2,
        ARC_SMP_FLAGS_HOLD   = 30,
        ARC_SMP_FLAGS_INIT   = 31,
};

typedef struct ARC_ProcessorDescriptor {
	struct ARC_Thread *current_thread;
	struct ARC_ProcessEntry *current_process;
	void *scheduler_meta;

	uint32_t acpi_uid;
	uint32_t acpi_flags;
	uint32_t flags;
	// Bit | Description
	// 1:0 | Reserved
	// 2   | 1: Timer values have been changed, cleared once changes have
	//          been accepted
	// 30  | 1: Holding
	// 31  | 1: Initialized

	uint32_t timer_ticks;
	uint32_t timer_mode;
} ARC_ProcessorDescriptor;

extern uint32_t Arc_ProcessorCounter;

/**
 * Hold the invoking processor.
 * */
void smp_hold();

ARC_ProcessorDescriptor *smp_get_proc_desc();

uint32_t smp_get_processor_id();

void smp_switch_to(ARC_Context *ctx);

int init_smp();

#endif
