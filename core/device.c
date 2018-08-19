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
 *
 */

/** @file
 *
 * Devices
 *
 */

#include <uniport/device.h>
#include <uniport/init.h>

/**
 * Initialise devices
 *
 */
static void devices_init ( void ) {
	struct device *dev;
	int rc;

	/* Register all device namespaces */
	for_each_table_entry ( dev, DEVICES ) {
		rc = resource_register ( &dev->ns );
		assert ( rc == 0 );
	}
}

/** Device initialisation function */
struct init_fn devices_init_fn __init_fn = {
	.init = devices_init,
};
