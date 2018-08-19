/*
 * Copyright (C) 2010 Michael Brown <mbrown@fensystems.co.uk>.
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

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>
#include <uniport/resource.h>
#include <uniport/parseopt.h>

/** @file
 *
 * Command line option parsing
 *
 * Derived from the implementation in iPXE.
 *
 */

/**
 * Parse string value
 *
 * @v text		Text
 * @ret value		String value
 * @ret rc		Return status code
 */
int parse_string ( char *text, char **value ) {

	/* Sanity check */
	assert ( text != NULL );

	/* Parse string */
	*value = text;

	return 0;
}

/**
 * Parse integer value
 *
 * @v text		Text
 * @ret value		Integer value
 * @ret rc		Return status code
 */
int parse_integer ( char *text, unsigned int *value ) {
	char *endp;

	/* Sanity check */
	assert ( text != NULL );

	/* Parse integer */
	*value = strtoul ( text, &endp, 0 );
	if ( *endp ) {
		printf ( "\"%s\": invalid integer value\n", text );
		return -EINVAL;
	}

	return 0;
}

/**
 * Parse flag
 *
 * @v text		Text (ignored)
 * @ret flag		Flag to set
 * @ret rc		Return status code
 */
int parse_flag ( char *text __unused, int *flag ) {

	/* Set flag */
	*flag = 1;

	return 0;
}

/**
 * Parse resource URI
 *
 * @v text		Text
 * @ret res		Resource
 * @ret rc		Return status code
 */
int parse_resource ( char *text, struct resource **res ) {

	/* Find resource */
	*res = resource_find ( text );
	if ( ! *res ) {
		printf ( "\"%s\": no such resource\n", text );
		return -ENOENT;
	}

	return 0;
}

/**
 * Print command usage message
 *
 * @v cmd		Command descriptor
 * @v argv		Argument list
 */
void print_usage ( struct command_descriptor *cmd, char **argv ) {
	struct option_descriptor *option;
	unsigned int i;
	int is_optional;

	printf ( "Usage:\n\n  %s", argv[0] );
	for ( i = 0 ; i < cmd->num_options ; i++ ) {
		option = &cmd->options[i];
		printf ( " [-%c|--%s", option->shortopt, option->longopt );
		if ( option->has_arg ) {
			is_optional = ( option->has_arg == optional_argument );
			printf ( " %s<%s>%s", ( is_optional ? "[" : "" ),
				 option->longopt, ( is_optional ? "]" : "" ) );
		}
		printf ( "]" );
	}
	if ( cmd->usage )
		printf ( " %s", cmd->usage );
	printf ( "\n\n" );
}

/**
 * Reparse command-line options
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @v cmd		Command descriptor
 * @v opts		Options (already initialised with default values)
 * @ret rc		Return status code
 */
int reparse_options ( int argc, char **argv, struct command_descriptor *cmd,
		      void *opts ) {
	struct option longopts[ cmd->num_options + 1 /* help */ + 1 /* end */ ];
	char shortopts[ cmd->num_options * 3 /* possible "::" */ + 1 /* "h" */
			+ 1 /* NUL */ ];
	unsigned int shortopt_idx = 0;
	int ( * parse ) ( char *text, void *value );
	void *value;
	unsigned int i;
	unsigned int j;
	unsigned int num_args;
	int c;
	int rc;

	/* Construct long and short option lists for getopt_long() */
	memset ( longopts, 0, sizeof ( longopts ) );
	for ( i = 0 ; i < cmd->num_options ; i++ ) {
		longopts[i].name = cmd->options[i].longopt;
		longopts[i].has_arg = cmd->options[i].has_arg;
		longopts[i].val = cmd->options[i].shortopt;
		shortopts[shortopt_idx++] = cmd->options[i].shortopt;
		assert ( cmd->options[i].has_arg <= optional_argument );
		for ( j = cmd->options[i].has_arg ; j > 0 ; j-- )
			shortopts[shortopt_idx++] = ':';
	}
	longopts[i].name = "help";
	longopts[i].val = 'h';
	shortopts[shortopt_idx++] = 'h';
	shortopts[shortopt_idx++] = '\0';
	assert ( shortopt_idx <= sizeof ( shortopts ) );

	/* Parse options */
	while ( ( c = getopt_long ( argc, argv, shortopts, longopts,
				    NULL ) ) >= 0 ) {
		switch ( c ) {
		case 'h' :
			/* Print help */
			print_usage ( cmd, argv );
			return -ECANCELED;
		case '?' :
			/* Print usage message */
			print_usage ( cmd, argv );
			return -EINVAL;
		case ':' :
			/* Print usage message */
			print_usage ( cmd, argv );
			return -EINVAL;
		default:
			/* Search for an option to parse */
			for ( i = 0 ; i < cmd->num_options ; i++ ) {
				if ( c != cmd->options[i].shortopt )
					continue;
				parse = cmd->options[i].parse;
				value = ( opts + cmd->options[i].offset );
				if ( ( rc = parse ( optarg, value ) ) != 0 )
					return rc;
				break;
			}
			assert ( i < cmd->num_options );
		}
	}

	/* Check remaining arguments */
	num_args = ( argc - optind );
	if ( ( num_args < cmd->min_args ) || ( num_args > cmd->max_args ) ) {
		print_usage ( cmd, argv );
		return -ERANGE;
	}

	return 0;
}

/**
 * Parse command-line options
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @v cmd		Command descriptor
 * @v opts		Options (may be uninitialised)
 * @ret rc		Return status code
 */
int parse_options ( int argc, char **argv, struct command_descriptor *cmd,
		    void *opts ) {

	/* Clear options */
	memset ( opts, 0, cmd->len );

	return reparse_options ( argc, argv, cmd, opts );
}
