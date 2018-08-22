#ifndef _UNIPORT_PROPERTY_H
#define _UNIPORT_PROPERTY_H

/** @file
 *
 * Resource properties
 *
 */

#include <stddef.h>
#include <uniport/uuid.h>

/** A property */
struct property {
	/** Name */
	const char *name;
	/** Offset from start of state descriptor */
	size_t offset;
	/** Property type */
	const struct property_type *type;
};

/** Define a property */
#define PROPERTY( _name, _state, _field, _check, _type ) {		\
	.name = _name,							\
	.offset = ( offsetof ( _state, _field ) +			\
		    ( ( &( ( ( _state * ) NULL )->_field ) ==		\
			( ( _check * ) NULL ) ) ? 0 : 0 ) ),		\
	.type = _type,							\
	}

/** A property type */
struct property_type {
	/** Name */
	const char *name;
	/** Format property as string
	 *
	 * @v prop		Property
	 * @v buf		String buffer
	 * @v len		Length of string buffer
	 * @v value		State variable
	 * @ret len		Length of string
	 */
	size_t ( * format ) ( struct property *prop, char *buf, size_t len,
			      const void *value );
	/** Parse property from a string
	 *
	 * @v prop		Property
	 * @v string		String
	 * @v value		State variable
	 * @ret rc		Return status code
	 */
	int ( * parse ) ( struct property *prop, const char *string,
			  void *state );
};

/** Type of a property format() method */
#define property_format_t( _type )					\
	size_t ( * ) ( struct property *prop, char *buf, size_t len,	\
		       const _type *value )

/** Define a property format() method */
#define PROPERTY_FORMAT( _type, _format )				\
	( ( property_format_t ( void ) )				\
	  ( ( ( ( property_format_t ( _type ) ) NULL )			\
	      == _format ) ? _format : _format ) )

/** Type of a property parse() method */
#define property_parse_t( _type )					\
	int ( * ) ( struct property *prop, const char *string,		\
		    _type *value )

/** Define a property parse() method */
#define PROPERTY_PARSE( _type, _parse )					\
	( ( property_parse_t ( void ) )					\
	  ( ( ( ( property_parse_t ( _type ) ) NULL )			\
	      == _parse ) ? _parse : _parse ) )

/** Define a property type */
#define PROPERTY_TYPE( _name, _type, _format, _parse ) {		\
	.name = _name,							\
	.format = PROPERTY_FORMAT ( _type, _format ),			\
	.parse = PROPERTY_PARSE ( _type, _parse ),			\
	}

extern const struct property_type boolean_property;
extern const struct property_type integer_property;
extern const struct property_type string_property;
extern const struct property_type uuid_property;

/** Define a boolean property */
#define PROPERTY_BOOLEAN( _name, _state, _field ) \
	PROPERTY ( _name, _state, _field, bool, &boolean_property )

/** Define an integer property */
#define PROPERTY_INTEGER( _name, _state, _field ) \
	PROPERTY ( _name, _state, _field, int, &integer_property )

/** Define a string property */
#define PROPERTY_STRING( _name, _state, _field ) \
	PROPERTY ( _name, _state, _field, const char *, &string_property )

/** Define a UUID property */
#define PROPERTY_UUID( _name, _state, _field ) \
	PROPERTY ( _name, _state, _field, union uuid, &uuid_property )

extern size_t property_format ( struct property *prop, char *buf, size_t len,
				const void *state );
extern char * property_format_alloc ( struct property *prop,
				      const void *state );
extern int property_parse ( struct property *prop, const char *string,
			    void *state );

#endif /* _UNIPORT_PROPERTY_H */
