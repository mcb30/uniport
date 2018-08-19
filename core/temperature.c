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
 * Temperature properties
 *
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#define TEMPERATURE_CONVERSION_PREFIX extern inline
#include <uniport/temperature.h>

/**
 * Format property as string
 *
 * @v prop		Property
 * @v buf		String buffer
 * @v len		Length of string buffer
 * @v value		State variable
 * @ret len		Length of string
 */
static size_t temperature_units_format ( struct property *prop __unused,
					 char *buf, size_t len,
					 const enum temperature_units *value ) {

	/* Format string */
	return snprintf ( buf, len, "%c", *value );
}

/**
 * Parse property from a string
 *
 * @v prop		Property
 * @v string		String
 * @v value		State variable
 * @ret rc		Return status code
 */
static int temperature_units_parse ( struct property *prop __unused,
				     const char *string,
				     enum temperature_units *value ) {
	int c;
	int unit;

	/* We may exploit the fact that:
	 *
	 * - 'C' occurs exactly once in 'Celsius' and 'Centigrade', and
	 *       does not occur in 'Fahrenheit' or 'Kelvin'
	 * - 'F' occurs exactly once in 'Fahrenheit', and
	 *       does not occur in 'Celsius', 'Centigrade', or 'Kelvin'
	 * - 'K' occurs exactly once in 'Kelvin', and
	 *       does not occur in 'Celsius', 'Centigrade', or 'Fahrenheit'
	 *
	 * Following the general robustness principle, we can
	 * therefore accept inputs of
	 *
	 * - 'C' / 'c' / 'Celsius' / 'Centigrade' / 'degC' / '°C' / etc
	 * - 'F' / 'f' / 'Fahrenheit' / 'degF' / '°F' / etc
	 * - 'K' / 'k' / 'Kelvin' / etc
	 *
	 * solely by checking for the presence of exactly one of the
	 * key letters 'C', 'F', or 'K'.
	 */
	*value = 0;
	while ( ( c = *(string++) ) ) {
		unit = toupper ( c );
		switch ( unit ) {
		case TEMPERATURE_UNITS_C:
		case TEMPERATURE_UNITS_F:
		case TEMPERATURE_UNITS_K:
			/* Accept if this is the only occurence */
			if ( *value )
				return -EINVAL;
			*value = c;
			break;
		default:
			/* Ignore all other characters */
			break;
		}
	}
	if ( ! *value )
		return -EINVAL;
	return 0;
}

/** Temperature units property type */
const struct property_type temperature_units_property =
	PROPERTY_TYPE ( "C/F/K", enum temperature_units,
			temperature_units_format, temperature_units_parse );
