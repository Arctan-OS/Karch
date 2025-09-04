/**
 * @file table.c
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
#include "arch/acpi/table.h"
#include "global.h"
#include "uacpi/event.h"
#include "uacpi/tables.h"

size_t acpi_get_table(const char *id, void **out) {
        uacpi_table table = { 0 };
        int r = 0;

        if ((r = uacpi_table_find_by_signature(id, &table)) != UACPI_STATUS_OK) {
                ARC_DEBUG(ERR, "Failed to get table (%d)\n", r);
                return 0;
        }

        *out = table.ptr + 44;

        return table.hdr->length - 44;
}

void *acpi_get_next_madt_entry(int type, ARC_MADTIterator *it) {
        if (type < 0 || type >= ARC_MADT_ENTRY_TYPE_MAX || it == NULL) {
                return NULL;
        }

        ARC_MADTEntry *base = NULL;
        size_t max = acpi_get_table("APIC", (void **)&base);

        if (max == 0) {
                return NULL;
        }

        size_t i = 0;
        ARC_MADTEntry *entry = *it;

        if (*it != NULL) {
                i = ((uintptr_t)*it - (uintptr_t)base) + entry->length;
        }

        if (i >= max) {
                *it = NULL;
                return NULL;
        }

        for (; i < max; i += entry->length) {
                entry = (void *)base + i;

                if (entry->type == type) {
                        *it = entry;
                        return (void *)&entry->d;
                }
        }

        *it = NULL;
        return NULL;
}

int acpi_get_next_mcfg_entry(ARC_MCFGIterator *it) {
        if (it == NULL) {
                return -1;
        }

        ARC_MADTEntry *base = NULL;
        size_t max = acpi_get_table("MCFG", (void **)&base);

        if (max == 0) {
                return -2;
        }

        size_t i = 0;
        ARC_MCFGEntry *entry = *it;

        if (*it != NULL) {
                i = ((uintptr_t)*it - (uintptr_t)base) + sizeof(*entry);
        }

        if (i >= max) {
                *it = NULL;
                return -3;
        }

        *it = (ARC_MCFGEntry *)((uintptr_t)base + i);
        return 0;
}
