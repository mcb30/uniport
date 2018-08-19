#ifndef _UNIPORT_DEVICE_H
#define _UNIPORT_DEVICE_H

/** @file
 *
 * Devices
 *
 */

#include <uniport/tables.h>
#include <uniport/resource.h>

/** A device */
struct device {
	/** Name */
	const char *name;
	/** Resource namespace */
	struct namespace ns;
};

/** Devices linker table */
#define DEVICES __table ( struct device, "devices" )

/** Declare a device */
#define __device __table_entry ( DEVICES, 01 )

#endif /* _UNIPORT_DEVICE_H */
