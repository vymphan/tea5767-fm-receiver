/*
 *  gpio.h
 *  Author: Vy Phan
 *  Created on: 04/24/2023
 *  Last Updated: 04/25/2023
 */
#include <stdio.h>      // Standard I/O
#include <stdint.h>     // Fixed-width int type
#include <string.h>     // String-handling library
#include <unistd.h>     // POSIX API
#include <fcntl.h>      // File controls
#include "gpio.h"       // Its header file

/****************************************************************
 * Function Name : GPIO_SetDirection
 * Description   : Set the direction of a GPIO - input or output
 * Returns       : 0 on success, -1 on failure
 * Params        : @gpio: the GPIO number
 *                 @p_direction: the direction in a string
 ****************************************************************/
int GPIO_SetDirection (const uint8_t gpio, const char* p_direction)
{
    int8_t ret = 0;      // to store the error code of function
    int fd = 0;      // to store opened GPIO file directory
    char buffer[50] = ""; // to store the formatted gpio path string

    /* Format the gpio path directory */
    ret = snprintf(buffer, sizeof(buffer), "%sgpio%d/direction", GPIO_PATH, gpio); 
    /* Check for formating error */
    if (ret < 0) 
    {
        perror("ERROR: GPIO - Failed to format the GPIO string path"); 
        return -1;
    }

    /* Open the GPIO file directory */
    fd = open(buffer, O_RDWR);
    /* Check for file directory opening error */
    if (fd < 0) 
    { 
        perror("ERROR: GPIO - Failed to open the GPIO file directory"); 
        return -1;
    }

    /* Write the direction to the gpio file directory */
    ret = write(fd, p_direction, strlen(p_direction)); 
    /* Check for writing error */
    if (ret < 0) 
    { 
        perror("ERROR: GPIO - Failed to write to the GPIO"); 
        return -1; 
    }

    /* Close the GPIO file directory */
    ret = close(fd);
    /* Check for closing error */
    if (ret < 0) 
    { 
        perror("ERROR: GPIO - Failed to close the GPIO file directory"); 
        return -1; 
    }

    return 0;
}

/****************************************************************
 * Function Name : GPIO_ReadValue
 * Description   : Read the input value of a GPIO
 * Returns       : pin value on success, -1 on failure
 * Params        : @gpio: the input GPIO number
 ****************************************************************/
int GPIO_ReadValue(const uint8_t gpio)
{
    int8_t ret = 0;      // to store the error code of the function
    uint8_t pinValue = 0; // to store the read input from the GPIO
    int fd = 0;      // to store the opened gpio file directory
    char buffer[50] = ""; // to store the formatted gpio file pathA
    char state[1] = "";   // buffer to store the input

     /* Format the gpio path to set the direction */
    ret = snprintf(buffer, sizeof(buffer), "%sgpio%d/value", GPIO_PATH, gpio);
	/* Check for formating error */
	if (ret < 0) 
    { 
        perror("ERROR: GPIO - Failed to format the GPIO string path"); 
        return -1; 
    }
    
	/* Open the GPIO file directory */
    fd = open(buffer, O_RDONLY);
	/* Check for opening error */
	if (ret < 0) 
    { 
        perror("ERROR: GPIO - Failed to open the GPIO file directory"); 
        return -1;
    }
	
	/* Read from the GPIO file directory */
    ret = read(fd, state, sizeof(state));
	/* check for writing error */
	if (ret < 0) 
    { 
        perror("ERROR: GPIO - Failed to read to the GPIO file directory"); 
        return -1;
    }
    
	/* Close the GPIO file directory */
	ret = close(fd);
	/* Check for closing error */
	if (ret < 0) 
    { 
        perror("ERROR: GPIO - Failed to close the GPIO file directory"); 
        return -1;
    }
	
	/* Convert char state into int state */
	if (!strcmp(state, "1"))
    {
        pinValue = 1;
    }
    else
    {
        pinValue = 0;
    }
    return pinValue;
}

/****************************************************************
 * Function Name : GPIO_WriteValue
 * Description   : Write a value to an ouput GPIO
 * Returns       : 0 on success, -1 on failure
 * Params        : @gpio: the output GPIO number
 *                 @p_value: the string value to be written
 ****************************************************************/
int GPIO_WriteValue(const uint8_t gpio, const char* p_value)
{
    int8_t ret = 0;      // to store the error code of the function
    int fd = 0;      // to store the opened gpio file directory
    char buffer[50] = ""; // to store the formatted gpio file pathA

     /* Format the gpio path to set the direction */
    ret = snprintf(buffer, sizeof(buffer), "%sgpio%d/value", GPIO_PATH, gpio);
	/* Check for formating error */
	if (ret < 0) 
    { 
        perror("ERROR: GPIO - Failed to format the GPIO string path"); 
        return -1;
    }
    
	/* Open the GPIO file directory */
    fd = open(buffer, O_WRONLY);
	/* Check for opening error */
	if (ret < 0) 
    { 
        perror("ERROR: GPIO - Failed to open the GPIO file directory"); 
        return -1;
    }
	
	/* Write to the GPIO file directory */
    ret = write(fd, p_value, 1);
	/* check for writing error */
	if (ret < 0)
    {
        perror("ERROR: GPIO - Failed to write to the GPIO file directory");
        return -1;
    }
    
	/* Close the GPIO file directory */
	ret = close(fd);
	/* Check for closing error */
	if (ret < 0) 
    { 
        perror("ERROR: GPIO - Failed to close the GPIO file directory"); 
        return -1;
    }
	return 0;
}
