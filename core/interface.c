/*
 * Copyright (C) 2018 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <string.h>
#include <uniport/tables.h>
#include <uniport/interface.h>

/** @file
 *
 * Interfaces
 *
 */

/** Baseline interface */
struct interface oic_if_baseline __interface = {
	.name = "oic.if.baseline",
	.flags = 0,
	.mask = 0,
};

/** Sensor interface */
struct interface oic_if_sensor __interface = {
	.name = "oic.if.s",
	.flags = 0,
	.mask = ( PROP_META | PROP_RW ),
};

/** Actuator interface */
struct interface oic_if_actuator __interface = {
	.name = "oic.if.a",
	.flags = PROP_RW,
	.mask = ( PROP_META | PROP_RW ),
};

/** Read-only interface */
struct interface oic_if_read_only __interface = {
	.name = "oic.if.r",
	.flags = 0,
	.mask = PROP_RW,
};

/** Read/write interface */
struct interface oic_if_read_write __interface = {
	.name = "oic.if.rw",
	.flags = PROP_RW,
	.mask = PROP_RW,
};

/**
 * Find interface
 *
 * @v name		Interface name
 * @ret intf		Interface, or NULL if not found
 */
struct interface * interface_find ( const char * name ) {
	struct interface *intf;

	for_each_table_entry ( intf, INTERFACES ) {
		if ( strcmp ( name, intf->name ) == 0 )
			return intf;
	}
	return NULL;
}
