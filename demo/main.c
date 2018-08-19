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
 * Main program
 *
 */

#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include <uniport/init.h>

#define PROMPT "uniport> "

/*
 * Linker table hacks
 *
 * The build system does not yet provide an equivalent of the iPXE
 * REQUIRE_OBJECT() macro.  These external symbol references provide a
 * temporary hack to achieve the same end goal.
 *
 */
extern struct init_fn devices_init_fn;
extern struct command ls_command;
extern struct command show_command;
extern struct command set_command;
extern struct command observe_command;
void *linker_hacks[] = {
	&devices_init_fn,
	&ls_command,
	&show_command,
	&set_command,
	&observe_command,
};

/**
 * Application entry point
 *
 */
void app_main ( void ) {
	char *line;

	/* Configure UART console */
	setvbuf ( stdin, NULL, _IONBF, 0 );
	setvbuf ( stdout, NULL, _IONBF, 0 );
	esp_vfs_dev_uart_set_rx_line_endings ( ESP_LINE_ENDINGS_CR );
	esp_vfs_dev_uart_set_tx_line_endings ( ESP_LINE_ENDINGS_CRLF );
	ESP_ERROR_CHECK ( uart_driver_install ( CONFIG_CONSOLE_UART_NUM,
						256, 0, 0, NULL, 0 ) );
	esp_vfs_dev_uart_use_driver ( CONFIG_CONSOLE_UART_NUM );

	/* Configure line editor */
	linenoiseSetMultiLine ( 1 );
	linenoiseHistorySetMaxLen ( 100 );

	/* Initialise system */
	initialise();

	/* Main loop */
	while ( 1 ) {

		/* Read line */
		line = linenoise ( PROMPT );
		printf ( "\n");
		if ( ! line )
			continue;

		/* Add to command history (ignoring errors) */
		linenoiseHistoryAdd ( line );

		/* Run command */
		system ( line );

		/* Free line */
		free ( line );
	}
}
