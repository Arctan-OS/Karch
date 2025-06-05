/**
 * @file smp.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan-OS/Kernel - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
 *
 * This file is part of Arctan-OS/Kernel.
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
 * Abstract the initialization and management of symmetric multi-processing on different
 * architectures.
*/
#ifndef ARC_ARCH_SMP_H
#define ARC_ARCH_SMP_H

// Offsets into flags attribute
#define ARC_SMP_FLAGS_RESV0 0
#define ARC_SMP_FLAGS_RESV1 1
#define ARC_SMP_FLAGS_WTIMER 2
#define ARC_SMP_FLAGS_HOLD 30
#define ARC_SMP_FLAGS_INIT 31

#include <stdint.h>
#include <lib/atomics.h>
#include <config.h>

#include <stdint.h>
#include <arch/context.h>
#include <lib/atomics.h>
#include <stdarg.h>
#include <userspace/thread.h>
#include <userspace/process.h>

#ifdef ARC_TARGET_ARCH_X86_64
#include <arch/x86-64/apic/lapic.h>
#endif

// TODO: This structure can be changed such that there is no need for syscall_stack
//       by making it so that the KernelGS pointer points to the stack, and on the stack
//       is this structure. This would also eliminate the need for a processor descriptor list.
struct ARC_ProcessorDescriptor {
	uintptr_t syscall_stack;
	struct ARC_ProcessorDescriptor *next;
	
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
}__attribute__((packed));

// NOTE: The index in Arc_ProcessorList corresponds to the ID
//       acquired from get_processor_id();
extern struct ARC_ProcessorDescriptor Arc_ProcessorList[ARC_MAX_PROCESSORS];
// NOTE: Since the processor structure includes a next pointer,
//       this allows us to also traverse the list by processor,
//       while still being able to address the list by processor ID
//       in O(1) time
extern struct ARC_ProcessorDescriptor *Arc_BootProcessor;
extern uint32_t Arc_ProcessorCounter;

/**
 * Hold the invoking processor.
 * */
void smp_hold();

int smp_switch_to_userspace();

struct ARC_ProcessorDescriptor *smp_get_proc_desc();

uint32_t smp_get_processor_id();

int smp_set_tcb(void *tcb);

/**
 * Initialize an AP into an SMP system.
 *
 * Initializes the given AP into an SMP system by creating the
 * relevant processor descriptors and sending INIT and START IPIs.
 *
 * NOTE: This functions is meant to only be called from the BSP.
 * NOTE: This function should be called to initialize the BSP as well
 *       since it needs to be inserted into the descriptor list.
 * @param uint32_t lapic - The ID of the LAPIC to initialize.
 * @param uint32_t acpi_uid - The UID given to the processor by ACPI.
 * @param uint32_t acpi_flags - The flags given by ACPI.
 * @param uint32_t version - The version of the LAPIC.
 * @return zero upon success.
 * */
int init_smp(uint32_t lapic, uint32_t acpi_uid, uint32_t acpi_flags, uint32_t version);

#endif
