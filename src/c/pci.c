/**
 * @file pci.c
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
#include "arch/acpi/acpi.h"
#include "arch/acpi/table.h"
#include "arch/io/port.h"
#include "arch/pci.h"
#include "drivers/resource.h"
#include "global.h"
#include "mm/allocator.h"

#define PCI_GET_FUNCTION_BASE(segment, bus, device, function)
#define PCI_IO_CFG_ADDRESS 0xCF8
#define PCI_IO_CFG_DATA    0xCFC

static ARC_MCFGEntry *mcfg_space = NULL;
static int mcfg_count = 0;

int pci_write(uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, size_t offset, uint8_t byte_width, uint32_t value) {
	if (mcfg_count > 0) {
		// Use mcfg space
		if (segment > mcfg_count) {
			ARC_DEBUG(ERR, "Invalid segment %d\n", segment);
			return -1;
		}

		size_t off = ((bus << 20) | (device << 15) | (function << 12)) + offset;
		void *base = (uint32_t *)ARC_PHYS_TO_HHDM(mcfg_space[segment].base + off);

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

	uint32_t addr = (1 << 31); // Enable
	addr |= bus << 16;
	addr |= (device & 0b11111) << 11;
	addr |= (function &0b111) << 8;
	addr |= (offset & 0xFC);
	outd(PCI_IO_CFG_ADDRESS, addr);

	switch (byte_width) {
		case 1: {
			outb(PCI_IO_CFG_DATA + (offset & 0b11), (value & 0xFF));
			break;
		}
		case 2: {
			outw(PCI_IO_CFG_DATA + (offset & 0b10), (value & 0xFFFF));
			break;
		}
		case 4: {
			outd(PCI_IO_CFG_DATA,  value);
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

		uintptr_t off = ((bus << 20) | (device << 15) | (function << 12)) + offset;
		uint32_t *base = (uint32_t *)ARC_PHYS_TO_HHDM(mcfg_space[segment].base + off);

		return *base;
	}

	uint32_t addr = (1 << 31); // Enable
	addr |= bus << 16;
	addr |= (device & 0b11111) << 11;
	addr |= (function &0b111) << 8;
	addr |= (offset & 0xFC);
	outd(PCI_IO_CFG_ADDRESS, addr);

	return ind(PCI_IO_CFG_DATA);
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

ARC_PCIHeaderMeta *pci_read_header(uint16_t segment, uint8_t bus, uint8_t device) {
	ARC_PCIHeader *header = alloc(sizeof(*header));

	if (header == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate header\n");
		return NULL;
	}

	ARC_PCIHeaderMeta *ret = alloc(sizeof(*ret));

	if (ret == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate meta structure\n");
		free(header);
		return NULL;
	}

	memset(ret, 0, sizeof(*ret));
	memset(header, 0, sizeof(*header));

	ret->header = header;

	ret->segment = segment;
	ret->bus = bus;
	ret->device = device;

	uint32_t *data = (uint32_t *)header;
	for (size_t i = 0; i < sizeof(*header); i += 4) {
		data[i / 4] = pci_read(segment, bus, device, 0, i);
	}

	return ret;
}

int pci_write_header(ARC_PCIHeaderMeta *meta) {
	if (meta == NULL) {
		return -1;
	}

	uint32_t *data = (uint32_t *)meta->header;
	uint16_t segment = meta->segment;
	uint8_t bus = meta->bus;
	uint8_t device = meta->device;

	for (size_t i = 0; i < sizeof(*meta->header); i += 4) {
		pci_write(segment, bus, device, 0, i, 4, data[i / 4]);
	}

	return 0;
}

ARC_PCIHeaderMeta *pci_get_mmio_header(uint16_t segment, uint8_t bus, uint8_t device) {
	if (mcfg_count <= 0) {
		return NULL;
	}

	if (segment > mcfg_count) {
		ARC_DEBUG(ERR, "Invalid segment %d\n", segment);
		return NULL;
	}

	ARC_PCIHeaderMeta *ret = alloc(sizeof(*ret));

	if (ret == NULL) {
		ARC_DEBUG(ERR, "Failed to allocate meta structure\n");
		return NULL;
	}

	memset(ret, 0, sizeof(*ret));

	ret->is_mmio = true;
	ret->segment = segment;
	ret->bus = bus;
	ret->device = device;

	size_t off = ((bus << 20) | (device << 15) | (0 << 12));
	void *base = (uint32_t *)ARC_PHYS_TO_HHDM(mcfg_space[segment].base + off);

	ret->header = base;

	return ret;
}

static int pci_enumerate() {
	// TODO: Make this account for bridges

	for (int i = 0; i < 256; i++) {
		uint32_t vendor_device = pci_get_vendor_device(0, 0, i);

		if (vendor_device == 0xFFFFFFFF) {
			continue;
		}

		ARC_PCIHeaderMeta *header = pci_get_mmio_header(0, 0, i);

		if (header == NULL) {
			header = pci_read_header(0, 0, i);
		}

		init_pci_resource(vendor_device, header);
	}

	return 0;
}

static int setup_mcfg() {
	ARC_MCFGIterator it = NULL;

	while (acpi_get_next_mcfg_entry(&it) == 0) {
		if (mcfg_count == 0) {
			mcfg_space = it;
		}

		ARC_DEBUG(INFO, "Configuration Space %d: Base: 0x%"PRIx64" Group: %d Buses: [%d %d]\n", mcfg_count, it->base, it->seg_group, it->start_bus, it->end_bus);
		mcfg_count++;
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
