#!/bin/bash

gcc -pthread main.c lib/gpio.c lib/gpio.h lib/i2c_bbb.c lib/i2c_bbb.h lib/tea5767_i2c_driver.c lib/tea5767_i2c_driver.h -o fm_receiver -Wall -Werror
