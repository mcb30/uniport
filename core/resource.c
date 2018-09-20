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
 * Resources
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <uniport/resource.h>
#include <uniport/interface.h>

/** List of resource namespaces */
struct list_head namespaces = LIST_HEAD_INIT ( namespaces );

/**
 * Retrieve resource state
 *
 * @v res		Resource
 * @ret state		Resource state
 */
const void * resource_retrieve ( struct resource *res ) {

	/* Retrieve resource state */
	return res->desc->retrieve ( res );
}

/**
 * Update resource state
 *
 * @v res		Resource
 * @v state		New resource state
 * @ret rc		Return status code
 */
int resource_update ( struct resource *res, const void *state ) {

	/* Fail if resource is not updatable */
	if ( ! res->desc->update )
		return -ENOTSUP;

	/* Update resource state */
	return res->desc->update ( res, state );
}

/**
 * Add observer
 *
 * @v obs		Observer
 */
void resource_observe ( struct observer *obs ) {
	struct resource *res = obs->res;

	/* Add to list of observers */
	list_add_tail ( &obs->list, &res->observers );

	/* Update observation state, if applicable */
	if ( res->desc->observe )
		res->desc->observe ( res );
}

/**
 * Remove observer
 *
 * @v obs		Observer
 */
void resource_unobserve ( struct observer *obs ) {
	struct resource *res = obs->res;

	/* Remove from list of observers */
	list_del ( &obs->list );

	/* Update observation state, if applicable */
	if ( res->desc->observe )
		res->desc->observe ( res );
}

/**
 * Notify observers of change in resource state
 *
 * @v res		Resource
 */
void resource_notify ( struct resource *res ) {
	struct observer *obs;
	const void *state;

	/* Retrieve resource state */
	state = resource_retrieve ( res );

	/* Notify each observer */
	list_for_each_entry ( obs, &res->observers, list )
		obs->notify ( obs, state );
}

/**
 * Print resource state (for debugging)
 *
 * @v res		Resource
 * @v state		Resource state
 * @v intf		Interface
 */
void resource_print ( struct resource *res, struct interface *intf,
		      const void *state ) {
	struct property *prop;
	char *value;
	unsigned int i;

	/* Print properties */
	printf ( "%s:", res->uri );
	for ( i = 0 ; i < res->desc->count ; i++ ) {
		prop = &res->desc->props[i];
		if ( ! interface_has_property ( intf, prop ) )
			continue;
		value = property_format_alloc ( prop, state );
		printf ( " %s=%s", prop->name, ( value ? value : "<ENOMEM>" ) );
		free ( value );
	}
	printf ( "\n" );
}

/**
 * Find resource namespace
 *
 * @v uri		URI
 * @ret ns		Resource namespace, or NULL if not found
 */
static struct namespace * resource_namespace ( const char *uri ) {
	struct namespace *ns;

	/* Find matching namespace and resource */
	list_for_each_entry ( ns, &namespaces, list ) {
		if ( strncmp ( ns->uri, uri, strlen ( ns->uri ) ) == 0 )
			return ns;
	}

	return NULL;
}

/**
 * Register resource namespace
 *
 * @v ns		Resource namespace
 * @ret rc		Return status code
 */
int resource_register ( struct namespace *ns ) {
	struct resource **res;

	/* Sanity check */
	if ( resource_namespace ( ns->uri ) != NULL )
		return -EINVAL;

	/* Add to list of namespaces */
	list_add_tail ( &ns->list, &namespaces );

	/* Print initial resource state for diagnostics */
	printf ( "Namespace %s...\n", ns->uri );
	for ( res = ns->resources ; *res ; res++ ) {
		resource_print ( *res, &oic_if_baseline,
				 resource_retrieve ( *res ) );
	}

	return 0;
}

/**
 * Unregister resource namespace
 *
 * @v ns		Resource namespace
 */
void resource_unregister ( struct namespace *ns ) {

	/* Remove from list of namespaces */
	list_del ( &ns->list );
}

/**
 * Find resource by URI
 *
 * @v uri		URI
 * @ret res		Resource, or NULL if not found
 */
struct resource * resource_find ( const char *uri ) {
	struct namespace *ns;
	struct resource **res;
	size_t len;

	/* Find resource namespace */
	ns = resource_namespace ( uri );
	if ( ! ns )
		return NULL;

	/* Find matching resource */
	len = strlen ( ns->uri );
	assert ( ns->uri[ len - 1 ] == '/' );
	for ( res = ns->resources ; *res ; res++ ) {
		if ( strcmp ( (*res)->uri, ( uri + len ) ) == 0 )
			return *res;
	}

	return NULL;
}

/**
 * Find resource property
 *
 * @v res		Resource
 * @v name		Property name
 * @ret prop		Property, or NULL if not found
 */
struct property * resource_property ( struct resource *res, const char *name ) {
	struct property *prop;
	unsigned int i;

	/* Find property */
	for ( i = 0 ; i < res->desc->count ; i++ ) {
		prop = &res->desc->props[i];
		if ( strcmp ( prop->name, name ) == 0 )
			return prop;
	}

	return NULL;
}
