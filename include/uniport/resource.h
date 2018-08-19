#ifndef _UNIPORT_RESOURCE_H
#define _UNIPORT_RESOURCE_H

/** @file
 *
 * Resources
 *
 */

#include <stdbool.h>
#include <stddef.h>
#include <uniport/list.h>
#include <uniport/property.h>

/** A resource namespace */
struct namespace {
	/** List of namespaces */
	struct list_head list;
	/** URI prefix (including the trailing '/') */
	const char *uri;
	/** List of resources */
	struct resource **resources;
};

/** A resource */
struct resource {
	/** URI suffix */
	const char *uri;
	/** Resource descriptor */
	const struct resource_descriptor *desc;
	/** List of observers */
	struct list_head observers;
};

/** A resource observer */
struct observer {
	/** Resource */
	struct resource *res;
	/** List of observers */
	struct list_head list;
	/**
	 * Notify of change in resource state
	 *
	 * @v obs		Observer
	 * @v state		Resource state
	 *
	 * This method is not permitted to modify the list of
	 * observers.
	 */
	void ( * notify ) ( struct observer *obs, const void *state );
};

/** Initialise observers list */
#define OBSERVERS_INIT( _res ) LIST_HEAD_INIT ( _res.observers )

/** Resource descriptor */
struct resource_descriptor {
	/** Length of resource state */
	size_t len;
	/** Properties */
	struct property *props;
	/** Number of properties */
	unsigned int count;
	/** Retrieve resource state
	 *
	 * @v res		Resource
	 * @ret state		Resource state
	 */
	const void * ( * retrieve ) ( struct resource *res );
	/** Update resource state
	 *
	 * @v res		Resource
	 * @v state		New resource state
	 * @ret rc		Return status code
	 *
	 * May be NULL for a read-only resource.
	 */
	int ( * update ) ( struct resource *res, const void *state );
	/** Update observation state
	 *
	 * @v res		Resource
	 *
	 * May be NULL for an unobservable resource.
	 */
	void ( * observe ) ( struct resource *res );
};

/** Type of a resource retrieve() method */
#define resource_retrieve_t( _type )					\
	const _type * ( * ) ( struct resource *res )

/** Define a resource retrieve() method */
#define RESOURCE_RETRIEVE( _type, _retrieve )				\
	( ( resource_retrieve_t ( void ) )				\
	  ( ( ( ( resource_retrieve_t ( _type ) ) NULL )		\
	      == _retrieve ) ? _retrieve : _retrieve ) )

/** Type of a resource update() method */
#define resource_update_t( _type )					\
	int ( * ) ( struct resource *res, const _type *state )

/** Define a resource update() method */
#define RESOURCE_UPDATE( _type, _update )				\
	( ( resource_update_t ( void ) )				\
	  ( ( ( ( resource_update_t ( _type ) ) NULL )			\
	      == _update ) ? _update : _update ) )

/** Define a resource descriptor */
#define RESOURCE_DESC( _type, _props, _retrieve, _update, _observe ) {	\
	.len = sizeof ( _type ),					\
	.props = _props,						\
	.count = ( sizeof ( _props ) / sizeof ( _props[0] ) ), 		\
	.retrieve = RESOURCE_RETRIEVE ( _type, _retrieve ),		\
	.update = RESOURCE_UPDATE ( _type, _update ),			\
	.observe = _observe,						\
	}

/**
 * Initialise observer
 *
 * @v obs		Observer
 * @v res		Resource
 * @v notify		Notification handler
 */
static inline __attribute__ (( always_inline )) void
observer_init ( struct observer *obs, struct resource *res,
		void ( * notify ) ( struct observer *obs,
				    const void *state ) ) {

	obs->res = res;
	obs->notify = notify;
}

/**
 * Check if resource has observers
 *
 * @v res		Resource
 * @ret has_observers	Resource has observers
 */
static inline __attribute__ (( always_inline )) bool
resource_has_observers ( struct resource *res ) {

	return ( ! list_empty ( &res->observers ) );
}

extern struct list_head namespaces;

extern const void * resource_retrieve ( struct resource *res );
extern int resource_update ( struct resource *res, const void *state );
extern void resource_observe ( struct observer *obs );
extern void resource_unobserve ( struct observer *obs );
extern void resource_notify ( struct resource *res );
extern void resource_print ( struct resource *res, const void *state );
extern int resource_register ( struct namespace *ns );
extern void resource_unregister ( struct namespace *ns );
extern struct resource * resource_find ( const char *uri );
extern struct property * resource_property ( struct resource *res,
					     const char *name );

#endif /* _UNIPORT_RESOURCE_H */
