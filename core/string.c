/*
 * Copyright (C) 2015 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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
 * String functions
 *
 */

#include <uniport/string.h>

/**
 * Calculate digit value
 *
 * @v character		Digit character
 * @ret digit		Digit value
 *
 * Invalid digits will be returned as a value greater than or equal to
 * the numeric base.
 */
unsigned int digit_value ( unsigned int character ) {

	if ( character >= 'a' )
		return ( character - ( 'a' - 10 ) );
	if ( character >= 'A' )
		return ( character - ( 'A' - 10 ) );
	if ( character <= '9' )
		return ( character - '0' );
	return character;
}
