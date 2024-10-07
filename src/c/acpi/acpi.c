/**
 * @file acpi.c
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
#include <uacpi/uacpi.h>
#include <uacpi/event.h>
#include <uacpi/tables.h>
#include <fs/vfs.h>
#include <global.h>
#include <lib/util.h>

int acpi_checksum(void *data, size_t length) {
	int8_t *bytes = (int8_t *)data;
	int8_t sum = *bytes;

	for (size_t i = 1; i < length; i++) {
		sum += bytes[i];
	}

	return sum;
}

size_t acpi_get_madt(uint8_t **out) {
	(void)out;

	uacpi_table table = { 0 };
	int r = 0;
	if ((r = uacpi_table_find_by_signature("APIC", &table)) != UACPI_STATUS_OK) {
		ARC_DEBUG(ERR, "Failed to get table %d\n");
		return -1;
	}

	*out = (uint8_t *)table.ptr + 44;

	return table.hdr->length - 44;
}

int init_acpi() {
	if (uacpi_initialize(0) != UACPI_STATUS_OK) {
		ARC_DEBUG(ERR, "Failed to initialize uACPi\n");
		return -1;
	}

	if (uacpi_namespace_load() != UACPI_STATUS_OK) {
		ARC_DEBUG(ERR, "Failed to load ACPI namespace\n");
	}

	if (uacpi_finalize_gpe_initialization() != UACPI_STATUS_OK) {
		ARC_DEBUG(ERR, "Failed to finalize GPE\n");
	}

	ARC_DEBUG(INFO, "Initialized uACPI\n");

        return 0;
}
