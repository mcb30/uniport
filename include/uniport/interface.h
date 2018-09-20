#ifndef _UNIPORT_INTERFACE_H
#define _UNIPORT_INTERFACE_H

/** @file
 *
 * Interfaces
 *
 */

#include <uniport/property.h>
#include <uniport/tables.h>

/** An interface */
struct interface {
	/** Name of the interface */
	const char *name;
	/** Flags field */
	unsigned int flags;
	/** Flags mask */
	unsigned int mask;
};

/** Interfaces table */
#define INTERFACES __table ( struct interface, "interfaces" )

/** Declare an interface */
#define __interface __table_entry ( INTERFACES, 01 )

/**
 * Test if interface has property
 *
 * @v intf		Interface
 * @v prop		Property
 * @ret has_property	Interface has property
 */
static inline int interface_has_property ( struct interface *intf,
					   struct property *prop ) {

	return ( ! ( ( prop->flags ^ intf->flags ) & intf->mask ) );
}

extern struct interface * interface_find ( const char * name );

extern struct interface oic_if_baseline __interface;

#endif /* _UNIPORT_INTERFACE_H */
