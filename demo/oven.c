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
 * Oven demo
 *
 */

#include "driver/gpio.h"
#include <uniport/device.h>
#include <uniport/temperature.h>
#include <uniport/init.h>

/** Power control */
#define OVEN_GPIO_POWER 23
/** Power control state */
struct oven_power_state {
	/** Binary switch value */
	bool value;
	/** Name */
	const char *name;
};

/** Power control properties */
static struct property oven_power_props[] = {
	PROPERTY_BOOLEAN ( "value", struct oven_power_state, value, PROP_RW ),
	PROPERTY_STRING ( "n", struct oven_power_state, name,
			  PROP_RW | PROP_META ),
};

/** Power control */
struct oven_power {
	/** Resource */
	struct resource res;
	/** Current state */
	struct oven_power_state state;
	/** GPIO */
	unsigned int gpio;
};

/** Temperature state */
struct oven_temperature_state {
	/** Temperature */
	int temperature;
	/** Units */
	enum temperature_units units;
	/** Name */
	const char *name;
};

/** Current temperature properties */
static struct property oven_current_props[] = {
	PROPERTY_INTEGER ( "temperature", struct oven_temperature_state,
			   temperature, 0 ),
	PROPERTY_TEMPERATURE_UNITS ( "units", struct oven_temperature_state,
				     units, 0 ),
	PROPERTY_STRING ( "n", struct oven_temperature_state, name,
			  PROP_RW | PROP_META ),
};

/** Target temperature properties */
static struct property oven_target_props[] = {
	PROPERTY_INTEGER ( "temperature", struct oven_temperature_state,
			   temperature, PROP_RW ),
	PROPERTY_TEMPERATURE_UNITS ( "units", struct oven_temperature_state,
				     units, PROP_RW ),
	PROPERTY_STRING ( "n", struct oven_temperature_state, name,
			  PROP_RW | PROP_META ),
};

/** Temperature */
struct oven_temperature {
	/** Resource */
	struct resource res;
	/** Current state */
	struct oven_temperature_state state;
};

/** An oven */
struct oven {
	/** Power control */
	struct oven_power power;
	/** Target temperature */
	struct oven_temperature target;
	/** Current temperature */
	struct oven_temperature current;
};

/**
 * Retrieve power control state
 *
 * @v res		Resource
 * @ret state		Resource state
 */
static const struct oven_power_state *
oven_power_retrieve ( struct resource *res ) {
	struct oven *oven = container_of ( res, struct oven, power.res );

	return &oven->power.state;
}

/**
 * Update power control state
 *
 * @v res		Resource
 * @v state		New resource state
 * @ret rc		Return status code
 */
static int oven_power_update ( struct resource *res,
			       const struct oven_power_state *state ) {
	struct oven *oven = container_of ( res, struct oven, power.res );

	/* Update power state */
	oven->power.state.value = state->value;
	gpio_set_level ( oven->power.gpio, oven->power.state.value );

	return 0;
}

/** Power control resource descriptor */
static struct resource_descriptor oven_power_desc =
	RESOURCE_DESC ( struct oven_power_state, oven_power_props,
			oven_power_retrieve, oven_power_update, NULL );

/**
 * Retrieve temperature state
 *
 * @v res		Resource
 * @ret state		Resource state
 */
static const struct oven_temperature_state *
oven_temperature_retrieve ( struct resource *res ) {
	struct oven_temperature *temp =
		container_of ( res, struct oven_temperature, res );

	return &temp->state;
}

/**
 * Update target temperature state
 *
 * @v res		Resource
 * @v state		New resource state
 * @ret rc		Return status code
 */
static int oven_target_update ( struct resource *res,
				const struct oven_temperature_state *state ) {
	struct oven *oven = container_of ( res, struct oven, target.res );

	/* Update target temperature */
	oven->target.state.temperature =
		temperature_to_celsius_int ( state->temperature, state->units );

	return 0;
}

/** Target temperature resource descriptor */
static struct resource_descriptor oven_target_desc =
	RESOURCE_DESC ( struct oven_temperature_state, oven_target_props,
			oven_temperature_retrieve, oven_target_update, NULL );

/** Current temperature resource descriptor */
static struct resource_descriptor oven_current_desc =
	RESOURCE_DESC ( struct oven_temperature_state, oven_current_props,
			oven_temperature_retrieve, NULL, NULL );

/** The oven */
static struct oven oven = {
	.power = {
		.res = {
			.uri = "power",
			.desc = &oven_power_desc,
			.observers = OBSERVERS_INIT ( oven.power.res ),
		},
		.state = {
			.name = "Power Control",
		},
		.gpio = OVEN_GPIO_POWER,
	},
	.target = {
		.res = {
			.uri = "target",
			.desc = &oven_target_desc,
			.observers = OBSERVERS_INIT ( oven.target.res ),
		},
		.state = {
			.name = "Target Temperature",
			.units = TEMPERATURE_UNITS_C,
		},
	},
	.current = {
		.res = {
			.uri = "current",
			.desc = &oven_current_desc,
			.observers = OBSERVERS_INIT ( oven.current.res ),
		},
		.state = {
			.name = "Current Temperature",
			.units = TEMPERATURE_UNITS_C,
		},
	},
};

/** Oven resources */
static struct resource *oven_res[] = {
	&oven.power.res,
	&oven.target.res,
	&oven.current.res,
	NULL
};

/** Oven device */
struct device oven_dev __device = {
	.name = "oven",
	.ns = {
		.uri = "/o/",
		.resources = oven_res,
	},
};

/**
 * Initialise oven
 *
 */
static void oven_init ( void ) {

	/* Configure GPIOs */
	gpio_set_direction ( oven.power.gpio, GPIO_MODE_OUTPUT );
}

/** Oven initialisation function */
struct init_fn oven_init_fn __init_fn = {
	.init = oven_init,
};
