/**
 * @file interrupt.h
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
*/
#ifndef ARC_ARCH_INTERRUPT_H
#define ARC_ARCH_INTERRUPT_H

#ifdef ARC_TARGET_ARCH_X86_64
#include "arch/x86-64/context.h"
#endif

#include <stdbool.h>
#include <stdint.h>

typedef enum {
        ARC_INTERRUPT_FLAGS_TRIGGER, // 1: Level, 0: Edge
        ARC_INTERRUPT_FLAGS_ACTIVE,  // 1: Low,   1: High
        ARC_INTERRUPT_FLAGS_GROUP,   // 1: Group, 0: Individual controller
} ARC_INTERRUPT_FLAGS;

// The architecture specific header must define a structure specifying
// the order of general purpose registers and a structure organizing the
// registers pushed upon invokation of an interrupt. The two structures are
// not mutually exclusive

int interrupt_set(void *handle, uint32_t number, void (*function)(ARC_InterruptFrame *), bool kernel);
int interrupt_map_gsi(uint32_t gsi, uint32_t to_irq, uint32_t to_id, uint8_t flags);
int interrupt_load(void *handle);

void *init_dynamic_interrupts(int count);
int init_early_irqs();
int init_early_exceptions();

#endif
