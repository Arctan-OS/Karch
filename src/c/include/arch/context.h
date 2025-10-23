/**
 * @file context.h
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
*/
#ifndef ARC_ARCH_CONTEXT_H
#define ARC_ARCH_CONTEXT_H

#ifdef ARC_TARGET_ARCH_X86_64
#include "arch/x86-64/context.h"
#endif

// NOTE: The architecture specific context.h header file should define
//       structures for accessing general purpose registers as ARC_Registers,
//       a structure to access the stack frame of an interrupt handler as
//       ARC_InterruptFrame, and define a structure holding all relevant
//       context information that may change from process to process as
//       ARC_Context

// NOTE: The architecture specific context.h header file should define
//       macros to, in assembly, push all and pop all registers in the
//       order defined in ARC_Registers


enum {
        ARC_CONTEXT_FLAG_FLOATS,
        ARC_CONTEXT_FLAG_MAX
};

void context_set_tcb(ARC_Context *ctx, void *tcb);
void *context_get_tcb(ARC_Context *ctx);

int uninit_context(ARC_Context *context);
ARC_Context *init_context(uint64_t flags);

#endif
