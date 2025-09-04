/**
 * @file info.h
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
#ifndef ARC_ARCH_INFO_H
#define ARC_ARCH_INFO_H

#include <stdint.h>

typedef enum {
        ARC_ARCH_TYPE_INTEL,
        ARC_ARCH_TYPE_AMD,
        ARC_ARCH_TYPE_CENTAUR,
        ARC_ARCH_TYPE_CYRIX,
        ARC_ARCH_TYPE_TRANSMETA,
        ARC_ARCH_TYPE_NSC,
        ARC_ARCH_TYPE_NEXGEN,
        ARC_ARCH_TYPE_RISE,
        ARC_ARCH_TYPE_SIS,
        ARC_ARCH_TYPE_UMC,
        ARC_ARCH_TYPE_VORTEX86,
        ARC_ARCH_TYPE_ZHAOXIN,
        ARC_ARCH_TYPE_HYGON,
        ARC_ARCH_TYPE_RDC,
        ARC_ARCH_TYPE_MCST,
        ARC_ARCH_TYPE_VIA,
        ARC_ARCH_TYPE_AMDK5,
        ARC_ARCH_TYPE_A0486_OLD,
        ARC_ARCH_TYPE_A0486_NEW,
        ARC_ARCH_TYPE_CONNECTIX6,
        ARC_ARCH_TYPE_MSVPC7,
        ARC_ARCH_TYPE_INSIGNIA,
        ARC_ARCH_TYPE_COMPAQ,
        ARC_ARCH_TYPE_POWERVM,
        ARC_ARCH_TYPE_NEKO,
        ARC_ARCH_TYPE_MAX
} ARC_ARCHTYPE;

uint32_t arch_physical_address_width();
uint32_t arch_virtual_address_width();
ARC_ARCHTYPE arch_processor_type();
uint64_t arch_get_cycles();

#endif
