/**
 * @file pci.h
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
#ifndef ARC_ARCH_PCI_PCI_H
#define ARC_ARCH_PCI_PCI_H

#include <stdint.h>
#include <stddef.h>
#include <global.h>

#define ARC_BAR_IS_IOSPACE(__bar) ((__bar & 1) == 1)
#define ARC_IOBAR_ADDR(__bar) ((__bar >> 2) & 0x3FFFFFFF)
#define ARC_BAR_IS_MEMSPACE(__bar) ((__bar & 1) == 0)
#define ARC_MEMBAR_TYPE(__bar) ((__bar >> 1) & 0b11)
#define ARC_MEMBAR_PREFETCHABLE(__bar) ((__bar >> 3) & 1)
#define ARC_MEMBAR_ADDR(__bar) ((__bar >> 4) & 0xFFFFFFF)

struct ARC_PCIHdrCommon {
	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t command;
	uint16_t status;
	uint8_t revision;
	uint8_t prog_if;
	uint8_t subclass;
	uint8_t class;
	uint8_t cahce_line_size;
	uint8_t latency;
	uint8_t header_type;
	uint8_t bist;
}__attribute__((packed));
STATIC_ASSERT(sizeof(struct ARC_PCIHdrCommon) == 0x10, "PCI Common Header wrong length");


struct ARC_PCIHdr0 {
	uint32_t bar0;
	uint32_t bar1;
	uint32_t bar2;
	uint32_t bar3;
	uint32_t bar4;
	uint32_t bar5;
	uint32_t cis_ptr;
	uint16_t subsystem_vendor;
	uint16_t subsystem_id;
	uint32_t rom_base;
	uint8_t capabilities_ptr;
	uint32_t resv0 : 24;
	uint32_t resv1;
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint8_t mint_grant;
	uint8_t max_latency;
}__attribute__((packed));
STATIC_ASSERT(sizeof(struct ARC_PCIHdr0) == 0x30, "PCI Header 0 wrong length");

struct ARC_PCIHdr1 {
	uint32_t bar0;
	uint32_t bar1;
	uint8_t primary_bus;
	uint8_t secondary_bus;
	uint8_t subordinate_bus;
	uint8_t secondary_latency_timer;
	uint8_t io_base;
	uint8_t io_limit;
	uint16_t secondary_stat;
	uint16_t mem_base;
	uint16_t mem_limit;
	uint16_t prefetch_mem_base;
	uint16_t prefetch_mem_limit;
	uint32_t prefetch_base_upper;
	uint32_t prefetch_limit_upper;
	uint16_t io_base_upper;
	uint16_t io_limit_upper;
	uint8_t capability_ptr;
	uint32_t resv0 : 24;
	uint32_t rom_base;
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint16_t bridge_ctrl;
}__attribute__((packed));
STATIC_ASSERT(sizeof(struct ARC_PCIHdr1) == 0x30, "PCI Header 1 wrong length");

struct ARC_PCIHdr2 {
	uint32_t carbus_exca_base;
	uint8_t caps_list_off;
	uint8_t resv0;
	uint16_t secondary_stat;
	uint8_t pci_bus;
	uint8_t cardbus;
	uint8_t subordinate_bus;
	uint8_t cardbus_latency_timer;
	uint32_t mem0_base;
	uint32_t mem0_limit;
	uint32_t mem1_base;
	uint32_t mem1_limit;
	uint32_t io0_base;
	uint32_t io0_limit;
	uint32_t io1_base;
	uint32_t io1_limit;
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint16_t bridge_ctrl;
	uint16_t subsys_device;
	uint16_t subsys_vendor;
	uint32_t legacy_mode_base;
}__attribute__((packed));
STATIC_ASSERT(sizeof(struct ARC_PCIHdr2) == 0x38, "PCI Header 2 wrong length");

struct ARC_PCIHeader {
	struct {
		uint32_t call; // 15:0 - Bus segment
			       // 23:16 - Bus
			       // 31:24 - Device
	} info;
	struct ARC_PCIHdrCommon common;
	union {
		struct ARC_PCIHdr0 header0;
		struct ARC_PCIHdr1 header1;
		struct ARC_PCIHdr2 header2;
	} headers;
}__attribute__((packed));

int pci_write(uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, size_t offset, uint8_t byte_width, uint32_t value);
uint32_t pci_read(uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, size_t offset);

struct ARC_PCIHeader *pci_read_header(uint16_t segment, uint8_t bus, uint8_t device);

uint16_t pci_get_status(uint16_t segment, uint8_t bus, uint8_t device);
int pci_set_command(uint16_t segment, uint8_t bus, uint8_t device, uint16_t command);

int init_pci();

#endif
