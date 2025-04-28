/**
 * @file acpi.c
 *
 * @author awewsomegamer <awewsomegamer@gmail.com>
 *
 * @LICENSE
 * Arctan - Operating System Kernel
 * Copyright (C) 2023-2025 awewsomegamer
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
#include <uacpi/namespace.h>
#include <uacpi/resources.h>
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

size_t acpi_get_table(const char *id, uint8_t **out) {
	uacpi_table table = { 0 };
	int r = 0;

	if ((r = uacpi_table_find_by_signature(id, &table)) != UACPI_STATUS_OK) {
		ARC_DEBUG(ERR, "Failed to get table (%d)\n", r);
		return -1;
	}

	*out = (uint8_t *)table.ptr + 44;

	return table.hdr->length - 44;
}

uacpi_resource_iteration_decision res_ls_callback(void *user, uacpi_resource *resource) {
	switch(resource->type) {
		case UACPI_RESOURCE_TYPE_IRQ:{
			printf("\tIRQs:\n");
			for (int i = 0; i < resource->irq.num_irqs; i++) {
				printf("\t\t%X\n", resource->irq.irqs[i]);
			}
			break;
		}

		case UACPI_RESOURCE_TYPE_IO: {
			printf("IO: 0x%X -> 0x%X (%d) ALIGN %d DECODE %d\n", resource->io.minimum, resource->io.maximum, resource->io.length, resource->io.alignment, resource->io.decode_type);
			break;
		}

		case UACPI_RESOURCE_TYPE_FIXED_IO: {
			printf("FIXED IO: 0x%X (%d)\n", resource->fixed_io.address, resource->fixed_io.length);
			break;
		}
	}

	return UACPI_RESOURCE_ITERATION_CONTINUE;
}

uacpi_ns_iteration_decision ls_callback(void *user, uacpi_namespace_node *node) {
	printf("%s\n", uacpi_namespace_node_generate_absolute_path(node));

	struct uacpi_resources *out_resources = NULL;

	if (uacpi_get_current_resources(node, &out_resources) == UACPI_STATUS_OK) {
		uacpi_for_each_resource(out_resources, res_ls_callback, NULL);
	}

	return UACPI_NS_ITERATION_DECISION_CONTINUE;
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

	uacpi_namespace_for_each_node_depth_first(uacpi_namespace_root(), ls_callback, NULL);

        return 0;
}
