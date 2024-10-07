/**
 * @file smp.c
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
 * Definitions of functions and variables declared in arch/smp.h.
*/
#include <arch/smp.h>
#include <stddef.h>

struct ARC_ProcessorDescriptor Arc_ProcessorList[256] = { 0 };
uint32_t Arc_ProcessorCounter = 0;
struct ARC_ProcessorDescriptor *Arc_BootProcessor = NULL;

uint32_t get_processor_id() {
#ifdef ARC_TARGET_ARCH_X86_64
	return lapic_get_id();
#endif
}
