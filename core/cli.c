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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <uniport/resource.h>
#include <uniport/command.h>
#include <uniport/parseopt.h>
#include <uniport/interface.h>

/** @file
 *
 * Command line interface
 *
 */

/** A command-line observer */
struct cli_observer {
	/** List of observers */
	struct list_head list;
	/** Observer */
	struct observer obs;
};

/** List of command-line observers */
static struct list_head cli_observers = LIST_HEAD_INIT ( cli_observers );

/**
 * Find command-line observer
 *
 * @v res		Resource
 * @ret obs		Command-line observer, or NULL if not found
 */
static struct cli_observer * cli_observer ( struct resource *res ) {
	struct cli_observer *obs;

	list_for_each_entry ( obs, &cli_observers, list ) {
		if ( obs->obs.res == res )
			return obs;
	}

	return NULL;
}

/**
 * Notify of change in resource state
 *
 * @v obs		Observer
 * @v state		Resource state
 */
static void cli_notify ( struct observer *obs, const void *state ) {

	/* Print resource state */
	resource_print ( obs->res, obs->intf, state );
}

/** "ls" options */
struct ls_options {};

/** "ls" option list */
static struct option_descriptor ls_opts[] = {};

/** "ls" command descriptor */
static struct command_descriptor ls_cmd =
	COMMAND_DESC ( struct ls_options, ls_opts, 0, 0, NULL );

/**
 * "ls" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int ls_exec ( int argc, char **argv ) {
	struct ls_options opts;
	struct namespace *ns;
	struct resource **res;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &ls_cmd, &opts ) ) != 0 )
		return rc;

	/* List all resource URIs */
	list_for_each_entry ( ns, &namespaces, list ) {
		for ( res = ns->resources ; *res ; res++ ) {
			printf ( "%s%s\n", ns->uri, (*res)->uri );
		}
	}

	return 0;
}

/** "ls" command */
struct command ls_command __command = {
	.name = "ls",
	.exec = ls_exec,
};

/** "show" options */
struct show_options {
	/** Interface in use */
	struct interface *intf;
};

/** "show" option list */
static struct option_descriptor show_opts[] = {
	OPTION_DESC ( "interface", 'i', required_argument,
		      struct show_options, intf, parse_interface ),
};

/** "show" command descriptor */
static struct command_descriptor show_cmd =
	COMMAND_DESC ( struct show_options, show_opts, 1, 1, "<uri>" );

/**
 * "show" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int show_exec ( int argc, char **argv ) {
	struct show_options opts;
	struct resource *res;
	const void *state;
	char *uri;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &show_cmd, &opts ) ) != 0 )
		return rc;

	/* Parse resource URI */
	uri = argv[optind];
	if ( ( rc = parse_resource ( uri, &res ) ) != 0 )
		return rc;

	/* Default to baseline interface where not specified */
	if ( ! opts.intf )
		opts.intf = &oic_if_baseline;

	/* Retrieve resource state */
	state = resource_retrieve ( res );

	/* Print resource state */
	resource_print ( res, opts.intf, state );

	return 0;
}

/** "show" command */
struct command show_command __command = {
	.name = "show",
	.exec = show_exec,
};

/** "set" options */
struct set_options {
	/** Interface in use */
	struct interface *intf;
};

/** "set" option list */
static struct option_descriptor set_opts[] = {
	OPTION_DESC ( "interface", 'i', required_argument,
		      struct set_options, intf, parse_interface ),
};

/** "set" command descriptor */
static struct command_descriptor set_cmd =
	COMMAND_DESC ( struct set_options, set_opts, 1, MAX_ARGUMENTS,
		       "<uri> [<prop>=<value>...]" );

