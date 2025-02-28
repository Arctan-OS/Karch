/**
 * @file smp.h
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
 * Abstract the initialization and management of symmetric multi-processing on different
 * architectures.
*/
#ifndef ARC_ARCH_SMP_H
#define ARC_ARCH_SMP_H

// Offsets into flags attribute
#define ARC_SMP_FLAGS_CTXWRITE 0
#define ARC_SMP_FLAGS_CTXSAVE 1
#define ARC_SMP_FLAGS_WTIMER 2
#define ARC_SMP_FLAGS_HOLD 30
#define ARC_SMP_FLAGS_INIT 31

#include <stdint.h>
#include <lib/atomics.h>
#include <config.h>

#include <stdint.h>
#include <arch/x86-64/context.h>
#include <lib/atomics.h>
#include <stdarg.h>
#include <arch/thread.h>
#include <arch/process.h>

#ifdef ARC_TARGET_ARCH_X86_64
#include <arch/x86-64/apic/lapic.h>
#endif

struct ARC_ProcessorDescriptor {
	uintptr_t syscall_stack;
	struct ARC_ProcessorDescriptor *next;
	struct ARC_Thread *last_thread;
	struct ARC_Thread *current_thread;
	struct ARC_ProcessEntry *current_process;
	uint32_t acpi_uid;
	uint32_t acpi_flags;
	uint32_t flags;
	// Bit | Description
	// 0   | 1: Signals external modification of register state, cleared once
	//          changes have been accepted
	// 1   | 1: Write all registers to the registers member of this structure
	// 2   | 1: Timer values have been changed, cleared once changes have
	//          been accepted
	// 30  | 1: Holding
	// 31  | 1: Initialized
	uint32_t timer_ticks;
	uint32_t timer_mode;
	ARC_GenericSpinlock timer_lock;
	ARC_GenericSpinlock register_lock;
	struct ARC_Registers registers;
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
void __attribute__((naked)) smp_hold();

/**
 * Write the given context to the given processor.
 *
 * NOTE: processor->register_lock is expected to be held.
 * */
int smp_context_write(struct ARC_ProcessorDescriptor *processor, struct ARC_Registers *regs);
/**
 * Write the processor's context to the given registers structure.
 *
 * NOTE: processor->register_lock is expected to be held.
 * */
int smp_context_save(struct ARC_ProcessorDescriptor *processor, struct ARC_Registers *regs);

/**
 * Tell the processor to execute the given function with the given arguments.
 *
 * NOTE: When a processor accepts the changes, it will write its current register
 *       (prior to accepting the changes) to processor->registers. The caller should
 *       save this if it wishes to return to this state.
 *
 * @param struct ARC_ProcessorDescriptor *processor - The processor to execute the jump on.
 * @param void *function - The function to jump to.
 * @param int argc - The number of arguments given to the function.
 * @param ... - The arguments to pass to the function.
 * @return zero on success.
 * */
int smp_jmp(struct ARC_ProcessorDescriptor *processor, void *function, int argc, ...);

int smp_switch_to_userspace();

struct ARC_ProcessorDescriptor *smp_get_proc_desc();

uint32_t smp_get_processor_id();

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
