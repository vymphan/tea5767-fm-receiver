/*
 * gpio.h
 * Author: Vy Phan
 * Created on: 04/24/2023
 * Last Updated: 04/25/2023
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdio.h>      // Standard I/O
#include <stdint.h>     // Fixed-width int type
#include <string.h>     // String-handling library
#include <unistd.h>     // POSIX API
#include <fcntl.h>      // File controls

/* Path to gpio folders in BBB */
#define GPIO_PATH   "/sys/class/gpio/"
/* Predefined common value for GPIO */
#define GPIO_IN     "in"
#define GPIO_OUT    "out"
#define GPIO_HI     "1"
#define GPIO_LO     "0"

#define P9_11 30
#define P9_12 60
#define P9_13 31
#define P9_18 4
#define P9_24 15
#define P9_26 14
#define P9_27 115


/****************************************************************
 * Function Name : GPIO_SetDirection
 * Description   : Set the direction of a GPIO - input or output
 * Returns       : 0 on success, -1 on failure
 * Params        : @gpio: the GPIO number
 *                 @p_direction: the direction in a string
 ****************************************************************/
int GPIO_SetDirection (const uint8_t gpio, const char* p_direction);

/****************************************************************
 * Function Name : GPIO_ReadValue
 * Description   : Read the input value of a GPIO
 * Returns       : pin value on success, -1 on failure
 * Params        : @gpio: the input GPIO number
 ****************************************************************/
int GPIO_ReadValue(const uint8_t gpio);

/****************************************************************
 * Function Name : GPIO_WriteValue
 * Description   : Write a value to an ouput GPIO
 * Returns       : 0 on success, -1 on failure
 * Params        : @gpio: the output GPIO number
 *                 @p_value: the string value to be written
 ****************************************************************/
int GPIO_WriteValue(const uint8_t gpio, const char* p_value);

#endif
