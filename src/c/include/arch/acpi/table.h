/**
 * @file table.h
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
#ifndef ARC_ARCH_ACPI_TABLE_H
#define ARC_ARCH_ACPI_TABLE_H

#include <stdint.h>

enum {
        ARC_MADT_ENTRY_TYPE_LAPIC               = 0x00,
        ARC_MADT_ENTRY_TYPE_IOAPIC              = 0x01,
        ARC_MADT_ENTRY_TYPE_INT_OVERRIDE_SRC    = 0x02,
        ARC_MADT_ENTRY_TYPE_NMI_SOURCE          = 0x03,
        ARC_MADT_ENTRY_TYPE_LAPIC_NMI           = 0x04,
        ARC_MADT_ENTRY_TYPE_LAPIC_ADDR_OVERRIDE = 0x05,
        ARC_MADT_ENTRY_TYPE_IOSAPIC             = 0x06,
        ARC_MADT_ENTRY_TYPE_LSAPIC              = 0x07,
        ARC_MADT_ENTRY_TYPE_PIS                 = 0x08,
        ARC_MADT_ENTRY_TYPE_Lx2APIC             = 0x09,
        ARC_MADT_ENTRY_TYPE_Lx2APIC_NMI         = 0x0A,
        ARC_MADT_ENTRY_TYPE_GICC                = 0x0B,
        ARC_MADT_ENTRY_TYPE_GICD                = 0x0C,
        ARC_MADT_ENTRY_TYPE_GIC_MSI             = 0x0D,
        ARC_MADT_ENTRY_TYPE_GICR                = 0x0E,
        ARC_MADT_ENTRY_TYPE_ITS                 = 0x0F,
        ARC_MADT_ENTRY_TYPE_MP_WAKEUP           = 0x10,
        ARC_MADT_ENTRY_TYPE_MAX,
};

typedef struct ARC_MADTLapic{
        uint8_t uid;
        uint8_t id;
        uint32_t flags;
}  __attribute__((packed)) ARC_MADTLapic;

typedef struct ARC_MADTIOApic {
        uint8_t id;
        uint8_t resv;
        uint32_t address;
        uint32_t gsi;
}  __attribute__((packed)) ARC_MADTIOApic;

typedef struct ARC_MADT_ISO {
        uint8_t bus;
        uint8_t source;
        uint32_t gsi;
        uint16_t flags;
}  __attribute__((packed)) ARC_MADT_ISO;

typedef struct ARC_MADT_NMI {
        uint8_t flags;
        uint32_t gsi;
} __attribute__((packed)) ARC_MADT_NMI;

typedef struct ARC_MADTEntry {
        uint8_t type;
        uint8_t length;
        union {
                ARC_MADTLapic lapic;
                ARC_MADTIOApic ioapic;
                ARC_MADT_ISO interrupt_source_override;
                ARC_MADT_NMI nmi;
        } d;
} __attribute__((packed)) ARC_MADTEntry;

typedef struct ARC_MCFGEntry {
	uint64_t base;
	uint16_t seg_group;
	uint8_t start_bus;
	uint8_t end_bus;
	uint32_t resv0;
} __attribute__((packed)) ARC_MCFGEntry;

typedef ARC_MADTEntry * ARC_MADTIterator;
typedef ARC_MCFGEntry * ARC_MCFGIterator;

void *acpi_get_next_madt_entry(int type, ARC_MADTIterator *it);
int acpi_get_next_mcfg_entry(ARC_MCFGIterator *it);

#endif
