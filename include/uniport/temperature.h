#ifndef _UNIPORT_TEMPERATURE_H
#define _UNIPORT_TEMPERATURE_H

/** @file
 *
 * Temperature properties
 *
 */

#include <uniport/property.h>

/** Temperature units */
enum temperature_units {
	TEMPERATURE_UNITS_C = 'C',	/**< Celsius */
	TEMPERATURE_UNITS_F = 'F',	/**< Fahrenheit */
	TEMPERATURE_UNITS_K = 'K',	/**< Kelvin */
};

extern const struct property_type temperature_units_property;

/** Define a temperature units property */
#define PROPERTY_TEMPERATURE_UNITS( _name, _state, _field )		\
	PROPERTY ( _name, _state, _field, enum temperature_units,	\
		   &temperature_units_property )

/** Convert Celsius to Fahrenheit, using integer operations
 *
 * @v c			Temperature in Celsius
 * @ret f		Temperature in Fahrenheit
 */
#define celsius_to_fahrenheit_integer( c )	( ( ( (c) * 9 ) / 5 ) + 32 )

/** Convert Celsius to Fahrenheit, using floating-point operations
 *
 * @v c			Temperature in Celsius
 * @ret f		Temperature in Fahrenheit
 */
#define celsius_to_fahrenheit_floating( c )	( ( (c) * 1.8 ) + 32 )

/** Convert Celsius to Kelvin, using integer operations
 *
 * @v c			Temperature in Celsius
 * @ret k		Temperature in Kelvin
 */
#define celsius_to_kelvin_integer( c )		( (c) + 273 )

/** Convert Celsius to Kelvin, using floating-point operations
 *
 * @v c			Temperature in Celsius
 * @ret k		Temperature in Kelvin
 */
#define celsius_to_kelvin_floating( c )		( (c) + 273.15 )

/** Convert Fahrenheit to Celsius, using integer operations
 *
 * @v f			Temperature in Fahrenheit
 * @ret c		Temperature in Celsius
 */
#define fahrenheit_to_celsius_integer( f )	( ( ( (f) - 32 ) * 5 ) / 9 )

/** Convert Fahrenheit to Celsius, using floating-point operations
 *
 * @v f			Temperature in Fahrenheit
 * @ret c		Temperature in Celsius
 */
#define fahrenheit_to_celsius_floating( f )	( ( (f) - 32 ) / 1.8 )

/** Convert Fahrenheit to Kelvin, using integer operations
 *
 * @v f			Temperature in Fahrenheit
 * @ret k		Temperature in Kelvin
 */
#define fahrenheit_to_kelvin_integer( f )	( ( ( (f) + 460 ) * 5 ) / 9 )

/** Convert Fahrenheit to Kelvin, using floating-point operations
 *
 * @v f			Temperature in Fahrenheit
 * @ret k		Temperature in Kelvin
 */
#define fahrenheit_to_kelvin_floating( f )	( ( (f) + 459.67 ) / 1.8 )

/** Convert Kelvin to Celsius, using integer operations
 *
 * @v k			Temperature in Kelvin
 * @ret c		Temperature in Celsius
 */
#define kelvin_to_celsius_integer( k )		( (k) - 273 )

/** Convert Kelvin to Celsius, using floating-point operations
 *
 * @v k			Temperature in Kelvin
 * @ret c		Temperature in Celsius
 */
#define kelvin_to_celsius_floating( k )		( (k) - 273.15 )

/** Convert Kelvin to Fahrenheit, using integer operations
 *
 * @v k			Temperature in Kelvin
 * @ret f		Temperature in Fahrenheit
 */
#define kelvin_to_fahrenheit_integer( k )	( ( ( (k) * 9 ) / 5 ) - 460 )

/** Convert Kelvin to Fahrenheit, using floating-point operations
 *
 * @v k			Temperature in Kelvin
 * @ret f		Temperature in Fahrenheit
 */
#define kelvin_to_fahrenheit_floating( k )	( ( (k) * 1.8 ) - 459.67 )

/**
 * Convert temperature to Celsius
 *
 * @v temperature		Temperature
 * @v units			Temperature units
 * @v ops			Type of operations
 * @ret celsius			Temperature in Celsius
 */
#define temperature_to_celsius( temperature, units, ops ) ( {		\
	typeof (temperature) tmp = (temperature);			\
	switch (units) {						\
	case TEMPERATURE_UNITS_F:					\
		tmp = fahrenheit_to_celsius_ ## ops ( tmp );		\
		break;							\
	case TEMPERATURE_UNITS_K:					\
		tmp = kelvin_to_celsius_ ## ops ( tmp );		\
		break;							\
	default:							\
		break;							\
	}								\
	tmp;								\
	} )

/**
 * Convert temperature to Fahrenheit
 *
 * @v temperature		Temperature
 * @v units			Temperature units
 * @v ops			Type of operations
 * @ret fahrenheit		Temperature in Fahrenheit
 */
#define temperature_to_fahrenheit( temperature, units, ops ) ( {	\
	typeof (temperature) tmp = (temperature);			\
	switch (units) {						\
	case TEMPERATURE_UNITS_C:					\
		tmp = celsius_to_fahrenheit_ ## ops ( tmp );		\
		break;							\
	case TEMPERATURE_UNITS_K:					\
		tmp = kelvin_to_fahrenheit_ ## ops ( tmp );		\
		break;							\
	default:							\
		break;							\
	}								\
	tmp;								\
	} )

/**
 * Convert temperature to Kelvin
 *
 * @v temperature		Temperature
 * @v units			Temperature units
 * @v ops			Type of operations
 * @ret kelvin			Temperature in Kelvin
 */
#define temperature_to_kelvin( temperature, units, ops ) ( {		\
	typeof (temperature) tmp = (temperature);			\
	switch (units) {						\
	case TEMPERATURE_UNITS_C:					\
		tmp = celsius_to_kelvin_ ## ops ( tmp );		\
		break;							\
	case TEMPERATURE_UNITS_F:					\
		tmp = fahrenheit_to_kelvin_ ## ops ( tmp );		\
		break;							\
	default:							\
		break;							\
	}								\
	tmp;								\
	} )

/** Define a temperature conversion function */
#define TEMPERATURE_CONVERSION( target, type, ops )			\
	TEMPERATURE_CONVERSION_PREFIX type				\
	temperature_to_ ## target ## _ ## type (			\
		type temperature, enum temperature_units units		\
	) {								\
		return temperature_to_ ## target ( temperature, units,	\
						   ops );		\
	}

/* Define temperature converters as extern inline by default */
#ifndef TEMPERATURE_CONVERSION_PREFIX
#define TEMPERATURE_CONVERSION_PREFIX inline
#endif

/* Define temperature converters */
TEMPERATURE_CONVERSION ( celsius, int, integer );
TEMPERATURE_CONVERSION ( celsius, float, floating );
TEMPERATURE_CONVERSION ( celsius, double, floating );
TEMPERATURE_CONVERSION ( fahrenheit, int, integer );
TEMPERATURE_CONVERSION ( fahrenheit, float, floating );
TEMPERATURE_CONVERSION ( fahrenheit, double, floating );
TEMPERATURE_CONVERSION ( kelvin, int, integer );
TEMPERATURE_CONVERSION ( kelvin, float, floating );
TEMPERATURE_CONVERSION ( kelvin, double, floating );

#endif /* _UNIPORT_TEMPERATURE_H */
