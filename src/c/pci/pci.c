/**
 * @file pci.c
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
#include <arch/acpi/acpi.h>
#include <arch/pci/pci.h>
#include <lib/resource.h>
#include <stdint.h>
#include <global.h>
#include <mm/allocator.h>

#define PCI_GET_FUNCTION_BASE(segment, bus, device, function)

struct mcfg_entry {
	uint64_t base;
	uint16_t seg_group;
	uint8_t start_bus;
	uint8_t end_bus;
	uint32_t resv0;
}__attribute__((packed));
static struct mcfg_entry *mcfg_space = NULL;
static int mcfg_count = 0;

int pci_write(uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, size_t offset, uint8_t byte_width, uint32_t value) {
	if (mcfg_count > 0) {
		// Use mcfg space
		if (segment > mcfg_count) {
			ARC_DEBUG(ERR, "Invalid segment %d\n", segment);
			return -1;
		}

		void *base = (uint32_t *)ARC_PHYS_TO_HHDM(mcfg_space[segment].base + ((bus << 20) | (device << 15) | (function << 12)) + offset);

		switch (byte_width) {
			case 1: {
				*(uint8_t *)base = (uint8_t)value;
				break;
			}
			case 2: {
				*(uint16_t *)base = (uint16_t)value;
				break;
			}
			case 4: {
				*(uint32_t *)base = value;
				break;
			}
		}
	}

	uint32_t addr = (1 << 31) | (bus << 16) | ((device & 0b11111) << 11) | ((function & 0b111) << 8) | (offset & 0xFC);
	outd(0xCF8, addr);

	switch (byte_width) {
		case 1: {
			outb(0xCFC + offset & 0b11, (value & 0xFF));
			break;
		}
		case 2: {
			outw(0xCFC + offset & 0b10, (value & 0xFFFF));
			break;
		}
		case 4: {
			outd(0xCFC, value);
			break;
		}
	}

	return 0;
}

uint32_t pci_read(uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, size_t offset) {
	if (mcfg_count > 0) {
		// Use mcfg space
		if (segment > mcfg_count) {
			ARC_DEBUG(ERR, "Invalid segment %d\n", segment);
			return -1;
		}

		uint32_t *base = (uint32_t *)ARC_PHYS_TO_HHDM(mcfg_space[segment].base + ((bus << 20) | (device << 15) | (function << 12)) + offset);

		return *base;
	}

	uint32_t addr = (1 << 31) | (bus << 16) | ((device & 0b11111) << 11) | ((function & 0b111) << 8) | (offset & 0xFC);
	outd(0xCF8, addr);

	return ind(0xCFC);
}

uint16_t pci_get_status(uint16_t segment, uint8_t bus, uint8_t device) {
	return pci_read(segment, bus, device, 4, 0);
}

int pci_set_command(uint16_t segment, uint8_t bus, uint8_t device, uint16_t command) {
	return pci_write(segment, bus, device, 6, 0, 2, command);
}

static inline uint32_t pci_get_vendor_device(uint16_t segment, uint8_t bus, uint8_t device) {
	return pci_read(segment, bus, device, 0, 0);
}

static inline uint8_t pci_get_header_type(uint16_t segment, uint8_t bus, uint8_t device) {
	return (pci_read(segment, bus, device, 0, 0xC) >> 16) & 0xFF;
}

static int pci_read_common_header(uint16_t segment, uint8_t bus, uint8_t device, struct ARC_PCIHdrCommon *common) {
	uint32_t r0 = pci_get_vendor_device(segment, bus, device);
	uint32_t r2 = pci_read(segment, bus, device, 0, 0x8);
	uint32_t r3 = pci_read(segment, bus, device, 0, 0xC);

	common->vendor = r0 & 0xFFFF;
	common->device = (r0 >> 16) & 0xFFFF;
	common->status = pci_get_status(segment, bus, device);
	common->revision = r2 & 0xFF;
	common->prog_if = (r2 >> 8) & 0xFF;
	common->subclass = (r2 >> 16) & 0xFF;
	common->class = (r2 >> 24) & 0xFF;
	common->cahce_line_size = r3 & 0xFF;
	common->latency = (r3 >> 8) & 0xFF;
	common->header_type = (r3 >> 16) & 0xFF;
	common->bist = (r3 >> 24) & 0xFF;

	return 0;
}

static int pci_read_header_type0(uint16_t segment, uint8_t bus, uint8_t device, struct ARC_PCIHdr0 *header) {
	pci_read_common_header(segment, bus, device, &header->common);

	header->bar0 = pci_read(segment, bus, device, 0, 0x10);
	header->bar1 = pci_read(segment, bus, device, 0, 0x14);
	header->bar2 = pci_read(segment, bus, device, 0, 0x18);
	header->bar3 = pci_read(segment, bus, device, 0, 0x1C);
	header->bar4 = pci_read(segment, bus, device, 0, 0x20);
	header->bar5 = pci_read(segment, bus, device, 0, 0x24);
	header->cis_ptr = pci_read(segment, bus, device, 0, 0x28);

	uint32_t r11 = pci_read(segment, bus, device, 0, 0x2C);

	header->subsystem_vendor = r11 & 0xFFFF;
	header->subsystem_id = (r11 >> 16) & 0xFFFF;

	header->rom_base = pci_read(segment, bus, device, 0, 0x30);

	header->capabilities_ptr = pci_read(segment, bus, device, 0, 0x34) & 0xFF;

	uint32_t r15 = pci_read(segment, bus, device, 0, 0x3C);

	header->interrupt_line = r15 & 0xFF;
	header->interrupt_pin = (r15 >> 8) & 0xFF;
	header->mint_grant = (r15 >> 16) & 0xFF;
	header->max_latency = (r15 >> 24) & 0xFF;

	return 0;
}

static int pci_read_header_type1(uint16_t segment, uint8_t bus, uint8_t device, struct ARC_PCIHdr1 *header) {
	pci_read_common_header(segment, bus, device, &header->common);

	header->bar0 = pci_read(segment, bus, device, 0, 0x10);
	header->bar1 = pci_read(segment, bus, device, 0, 0x14);

	uint32_t r6 = pci_read(segment, bus, device, 0, 0x18);

	header->primary_bus = r6 & 0xFF;
	header->secondary_bus = (r6 >> 8) & 0xFF;
	header->subordinate_bus = (r6 >> 16) & 0xFF;
	header->secondary_latency_timer = (r6 >> 24) & 0xFF;

	uint32_t r7 = pci_read(segment, bus, device, 0, 0x1C);

	header->io_base = r7 & 0xFF;
	header->io_limit = (r7 >> 8) & 0xFF;
	header->secondary_stat = (r7 >> 16) & 0xFFFF;

	uint32_t r8 = pci_read(segment, bus, device, 0, 0x20);

	header->mem_base = r8 & 0xFFFF;
	header->mem_limit = (r8 >> 16) & 0xFFFF;

	uint32_t r9 = pci_read(segment, bus, device, 0, 0x24);

	header->prefetch_mem_base = r9 & 0xFFFF;
	header->prefetch_mem_limit = (r9 >> 16) & 0xFFFF;

	header->prefetch_base_upper = pci_read(segment, bus, device, 0, 0x28);
	header->prefetch_limit_upper = pci_read(segment, bus, device, 0, 0x2C);

	uint32_t r12 = pci_read(segment, bus, device, 0, 0x30);

	header->io_base_upper = r12 & 0xFFFF;
	header->io_limit_upper = (r12 >> 16) & 0xFFFF;

	header->capability_ptr = pci_read(segment, bus, device, 0, 0x34) & 0xFF;

	header->rom_base = pci_read(segment, bus, device, 0, 0x38);

	uint32_t r15 = pci_read(segment, bus, device, 0, 0x3C);

	header->interrupt_line = r15 & 0xFF;
	header->interrupt_pin = (r15 >> 8) & 0xFF;
	header->bridge_ctrl = (r15 >> 16) & 0xFFFF;

	return 0;
}

static int pci_read_header_type2(uint16_t segment, uint8_t bus, uint8_t device, struct ARC_PCIHdr2 *header) {
	pci_read_common_header(segment, bus, device, &header->common);

	header->carbus_exca_base = pci_read(segment, bus, device, 0, 0x10);

	uint32_t r5 = pci_read(segment, bus, device, 0, 0x14);

	header->caps_list_off = r5 & 0xFF;
	header->secondary_stat = (r5 >> 16) & 0xFFFF;

	uint32_t r6 = pci_read(segment, bus, device, 0, 0x18);

	header->pci_bus = r6 & 0xFF;
	header->cardbus = (r6 >> 8) & 0xFF;
	header->subordinate_bus = (r6 >> 16) & 0xFF;
	header->cardbus_latency_timer = (r6 >> 24) & 0xFF;

	header->mem0_base = pci_read(segment, bus, device, 0, 0x1C);
	header->mem0_limit = pci_read(segment, bus, device, 0, 0x20);
	header->mem1_base = pci_read(segment, bus, device, 0, 0x24);
	header->mem1_limit = pci_read(segment, bus, device, 0, 0x28);
	header->io0_base = pci_read(segment, bus, device, 0, 0x2C);
	header->io0_limit = pci_read(segment, bus, device, 0, 0x30);
	header->io1_base = pci_read(segment, bus, device, 0, 0x34);
	header->io1_limit = pci_read(segment, bus, device, 0, 0x38);

	uint32_t r15 = pci_read(segment, bus, device, 0, 0x3C);

	header->interrupt_line = r15 & 0xFF;
	header->interrupt_pin = (r15 >> 8) & 0xFF;
	header->bridge_ctrl = (r15 >> 16) & 0xFFFF;

	uint32_t r16 = pci_read(segment, bus, device, 0, 0x40);

	header->subsys_device = r16 & 0xFFFF;
	header->subsys_vendor = (r16 >> 16) & 0xFFFF;

	header->legacy_mode_base = pci_read(segment, bus, device, 0, 0x44);

	return 0;
}

static int pci_enumerate() {
	// TODO: Make this account for bridges

	for (int i = 0; i < 256; i++) {
		uint32_t vendor_device = pci_get_vendor_device(0, 0, i);

		if (vendor_device == 0xFFFFFFFF) {
			continue;
		}

		void *args = NULL;

		switch (pci_get_header_type(0, 0, i)) {
			case 0: {
				args = alloc(sizeof(struct ARC_PCIHdr0));
				pci_read_header_type0(0, 0, i, args);

				break;
			}
			case 1: {
				args = alloc(sizeof(struct ARC_PCIHdr1));
				pci_read_header_type1(0, 0, i, args);

				break;
			}
			case 2: {
				args = alloc(sizeof(struct ARC_PCIHdr2));
				pci_read_header_type2(0, 0, i, args);

				break;
			}
		}

		init_pci_resource_at("/dev/", vendor_device & 0xFFFF, (vendor_device >> 16) & 0xFFFF, args);
	}

	return 0;
}

static int setup_mcfg() {
	size_t size = acpi_get_table("MCFG", (uint8_t **)&mcfg_space);

	if (size == 0) {
		return -1;
	}

	mcfg_count = size / sizeof(*mcfg_space);

	for (int i = 0; i < mcfg_count; i++) {
		ARC_DEBUG(INFO, "Configuration Space %d: Base: 0x%"PRIx64" Group: %d Buses: [%d %d]\n", i, mcfg_space[i].base, mcfg_space[i].seg_group, mcfg_space[i].start_bus, mcfg_space[i].end_bus);
	}

	return 0;
}

int setup_io() {
	ARC_DEBUG(ERR, "Definitely setting up the IO stuff\n");
	return 0;
}

int init_pci() {
	ARC_DEBUG(INFO, "Initializing PCI\n");

	int r = setup_mcfg();

	if (r != 0) {
		ARC_DEBUG(INFO, "Cannot setup memory mapped PCI access, trying to setup using I/O ports\n");
	}

	r = pci_enumerate();

	if (r != 0) {
		ARC_DEBUG(ERR, "Failed to initialize PCI\n");
		return -1;
	}

	ARC_DEBUG(INFO, "Initialized PCI\n");

	return 0;
}
