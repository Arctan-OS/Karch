/**
 * @file smp_generic.h
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2024 awewsomegamer
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
 * The generic definitions for symmetric multi-processing. This is not meant to be
 * directly included by external callers, but by SMP implementations like arch/x86-64.
 * This is meant to serve is an outline for the absolute minimum that a processor descriptor
 * structure must have.
*/
#ifndef ARC_ARCH_GENERIC_SMP_H
#define ARC_ARCH_GENERIC_SMP_H

#include <stdint.h>
#include <lib/atomics.h>

#ifdef ARC_TARGET_ARCH_X86_64
#include <arch/x86-64/context.h>
#endif

struct ARC_GenericProcessorDescriptor {
	uint32_t processor;
	uint32_t acpi_uid;
	uint32_t acpi_flags;
	uint32_t status;
	// Bit | Description
	// 0   | 1: Initialized
	// 1   | 1: Holding
	struct ARC_Registers registers;
	ARC_GenericMutex register_lock;
	uint32_t flags;
	// Bit | Description
	// 0   | 1: Signals external modification of register state, cleared once
	//          changes have been accepted
	// 1   | 1: Write all registers to the registers member of this structure
	// 2   | 1: Timer values have been changed, cleared once changes have
	//          been accepted
	struct ARC_ProcessorDescriptor *next;
	uint32_t timer_ticks;
	uint32_t timer_mode;
	ARC_GenericMutex timer_lock;
};

#endif
