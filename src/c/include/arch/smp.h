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

#include <stdint.h>
#include <lib/atomics.h>
#include <config.h>

#ifdef ARC_TARGET_ARCH_X86_64
#include <arch/x86-64/apic/lapic.h>
#include <arch/x86-64/smp.h>
#endif

// NOTE: The index in Arc_ProcessorList corresponds to the ID
//       acquired from get_processor_id();
extern struct ARC_ProcessorDescriptor Arc_ProcessorList[ARC_MAX_PROCESSORS];
// NOTE: Since the processor structure includes a next pointer,
//       this allows us to also traverse the list by processor,
//       while still being able to address the list by processor ID
//       in O(1) time
extern struct ARC_ProcessorDescriptor *Arc_BootProcessor;
extern uint32_t Arc_ProcessorCounter;

uint32_t get_processor_id();

#endif
