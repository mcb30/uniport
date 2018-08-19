/*
 * Copyright (C) 2006 Michael Brown <mbrown@fensystems.co.uk>.
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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <uniport/tables.h>
#include <uniport/command.h>
#include <uniport/parseopt.h>

/** @file
 *
 * Command execution
 *
 * Derived from the implementation in iPXE.
 *
 */

/**
 * Execute command
 *
 * @v command		Command name
 * @v argv		Argument list
 * @ret rc		Return status code
 *
 * Execute the named command.  Unlike a traditional POSIX execv(),
 * this function returns the exit status of the command.
 */
int execv ( const char *command, char * const argv[] ) {
	struct command *cmd;
	int argc;

	/* Count number of arguments */
	for ( argc = 0 ; argv[argc] ; argc++ ) {}

	/* An empty command is deemed to do nothing, successfully */
	if ( command == NULL ) {
		return 0;
	}

	/* Sanity checks */
	if ( argc == 0 )
		return -EINVAL;

	/* Reset getopt() library ready for use by the command */
	optind = 0;

	/* Hand off to command implementation */
	for_each_table_entry ( cmd, COMMANDS ) {
		if ( strcmp ( command, cmd->name ) == 0 ) {
			return cmd->exec ( argc, ( char ** ) argv );
		}
	}

	printf ( "%s: command not found\n", command );
	return -ENOEXEC;
}

/**
 * Split command line into tokens
 *
 * @v command		Command line
 * @v tokens		Token list to populate, or NULL
 * @ret count		Number of tokens
 *
 * Splits the command line into whitespace-delimited tokens.  If @c
 * tokens is non-NULL, any whitespace in the command line will be
 * replaced with NULs.
 */
static int split_command ( char *command, char **tokens ) {
	int count = 0;
	int c;

	while ( 1 ) {
		/* Skip over any whitespace / convert to NUL */
		while ( isspace ( ( c = *command ) ) ) {
			if ( tokens )
				*command = '\0';
			command++;
		}
		/* Check for end of line */
		if ( ! c )
			break;
		/* We have found the start of the next argument */
		if ( tokens )
			tokens[count] = command;
		count++;
		/* Skip to start of next whitespace, if any */
		while ( ( c = *command ) && ! isspace ( c ) ) {
			command++;
		}
	}
	return count;
}

/**
 * Execute command line
 *
 * @v command		Command line
 * @ret rc		Return status code
 *
 * Execute the named command and arguments.
 */
int system ( const char *command ) {
	int argc = split_command ( ( char * ) command, NULL );
	char *argv[ argc + 1 ];
	char *command_copy;
	int rc;

	/* Create modifiable copy of command */
	command_copy = strdup ( command );
	if ( ! command_copy )
		return -ENOMEM;

	/* Split command into tokens */
	split_command ( command_copy, argv );
	argv[argc] = NULL;

	/* Execute command */
	rc = execv ( argv[0], argv );

	/* Free modified copy of command */
	free ( command_copy );

	return rc;
}

/**
 * "help" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Return status code
 */
static int help_exec ( int argc __unused, char **argv __unused ) {
	struct command *command;
	unsigned int hpos = 0;

	printf ( "\nAvailable commands:\n\n" );
	for_each_table_entry ( command, COMMANDS ) {
		hpos += printf ( "  %s", command->name );
		if ( hpos > ( 16 * 4 ) ) {
			printf ( "\n" );
			hpos = 0;
		} else {
			while ( hpos % 16 ) {
				printf ( " " );
				hpos++;
			}
		}
	}
	printf ( "\n\nType \"<command> --help\" for further information\n\n" );
	return 0;
}

/** "help" command */
struct command help_command __command = {
	.name = "help",
	.exec = help_exec,
};
