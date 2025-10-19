/**
 * @file pager.h
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
 * Abstract virtual memory, or paging, on different architectures.
*/
#ifndef ARC_ARCH_PAGER_H
#define ARC_ARCH_PAGER_H

#include <stdint.h>
#include <stddef.h>

// TODO: Make this abstract. In its current state, it is specific to x86-64

enum {
        // Page attributes
        // +0 PWT, +1 PCD, +2 PAT
        ARC_PAGER_PAT,
        // 1: Allow user access
        ARC_PAGER_US = 3,
        // 1: Overwrite existing paging structures
        ARC_PAGER_OVW,
        // 1: Mark pages as non-executable
        ARC_PAGER_NX,
        // 1: Only use 4K pages
        ARC_PAGER_4K,
        // 1: Mark pages as R/W
        ARC_PAGER_RW,
        // Reserved for internal use
        ARC_PAGER_RESV0,
        ARC_PAGER_RESV1,
        ARC_PAGER_RESV2,
        // 4-bit number corresponding to what (level - 1) will serve
        // as the cut off point for setting the US and RW flags
        ARC_PAGER_AUTO_USRW_DISABLE,
        // Reserved for internal use
        ARC_PAGER_RESV3 = 15,
};

// Indices in the 0x277 MSR (page attributes)
enum {
        ARC_PAGER_PAT_WB,   // WB
        ARC_PAGER_PAT_UC,   // UC
        ARC_PAGER_PAT_UCM,  // UC-
        ARC_PAGER_WC,       // WC
        ARC_PAGER_WT,       // WT
        ARC_PAGER_WP,       // WP
};

extern uintptr_t Arc_KernelPageTables;

void *pager_create_page_tables();
int pager_delete_page_tables(void *page_tables);
int pager_map(void *page_tables, uintptr_t virtual, uintptr_t physical, size_t size, uint32_t attributes) ;
int pager_unmap(void *page_tables, uintptr_t virtual, size_t size, void **physical);
int pager_fly_map(void *page_tables, uintptr_t virtual, size_t size, uint32_t attributes);
int pager_fly_unmap(void *page_tables, uintptr_t virtual, size_t size);
int pager_set_attr(void *page_tables, uintptr_t virtual, size_t size, uint32_t attributes);
int pager_clone(void *page_tables, uintptr_t virt_src, uintptr_t virt_dest, size_t size, int source);
uintptr_t pager_switch_to_kpages();
int init_pager();

#endif
