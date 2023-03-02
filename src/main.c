/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include "remote.h"

#define LOG_MODULE_NAME app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);
#define RUN_STATUS_LED DK_LED1
#define CONN_STATUS_LED DK_LED2
#define RUN_LED_BLINK_INTERVAL 1000


void button_handler(uint32_t button_state, uint32_t has_changed){
	int button_pressed = 0;
	if(has_changed & button_state){
		switch(has_changed){
			case DK_BTN1_MSK:
				button_pressed = 1;
				break;
			case DK_BTN2_MSK:
				button_pressed = 2;
				break;
			case DK_BTN3_MSK:
				button_pressed = 3;
				break;
			case DK_BTN4_MSK:
				button_pressed = 4;
				break;
			default:
				break;
		}
		LOG_INF("button %d pressed", button_pressed);
	}
}

static void configure_dk_buttons_leds(void){
	int err;
	err = dk_buttons_init(button_handler);
	if(err){
		LOG_ERR("Cannot init buttons (err:%d)", err);
	}
	err = dk_leds_init();
	if(err){
		LOG_ERR("Cannot init leds (err:%d)", err);
	}
}

void main(void)
{
	int err;
	int blink_interval = 0;
	LOG_INF("Hello world! %s\n", CONFIG_BOARD);

	configure_dk_buttons_leds();

	err = bluetooth_init();
	if (err){
		LOG_ERR("bluetooth_init() required %d", err);
	}
	LOG_INF("Running...");

	while(true){
		dk_set_led(RUN_STATUS_LED, (blink_interval++)%2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}

}