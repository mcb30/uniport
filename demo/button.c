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
 * Button demo
 *
 */

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <uniport/device.h>
#include <uniport/init.h>

/* GPIOs */
#define GPIO_LEFT 13
#define GPIO_RIGHT 14

/** Button state */
struct button_state {
	/** Binary switch value */
	bool value;
	/** Name */
	const char *name;
};

/** Button properties */
static struct property button_props[] = {
	PROPERTY_BOOLEAN ( "value", struct button_state, value ),
	PROPERTY_STRING ( "n", struct button_state, name ),
};

/** A button */
struct button {
	/** Resource */
	struct resource res;
	/** Current state */
	struct button_state state;

	/** GPIO to which button is attached l*/
	unsigned int gpio;
};

/**
 * Retrieve resource state
 *
 * @v res		Resource
 * @ret state		Resource state
 */
static const struct button_state * button_retrieve ( struct resource *res ) {
	struct button *button = container_of ( res, struct button, res );

	/* Update state */
	button->state.value = ( ! gpio_get_level ( button->gpio ) );

	return &button->state;
}

/** Button resource descriptor */
static struct resource_descriptor button_desc =
	RESOURCE_DESC ( struct button_state, button_props,
			button_retrieve, NULL, NULL );

/** Left button */
static struct button button_left = {
	.res = {
		.uri = "left",
		.desc = &button_desc,
		.observers = OBSERVERS_INIT ( button_left.res ),
	},
	.state = {
		.name = "Left button",
	},
	.gpio = GPIO_LEFT,
};

/** Right button */
static struct button button_right = {
	.res = {
		.uri = "right",
		.desc = &button_desc,
		.observers = OBSERVERS_INIT ( button_right.res ),
	},
	.state = {
		.name = "Right button",
	},
	.gpio = GPIO_RIGHT,
};

/** Buttons resources */
static struct resource *buttons_res[] = {
	&button_left.res,
	&button_right.res,
	NULL
};

/** Buttons device */
struct device buttons_dev __device = {
	.name = "buttons",
	.ns = {
		.uri = "/b/",
		.resources = buttons_res,
	},
};

/** Event queue */
static xQueueHandle button_queue = NULL;

/**
 * Button interrupt handler
 *
 * @v opaque		Resource
 */
static void button_isr ( void *opaque ) {

	/* Wake up task */
	xQueueSendFromISR ( button_queue, &opaque, NULL );
}

/**
 * Button task
 *
 * @v arg		Argument (ignored)
 */
static void button_task ( void *arg __unused ) {
	struct button *button;
	bool previous;

	while ( 1 ) {

		/* Receive from event queue */
		if ( xQueueReceive ( button_queue, &button, portMAX_DELAY ) ) {

			/* Notify observers if state has changed */
			previous = button->state.value;
			button_retrieve ( &button->res );
			if ( button->state.value != previous )
				resource_notify ( &button->res );
		}
	}
}

/**
 * Initialise buttons
 *
 */
static void buttons_init ( void ) {
	struct resource **res;
	struct button *button;

	/* Create event queue */
	button_queue = xQueueCreate ( 16, sizeof ( void * ) );

	/* Create notification task */
	xTaskCreate ( button_task, "button_task", 4096, NULL, 10, NULL );

	/* Use per-GPIO interrupts */
	gpio_install_isr_service ( 0 );

	/* Configure GPIOs */
	for ( res = buttons_res ; *res ; res++ ) {
		button = container_of ( *res, struct button, res );
		gpio_set_direction ( button->gpio, GPIO_MODE_INPUT );
		gpio_set_pull_mode ( button->gpio, GPIO_PULLUP_ONLY );
		gpio_set_intr_type ( button->gpio, GPIO_INTR_ANYEDGE );
		button_retrieve ( &button->res );
		gpio_isr_handler_add ( button->gpio, button_isr, button );
	}
}

/** Buttons initialisation function */
struct init_fn buttons_init_fn __init_fn = {
	.init = buttons_init,
};
