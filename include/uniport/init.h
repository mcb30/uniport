#ifndef _UNIPORT_INIT_H
#define _UNIPORT_INIT_H

/** @file
 *
 * Initialisation functions
 *
 */

#include <uniport/tables.h>

/**
 * An initialisation function
 *
 * Initialisation functions are called exactly once, as part of the
 * call to initialise().
 */
struct init_fn {
	/** Initialise */
        void ( * init ) ( void );
};

/** Initialisation function table */
#define INIT_FNS __table ( struct init_fn, "init_fns" )

/** Declare an initialisation functon */
#define __init_fn __table_entry ( INIT_FNS, 01 )

extern void initialise ( void );

#endif /* _UNIPORT_INIT_H */
