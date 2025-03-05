/*
 * TEA5767_I2C_DRIVER Header File
 * Author: Vy Phan
 * Created on: 04/24/2023
 * Last Updated: 04/28/2023
 */

#ifndef TEA5767_I2C_DRIVER_H
#define TEA5767_I2C_DRIVER_H

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdint.h>

#include "i2c_bbb.h"

#define BUFFER_SIZE 5

#define BYTE_1 0
#define BYTE_2 1
#define BYTE_3 2
#define BYTE_4 3
#define BYTE_5 4

#define PLL_MASK_BYTE_1 0x3F // BYTE 1 | bit 5-0 | PLL[13:8] | AND
#define PLL_MASK_BYTE_2 0xFF // BYTE 2 | bit 7-0 | PLL[7:0] | AND
#define HI_INJECTION    0x10 // BYTE 3 | bit 4 | OR
#define LO_INJECTION    0xEF // BYTE 3 | bit 4 | AND
#define MONO_MASK       0x08 // BYTE 3 | bit 3 | OR
#define STEREO_MASK     0xF7 // BYTE 3 | bit 3 | AND
#define XTAL_32768HZ    0x10 // BYTE 4 | bit 4 | OR
#define DTC_75US        0x40 // BYTE 5 | bit 5 | OR | PLLREF = 0

#define MUTE_MASK             0x80   // BYTE 1 | bit 1 | OR
#define UNMUTE_MASK           0x7F   // BYTE 1 | bit 1 | AND
#define STANDBY_ON_MASK       0x40   // BYTE 4 | bit 2 | OR
#define STANDBY_OFF_MASK      0xBF   // BYTE 4 | bit 2 | AND

#define INTERMEDIATE_FREQ 225000 // 225kHz
#define REF_FREQ_32768HZ  32768  // 32.768kHz crystal
#define REF_FREQ_OTHER    50000  // 13MHz crystal or 6.5MHz external clock

#define DEFAULT_FREQ 94.7 // 94.7 Station

typedef struct TEA5767_FM_module {
    int *i2c_bus;
    BYTE device_addr;
} TEA5767_FM_module;

/****************************************************************
 * Function Name : TEA5767_Init
 * Description   : Connect the i2c bus to the FM module at 
 *                 the given slave address.
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 ****************************************************************/
extern int TEA5767_Init (TEA5767_FM_module *device);

/****************************************************************
 * Function Name : TEA5767_Mute
 * Description   : Mute the audio from the FM module
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 ****************************************************************/
extern int TEA5767_Mute (TEA5767_FM_module *device);

/****************************************************************
 * Function Name : TEA5767_Unmute
 * Description   : Unmute the audio from the FM module
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 ****************************************************************/
extern int TEA5767_Unmute (TEA5767_FM_module *device);

/****************************************************************
 * Function Name : TEA5767_StandbyON
 * Description   : Turn on Standby mode
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 ****************************************************************/
extern int TEA5767_StandbyON (TEA5767_FM_module *device);

/****************************************************************
 * Function Name : TEA5767_StandbyOFF
 * Description   : Turn off Standby mode
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 ****************************************************************/
extern int TEA5767_StandbyOFF (TEA5767_FM_module *device);

/****************************************************************
 * Function Name : TEA5767_SetClockFrequency
 * Description   : set the clock frequency (default at 32.768kHz)
 * Returns       : void
 * Params        @input_clock_freq: the desired clock frequency
 ****************************************************************/
extern void TEA5767_SetClockFrequency (uint16_t input_clock_freq);

/****************************************************************
 * Function Name : TEA5767_GetClockFrequency
 * Description   : get the current clock frequency
 * Returns       : the current clock requency
 * Params        : N/A
 ****************************************************************/
extern int TEA5767_GetClockFrequency (void);

/****************************************************************
 * Function Name : TEA5767_SetFrequency
 * Description   : Tune to the selected frequency
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 *               @tuning_freq: the desired frequency for tuning
 ****************************************************************/
extern int TEA5767_SetFrequency (TEA5767_FM_module *device, float tuning_freq);

#endif