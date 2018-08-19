#ifndef _UNIPORT_COMMAND_H
#define _UNIPORT_COMMAND_H

#include <uniport/tables.h>

/** A command-line command */
struct command {
	/** Name of the command */
	const char *name;
	/**
	 * Function implementing the command
	 *
	 * @v argc		Argument count
	 * @v argv		Argument list
	 * @ret rc		Return status code
	 */
	int ( * exec ) ( int argc, char **argv );
};

/** Commands linker table */
#define COMMANDS __table ( struct command, "commands" )

/** Declare a command */
#define __command __table_entry ( COMMANDS, 01 )

#endif /* _UNIPORT_COMMAND_H */
