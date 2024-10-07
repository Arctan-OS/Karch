/**
 * @file pager.h
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
 * Abstract virtual memory, or paging, on different architectures.
*/
#ifndef ARC_ARCH_PAGER_H
#define ARC_ARCH_PAGER_H

// * = Specific to Arctan
// () = Default value
// Offset of page attributes
//  +0: PWT, +1: PCD, +2: PAT
// Page attributes (0)
#define ARC_PAGER_PAT     0
// 1: User accessible page (0)
#define ARC_PAGER_US      3
// 1: Overwrite if entry is already present * (0)
#define ARC_PAGER_OVW     4
// 1: Disable execution on entry (0)
#define ARC_PAGER_NX      5
// 1: Use only 4K pages (0)
#define ARC_PAGER_4K      6
// 1: Read and Write allowed (0)
#define ARC_PAGER_RW      7
// Reserved for internal use
#define ARC_PAGER_RESV0   8
#define ARC_PAGER_RESV1   9
#define ARC_PAGER_RESV2   10

// Indices in the 0x277 MSR (page attributes)
#define ARC_PAGER_PAT_WB  0 // WB
#define ARC_PAGER_PAT_UC  1 // UC
#define ARC_PAGER_PAT_UCM 2 // UC-
#define ARC_PAGER_PAT_WC  3 // WC
#define ARC_PAGER_PAT_WT  4 // WT
#define ARC_PAGER_PAT_WP  5 // WP

#ifdef ARC_TARGET_ARCH_X86_64
#include <arch/x86-64/pager.h>
#endif

#endif
