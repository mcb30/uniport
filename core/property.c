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
 * Resource properties
 *
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <arpa/inet.h>
#include <uniport/string.h>
#include <uniport/property.h>

/*****************************************************************************
 *
 * Boolean properties
 *
 *****************************************************************************
 */

/**
 * Format property as string
 *
 * @v prop		Property
 * @v buf		String buffer
 * @v len		Length of string buffer
 * @v value		State variable
 * @ret len		Length of string
 */
static size_t boolean_format ( struct property *prop __unused, char *buf,
			       size_t len, const bool *value ) {

	/* Format string */
	return snprintf ( buf, len, "%s", ( *value ? "true" : "false" ) );
}

/**
 * Parse property from a string
 *
 * @v prop		Property
 * @v string		String
 * @v value		State variable
 * @ret rc		Return status code
 */
static int boolean_parse ( struct property *prop __unused, const char *string,
			   bool *value ) {

	/* Parse string */
	if ( ( strcasecmp ( string, "true" ) == 0 ) ||
	     ( strcmp ( string, "1" ) == 0 ) ) {
		*value = true;
	} else if ( ( strcasecmp ( string, "false" ) == 0 ) ||
		    ( strcmp ( string, "0" ) == 0 ) ) {
		*value = false;
	} else {
		return -EINVAL;
	}

	return 0;
}

/** Boolean property type */
const struct property_type boolean_property =
	PROPERTY_TYPE ( "boolean", bool, boolean_format, boolean_parse );

/*****************************************************************************
 *
 * Integer properties
 *
 *****************************************************************************
 */

/**
 * Format property as string
 *
 * @v prop		Property
 * @v buf		String buffer
 * @v len		Length of string buffer
 * @v value		State variable
 * @ret len		Length of string
 */
static size_t integer_format ( struct property *prop __unused, char *buf,
			       size_t len, const int *value ) {

	/* Format string */
	return snprintf ( buf, len, "%d", *value );
}

/**
 * Parse property from a string
 *
 * @v prop		Property
 * @v string		String
 * @v value		State variable
 * @ret rc		Return status code
 */
static int integer_parse ( struct property *prop __unused, const char *string,
			   int *value ) {
	char *end;

	/* Parse string */
	*value = strtol ( string, &end, 0 );
	if ( *end )
		return -EINVAL;

	return 0;
}

/** Integer property type */
const struct property_type integer_property =
	PROPERTY_TYPE ( "integer", int, integer_format, integer_parse );

/*****************************************************************************
 *
 * String properties
 *
 *****************************************************************************
 */

/**
 * Format property as string
 *
 * @v prop		Property
 * @v buf		String buffer
 * @v len		Length of string buffer
 * @v value		State variable
 * @ret len		Length of string
 */
static size_t string_format ( struct property *prop __unused, char *buf,
			      size_t len, const char **value ) {

	/* Format string */
	return snprintf ( buf, len, "%s", *value );
}

/**
 * Parse property from a string
 *
 * @v prop		Property
 * @v string		String
 * @v value		State variable
 * @ret rc		Return status code
 */
static int string_parse ( struct property *prop __unused, const char *string,
			  const char **value ) {

	/* Parse string */
	*value = string;
	return 0;
}

/** String property type */
const struct property_type string_property =
	PROPERTY_TYPE ( "string", const char *, string_format, string_parse );

/*****************************************************************************
 *
 * UUID properties
 *
 *****************************************************************************
 */

/**
 * Format property as string
 *
 * @v prop		Property
 * @v buf		String buffer
 * @v len		Length of string buffer
 * @v value		State variable
 * @ret len		Length of string
 */
static size_t uuid_format ( struct property *prop __unused, char *buf,
			    size_t len, const union uuid *value ) {

	/* Format string */
	return snprintf ( buf, len,
			  "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
			  ntohl ( value->canonical.a ),
			  ntohs ( value->canonical.b ),
			  ntohs ( value->canonical.c ),
			  ntohs ( value->canonical.d ),
			  value->canonical.e[0], value->canonical.e[1],
			  value->canonical.e[2], value->canonical.e[3],
			  value->canonical.e[4], value->canonical.e[5] );
}

/**
 * Parse property from a string
 *
 * @v prop		Property
 * @v string		String
 * @v value		State variable
 * @ret rc		Return status code
 */
static int uuid_parse ( struct property *prop __unused, const char *string,
			union uuid *value ) {
	uint8_t *byte = value->raw;
	unsigned int character;
	unsigned int digit;
	unsigned int i;

	/* Parse string */
	for ( i = 0 ; i < ( sizeof ( value->raw ) * 2 /* digits */ ) ; i++ ) {

		/* Skip any hyphens */
		while ( ( character = *(string++) ) == '-' ) {}

		/* Parse input character */
		*byte <<= 4;
		digit = digit_value ( character );
		if ( digit >= 16 )
			return -EINVAL;
		*byte |= digit;

		/* Move to next output byte if applicable */
		if ( i % 2 )
			byte++;
	}

	return 0;
}

/** UUID property type */
const struct property_type uuid_property =
	PROPERTY_TYPE ( "uuid", union uuid, uuid_format, uuid_parse );

/*****************************************************************************
 *
 * Generic interface
 *
 *****************************************************************************
 */

/**
 * Format property as string
 *
 * @v prop		Property
 * @v buf		String buffer
 * @v len		Length of string buffer
 * @v state		Resource state
 * @ret len		Length of string
 */
size_t property_format ( struct property *prop, char *buf, size_t len,
			 const void *state ) {

	return prop->type->format ( prop, buf, len, ( state + prop->offset ) );
}

/**
 * Format property as allocated string
 *
 * @v prop		Property
 * @v state		Resource state
 * @ret string		Allocated string, or NULL on allocation error
 *
 * The caller is responsible for freeing the allocated string.
 */
char * property_format_alloc ( struct property *prop, const void *state ) {
	char *buf;
	size_t len;
	size_t check;

	/* Calculate length */
	len = property_format ( prop, NULL, 0, state );

	/* Allocate string */
	buf = malloc ( len + 1 /* NUL */ );
	if ( ! buf )
		return NULL;

	/* Format string */
	check = property_format ( prop, buf, ( len + 1 /* NUL */ ), state );
	assert ( check == len );

	return buf;
}

/**
 * Parse property from a string
 *
 * @v prop		Property
 * @v string		String
 * @v state		Resource state
 * @ret rc		Return status code
 */
int property_parse ( struct property *prop, const char *string, void *state ) {

	return prop->type->parse ( prop, string, ( state + prop->offset ) );
}
