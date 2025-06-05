/**
 * @file acpi.h
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
#ifndef ARC_ARCH_ACPI_ACPI_H
#define ARC_ARCH_ACPI_ACPI_H

#include <stdint.h>
#include <stddef.h>

struct ARC_ACPIDevIO {
        struct ARC_ACPIDevIO *next;
        uint32_t base;
        uint32_t length; // Bytes
        uint32_t align;
        uint32_t decode_type;
};

struct ARC_ACPIDevIRQ{
        struct ARC_ACPIDevIRQ *next;
        uint8_t *irq_list;
        uint32_t irq_count;
        uint8_t polarity;
        uint8_t sharing;
        uint8_t triggering;
        uint8_t wake_capability;
        uint8_t length_kind;
};

struct ARC_ACPIDevInfo {
        struct ARC_ACPIDevIO *io;
        struct ARC_ACPIDevIRQ *irq;
};

int acpi_checksum(void *data, size_t length);
size_t acpi_get_table(const char *id, uint8_t **out);
int init_acpi();

#endif
