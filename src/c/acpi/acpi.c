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
#include "interface/printf.h"
#include "lib/resource.h"
#include "uacpi/utilities.h"
#include <uacpi/namespace.h>
#include <uacpi/resources.h>
#include <arch/acpi/acpi.h>
#include <uacpi/uacpi.h>
#include <uacpi/event.h>
#include <uacpi/tables.h>
#include <fs/vfs.h>
#include <global.h>
#include <lib/util.h>
#include <mm/allocator.h>

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

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
uint64_t acpi_hash_resource_hid(char *hid, size_t len) {
	uint64_t hash = 0xcbf29ce484222325;

	for (size_t i = 0; i < len; i++) {
		hash ^= hid[i];
		hash *= 0x100000001b3;
	}

	return hash;
}

static int acpi_clean_up_args(struct ARC_ACPIDevInfo *args) {
	if (args == NULL) {
		return -1;
	}

	struct ARC_ACPIDevIO *io = args->io;

	while (io != NULL) {
		void *next = io->next;
		free(io);
		io = next;
	}

	struct ARC_ACPIDevIRQ *irq = args->irq;

	while (irq != NULL) {
		void *next = irq->next;
		free(irq);
		irq = next;
	}

	return 0;
}

uacpi_resource_iteration_decision res_ls_callback(void *user, uacpi_resource *resource) {
	struct ARC_ACPIDevInfo *info = (struct ARC_ACPIDevInfo *)user;

	if (info == NULL) {
		return UACPI_RESOURCE_ITERATION_ABORT;
	}

	switch(resource->type) {
		case UACPI_RESOURCE_TYPE_IRQ:{
			for (int i = 0; i < resource->irq.num_irqs; i++) {
				ARC_DEBUG(INFO, "\tIRQ: %d\n", resource->irq.irqs[i]);
			}

			struct ARC_ACPIDevIRQ *irq = alloc(sizeof(*irq));

			if (irq == NULL) {
				ARC_DEBUG(ERR, "Failed to allocate IRQ descriptor\n");
				return UACPI_RESOURCE_ITERATION_ABORT;
			}

			irq->irq_count = resource->irq.num_irqs;
			irq->irq_list = resource->irq.irqs;
			irq->length_kind = resource->irq.length_kind;
			irq->polarity = resource->irq.polarity;
			irq->sharing = resource->irq.sharing;
			irq->triggering = resource->irq.triggering;
			irq->wake_capability = resource->irq.wake_capability;

			if (info->irq != NULL) {
				irq->next = info->irq;	
			}

			info->irq = irq;

			break;
		}

		case UACPI_RESOURCE_TYPE_IO: {
			ARC_DEBUG(INFO, "\tIO: 0x%X -> 0x%X (%d) ALIGN %d DECODE %d\n", resource->io.minimum, resource->io.maximum, resource->io.length, resource->io.alignment, resource->io.decode_type);

			struct ARC_ACPIDevIO *io = alloc(sizeof(*io));

			if (io == NULL) {
				ARC_DEBUG(ERR, "Failed to allocate IRQ descriptor\n");
				return UACPI_RESOURCE_ITERATION_ABORT;
			}

			io->base = resource->io.minimum;
			io->length = resource->io.length;
			io->align = resource->io.alignment;
			io->decode_type = resource->io.decode_type;

			if (info->io != NULL) {
				io->next = info->io;	
			}

			info->io = io;

			break;
		}

		case UACPI_RESOURCE_TYPE_FIXED_IO: {
			ARC_DEBUG(INFO, "\tFIXED IO: 0x%X (%d)\n", resource->fixed_io.address, resource->fixed_io.length);
			struct ARC_ACPIDevIO *io = alloc(sizeof(*io));

			if (io == NULL) {
				ARC_DEBUG(ERR, "Failed to allocate IRQ descriptor\n");
				return UACPI_RESOURCE_ITERATION_ABORT;
			}

			io->base = resource->fixed_io.address;
			io->length = resource->fixed_io.length;

			if (info->io != NULL) {
				io->next = info->io;	
			}

			info->io = io;

			break;
		}
	}

	return UACPI_RESOURCE_ITERATION_CONTINUE;
}

uacpi_ns_iteration_decision ls_callback(void *user, uacpi_namespace_node *node) {
	(void)user;
	
	struct uacpi_resources *out_resources = NULL;

	if (uacpi_get_current_resources(node, &out_resources) == UACPI_STATUS_OK) {
		uacpi_id_string *uid = NULL;
		uacpi_id_string *hid = NULL;
		uint64_t hash = 0;

		uacpi_eval_uid(node, &uid);
		if (uacpi_eval_hid(node, &hid) == UACPI_STATUS_OK) {
			hash = acpi_hash_resource_hid(hid->value, hid->size);
		}

		ARC_DEBUG(INFO, "%s (UID: %.*s HID: %.*s -> 0x%"PRIX64")\n", uacpi_namespace_node_generate_absolute_path(node), 
						              (uid != NULL ? uid->size : 0), (uid != NULL ? uid->value : 0),
						              (hid != NULL ? hid->size : 0), (hid != NULL ? hid->value : 0),
							      hash);
							
		struct ARC_ACPIDevInfo info = { 0 };
		uacpi_for_each_resource(out_resources, res_ls_callback, (void *)&info);

		init_acpi_resource(hash, (void *)&info);
		// acpi_clean_up_args(&info);
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