/**
 * "set" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int set_exec ( int argc, char **argv ) {
	struct set_options opts;
	struct resource *res;
	struct property *prop;
	void *state;
	char *uri;
	char *name;
	char *sep;
	char *value;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &set_cmd, &opts ) ) != 0 )
		goto err_parse_options;

	/* Parse resource URI */
	uri = argv[optind++];
	if ( ( rc = parse_resource ( uri, &res ) ) != 0 )
		goto err_parse_resource;

	/* Default to baseline interface where not specified */
	if ( ! opts.intf )
		opts.intf = &oic_if_baseline;

	/* Allocate space for copy of resource state */
	state = malloc ( res->desc->len );
	if ( ! state ) {
		rc = -ENOMEM;
		goto err_alloc;
	}

	/* Retrieve and copy resource state */
	memcpy ( state, resource_retrieve ( res ), res->desc->len );

	/* Update properties */
	for ( ; optind < argc ; optind++ ) {

		/* Split into name and value */
		name = argv[optind];
		sep = strchr ( name, '=' );
		if ( ! sep ) {
			print_usage ( &set_cmd, argv );
			rc = -EINVAL;
			goto err_sep;
		}
		*sep = '\0';
		value = ( sep + 1 );

		/* Find property */
		prop = resource_property ( res, name );
		if ( ! prop ) {
			printf ( "\"%s\": no such property\n", name );
			rc = -ENOENT;
			goto err_property;
		}

		/* Check if interface has property */
		if ( ! interface_has_property ( opts.intf, prop ) ) {
			printf ( "\"%s\": not accessible via \"%s\"\n",
				 name, opts.intf->name );
			rc = -ENOTTY;
			goto err_interface;
		}

		/* Check if property is writable */
		if ( ! ( prop->flags & PROP_RW ) ) {
			printf ( "\"%s\": property is read-only\n", name );
			rc = -EROFS;
			goto err_read_only;
		}

		/* Parse property */
		if ( ( rc = property_parse ( prop, value, state ) ) != 0 ) {
			printf ( "\"%s\": %s\n", name, strerror ( rc ) );
			goto err_parse;
		}
	}

	/* Update resource state */
	if ( ( rc = resource_update ( res, state ) ) != 0 ) {
		printf ( "Could not update resource state: %s\n",
			 strerror ( rc ) );
		goto err_update;
	}

 err_update:
 err_parse:
 err_read_only:
 err_interface:
 err_property:
 err_sep:
	free ( state );
 err_alloc:
 err_parse_resource:
 err_parse_options:
	return rc;
}

/** "set" command */
struct command set_command __command = {
	.name = "set",
	.exec = set_exec,
};

/** "observe" options */
struct observe_options {
	/** Delete observer */
	int delete;
	/** Interface in use */
	struct interface *intf;
};

/** "observe" option list */
static struct option_descriptor observe_opts[] = {
	OPTION_DESC ( "delete", 'd', no_argument,
		      struct observe_options, delete, parse_flag ),
	OPTION_DESC ( "interface", 'i', required_argument,
		      struct observe_options, intf, parse_interface ),
};

/** "observe" command descriptor */
static struct command_descriptor observe_cmd =
	COMMAND_DESC ( struct observe_options, observe_opts, 1, 1, "<uri>" );

/**
 * "observe" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int observe_exec ( int argc, char **argv ) {
	struct observe_options opts;
	struct resource *res;
	struct cli_observer *obs;
	char *uri;
	int rc;

	/* Parse options */
	if ( ( rc = parse_options ( argc, argv, &observe_cmd, &opts ) ) != 0 )
		return rc;

	/* Parse resource URI */
	uri = argv[optind];
	if ( ( rc = parse_resource ( uri, &res ) ) != 0 )
		return rc;

	/* Default to baseline interface where not specified */
	if ( ! opts.intf )
		opts.intf = &oic_if_baseline;

	/* Find existing observer, if any */
	obs = cli_observer ( res );

	/* Create, delete, or modify observer as applicable */
	if ( ( ! obs ) && ( ! opts.delete ) ) {
		obs = malloc ( sizeof ( *obs ) );
		if ( ! obs )
			return -ENOMEM;
		observer_init ( &obs->obs, res, opts.intf, cli_notify );
		list_add_tail ( &obs->list, &cli_observers );
		resource_observe ( &obs->obs );
	} else if ( obs && opts.delete ) {
		resource_unobserve ( &obs->obs );
		list_del ( &obs->list );
		free ( obs );
	} else if ( obs ) {
		resource_unobserve ( &obs->obs );
		obs->obs.intf = opts.intf;
		resource_observe ( &obs->obs );
	}

	return 0;
}

/** "observe" command */
struct command observe_command __command = {
	.name = "observe",
	.exec = observe_exec,
};
