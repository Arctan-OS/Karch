/**
 * @file acpi.h
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
*/
#ifndef ARC_ARCH_ACPI_ACPI_H
#define ARC_ARCH_ACPI_ACPI_H

#include <stdint.h>
#include <stddef.h>

#define ARC_ACPI_TBLSIG_APIC 0x43495041 // "APIC"
#define ARC_ACPI_TBLSIG_RSDT 0x54445352 // "RSDT"
#define ARC_ACPI_TBLSIG_HPET 0x54455048 // "HPET"
#define ARC_ACPI_TBLSIG_DSDT 0x54445344 // "DSDT"
#define ARC_ACPI_TBLSIG_SSDT 0x54445353 // "SSDT"
#define ARC_ACPI_TBLSIG_PSDT 0x54445350 // "PSDT"
#define ARC_ACPI_TBLSIG_FACP 0x50434146 // "FACP"
#define ARC_ACPI_TBLSIG_MCFG 0x4746434D // "MCFG"
#define ARC_ACPI_TBLSIG_WAET 0x54454157 // "WAET"
#define ARC_ACPI_TBLSIG_XSDT 0x54445358 // "XSDT"

struct ARC_RSDTBaseEntry {
	uint32_t signature;
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	uint8_t OEMID[6];
	uint8_t OEMTID[8];
	uint32_t OEMREV;
	char creator_id[4];
	uint32_t creator_rev;
}__attribute__((packed));

int acpi_checksum(void *data, size_t length);

size_t acpi_get_madt(uint8_t **out);

int init_acpi();

#endif
