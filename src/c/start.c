/**
 * @file start.c
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
#include <arch/start.h>
#include <arch/acpi/acpi.h>
#include <arch/pager.h>
#include <global.h>
#include <mm/allocator.h>
#include <mm/algo/allocator.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <boot/parse.h>
#include <lib/perms.h>

#include <fs/vfs.h>
#include <lib/resource.h>
#include <drivers/dri_defs.h>

#ifdef ARC_TARGET_ARCH_X86_64
#include <arch/x86-64/gdt.h>
#include <arch/x86-64/idt.h>
#include <arch/x86-64/sse.h>
#include <arch/x86-64/syscall.h>
#include <arch/x86-64/apic/apic.h>
#endif

int init_arch() {
	if (init_pmm((struct ARC_MMap *)Arc_BootMeta->arc_mmap, Arc_BootMeta->arc_mmap_len) != 0) {
		ARC_DEBUG(ERR, "Failed to initialize physical memory manager\n");
		ARC_HANG;
	}

	if (init_pager() != 0) {
		ARC_DEBUG(ERR, "Failed to initialize architectural pager\n");
		ARC_HANG;
	}

	if (parse_boot_info() != 0) {
		ARC_DEBUG(ERR, "Failed to parse boot information\n");
		ARC_HANG;
	}

	if (Arc_MainTerm.framebuffer != NULL) {
		Arc_MainTerm.term_width = (Arc_MainTerm.fb_width / Arc_MainTerm.font_width);
		Arc_MainTerm.term_height = (Arc_MainTerm.fb_height / Arc_MainTerm.font_height);
	}

	// Initialize the internal SLAB allocator
	if (init_iallocator(128) != 0) {
		ARC_DEBUG(ERR, "Failed to initialize internal allocator\n");
		ARC_HANG;
	}

	// Initialize the top level kernel allocator
	if (init_vmm((void *)(ARC_HHDM_VADDR + Arc_BootMeta->highest_address), 0x100000000000) != 0) {
		ARC_DEBUG(ERR, "Failed to initialize virtual memory manager\n");
		ARC_HANG;
	}

	if (init_allocator(256) != 0) {
		ARC_DEBUG(ERR, "Failed to initialize kernel allocator\n");
		ARC_HANG;
	}

#ifdef ARC_TARGET_ARCH_X86_64
	init_gdt();
	init_idt();
#endif

	init_vfs();

	vfs_create("/initramfs/", ARC_STD_PERM, ARC_VFS_N_DIR, NULL);
        vfs_create("/dev/", ARC_STD_PERM, ARC_VFS_N_DIR, NULL);

	Arc_InitramfsRes = init_resource(0, ARC_SDRI_INITRAMFS, (void *)ARC_PHYS_TO_HHDM(Arc_BootMeta->initramfs));
	vfs_mount("/initramfs/", Arc_InitramfsRes);

        init_acpi();

#ifdef ARC_TARGET_ARCH_X86_64
        if (init_apic() != 0) {
		ARC_DEBUG(ERR, "Failed to initialize interrupts\n");
		ARC_HANG;
	}
	__asm__("sti");
	init_syscall();
#endif

	vfs_link("/initramfs/boot/ANTIQUE.F14", "/font.fnt", -1);
	vfs_rename("/font.fnt", "/fonts/font.fnt");
	vfs_open("/fonts/font.fnt", 0, ARC_STD_PERM, (void *)&Arc_FontFile);

	return 0;
}
