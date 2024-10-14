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
#include <stdint.h>
#include <global.h>

#define PCI_GET_FUNCTION_BASE(segment, bus, device, function)

struct mcfg_entry {
	uint64_t base;
	uint16_t seg_group;
	uint8_t start_bus;
	uint8_t end_bus;
	uint32_t resv0;
};
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

	// TODO: Use registers

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

	// TODO: Use registers

	return 0;
}

static inline uint32_t pci_get_vendor_device(uint16_t segment, uint8_t bus, uint8_t device) {
	return pci_read(segment, bus, device, 0, 0);
}

static inline uint16_t pci_get_status(uint16_t segment, uint8_t bus, uint8_t device) {
	return pci_read(segment, bus, device, 4, 0);
}

static inline int pci_set_command(uint16_t segment, uint8_t bus, uint8_t device, uint16_t command) {
	return pci_write(segment, bus, device, 6, 0, 2, command);
}

// Register 0x2
static inline uint8_t pci_get_revision_id(uint16_t segment, uint8_t bus, uint8_t device) {
	return pci_read(segment, bus, device, 0, 8) & 0xFF;
}

static inline uint8_t pci_get_prog_if(uint16_t segment, uint8_t bus, uint8_t device) {
	return (pci_read(segment, bus, device, 0, 8) >> 8) & 0xFF;
}

static inline uint8_t pci_get_subclass(uint16_t segment, uint8_t bus, uint8_t device) {
	return (pci_read(segment, bus, device, 0, 8) >> 16) & 0xFF;
}

static inline uint8_t pci_get_class(uint16_t segment, uint8_t bus, uint8_t device) {
	return (pci_read(segment, bus, device, 0, 8) >> 24) & 0xFF;
}
// Register 0x2

// Register 0x3
static inline uint8_t pci_get_cache_line_size(uint16_t segment, uint8_t bus, uint8_t device) {
	return pci_read(segment, bus, device, 0, 0xC) & 0xFF;
}

static inline uint8_t pci_get_latency(uint16_t segment, uint8_t bus, uint8_t device) {
	return (pci_read(segment, bus, device, 0, 0xC) >> 8) & 0xFF;
}

static inline uint8_t pci_get_header_type(uint16_t segment, uint8_t bus, uint8_t device) {
	return (pci_read(segment, bus, device, 0, 0xC) >> 16) & 0xFF;
}

static inline uint8_t pci_get_bist(uint16_t segment, uint8_t bus, uint8_t device) {
	return (pci_read(segment, bus, device, 0, 0xC) >> 24) & 0xFF;
}
// Register 0x3

static int pci_enumerate() {
	for (int i = 0; i < 256; i++) {
		uint32_t vendor_device = pci_get_vendor_device(0, 0, i);
		if ((vendor_device & 0xFFFF) == 0xFFFF) {
			continue;
		}

		printf("Device #%d (%04x:%04x):\n", i, vendor_device & 0xFFFF, vendor_device >> 16);
		printf("\tStatus: 0x%x\n", pci_get_status(0, 0, i));
		printf("\tRevision: 0x%x\n", pci_get_revision_id(0, 0, i));
		printf("\tIF: 0x%x\n", pci_get_prog_if(0, 0, i));
		printf("\tSubclass: 0x%x\n", pci_get_subclass(0, 0, i));
		printf("\tClass: 0x%x\n", pci_get_class(0, 0, i));
		printf("\tCLS: 0x%x\n", pci_get_cache_line_size(0, 0, i));
		printf("\tLatency: 0x%x\n", pci_get_latency(0, 0, i));
		printf("\tHeader Type: 0x%x\n", pci_get_header_type(0, 0, i));
		printf("\tBIST: 0x%x\n", pci_get_bist(0, 0, i));
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

	r = pci_enumerate();

	if (r != 0) {
		ARC_DEBUG(ERR, "Failed to enumerate PCI\n");
		return -2;
	}

	ARC_DEBUG(INFO, "Initialized PCI\n");

	return 0;
}
