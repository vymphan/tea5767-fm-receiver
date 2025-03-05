/*
 * TEA5767_I2C_DRIVER Source File
 * Author: Vy Phan
 * Created on: 04/24/2023
 * Last Updated: 04/28/2023
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdint.h>

#include "i2c_bbb.h"
#include "tea5767_i2c_driver.h"

static BYTE writeBuffer[BUFFER_SIZE] = { 0x00 };

static uint8_t mute_state = 0;
static uint8_t standby_mode = 0;
static uint16_t clock_frequency = 32768;

/****************************************************************
 * Function Name : TEA5767_Init
 * Description   : Connect the i2c bus to the FM module at 
 *                 the given slave address.
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 ****************************************************************/
extern int TEA5767_Init (TEA5767_FM_module *device)
{
    /* Connect to the I2C FM module at the given slave addr */
    if (I2C_ConnectToDevice(*(device->i2c_bus), device->device_addr) < 0)
    {
        /* Fail to connect to the device */
        perror("ERROR: TEA5767 - Failed to init the FM module");
        return -1;
    }

    /* Tune to the default frequency */
    if (TEA5767_SetFrequency(device, 94.73) < 0)
    {
        perror("ERROR: TEA5767 - Failed to tune to the default frequency 94.7MHz");
        return -1;
    }

    return 0;
}


/****************************************************************
 * Function Name : TEA5767_Mute
 * Description   : Mute the audio from the FM module
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 ****************************************************************/
extern int TEA5767_Mute (TEA5767_FM_module *device)
{
    /*  BYTE 1 | Bit 1 | MUTE
     *  If MUTE = 1, then L and R audio are muted
     *  Turn on the bit 1 of BYTE 1 with bit-mask
     */
    mute_state = 1;
    writeBuffer[BYTE_1] |= MUTE_MASK;

    /* Write the buffer to registers */
    if (I2C_WriteRegisters(*(device->i2c_bus), writeBuffer, BUFFER_SIZE) < 0)
    {
        perror("ERROR: TEA5767 - Failed to mute the FM module.");
        return -1;
    }
    return 0;
}

/****************************************************************
 * Function Name : TEA5767_Unmute
 * Description   : Unmute the audio from the FM module
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 ****************************************************************/
extern int TEA5767_Unmute (TEA5767_FM_module *device)
{

    /*  BYTE 1 | Bit 1 | MUTE
     *  If MUTE = 0, then L and R audio are not muted
     *  Turn off the bit 1 of BYTE 1 with bit-mask
     */
    mute_state = 0;
    writeBuffer[BYTE_1] &= UNMUTE_MASK;

    /* Write the buffer to registers */
    if (I2C_WriteRegisters(*(device->i2c_bus), writeBuffer, BUFFER_SIZE) < 0)
    {
        perror("ERROR: TEA5767 - Failed to unmute the FM module.");
        return -1;
    }
    return 0;
}

/****************************************************************
 * Function Name : TEA5767_StandbyON
 * Description   : Turn on Standby mode
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 ****************************************************************/
extern int TEA5767_StandbyON (TEA5767_FM_module *device)
{
    /*  BYTE 4 | Bit 2 | STBY
     *  If STBY = 1, then in Standby mode
     *  Turn on the bit 2 of BYTE 4 with bit-mask
     */
    standby_mode = 1;
    writeBuffer[BYTE_4] |= STANDBY_ON_MASK;

    /* Write the buffer to registers */
    if (I2C_WriteRegisters(*(device->i2c_bus), writeBuffer, BUFFER_SIZE) < 0)
    {
        perror("ERROR: TEA5767 - Failed to turn on Standby mode.");
        return -1;
    }
    return 0;
}

/****************************************************************
 * Function Name : TEA5767_StandbyOFF
 * Description   : Turn off Standby mode
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 ****************************************************************/
extern int TEA5767_StandbyOFF (TEA5767_FM_module *device)
{
    /*  BYTE 4 | Bit 2 | STBY
     *  If STBY = 0, then not in Standby mode
     *  Turn off the bit 2 of BYTE 4 with bit-mask
     */
    standby_mode = 0;
    writeBuffer[BYTE_4] &= STANDBY_OFF_MASK;

    /* Write the buffer to registers */
    if (I2C_WriteRegisters(*(device->i2c_bus), writeBuffer, BUFFER_SIZE) < 0)
    {
        perror("ERROR: TEA5767 - Failed to tune off Standby mode.");
        return -1;
    }
    return 0;
}

/****************************************************************
 * Function Name : TEA5767_SetClockFrequency
 * Description   : set the clock frequency (default at 32.768kHz)
 * Returns       : void
 * Params        @input_clock_freq: the desired clock frequency
 ****************************************************************/
extern void TEA5767_SetClockFrequency (const uint16_t input_clock_freq)
{
    clock_frequency = input_clock_freq;
}

/****************************************************************
 * Function Name : TEA5767_GetClockFrequency
 * Description   : get the current clock frequency
 * Returns       : the current clock requency
 * Params        : N/A
 ****************************************************************/
extern int TEA5767_GetClockFrequency (void)
{
    return clock_frequency;
}

/****************************************************************
 * Function Name : getFrequency (private)
 * Description   : Calculate the decimal value of PLL word
 * Returns       : a word (uint16_t) on success
 * Params        @tuning_freq: the desired frequency for tuning
 ****************************************************************/
static WORD getFrequency (float tuning_freq)
{
    uint16_t ref_freq;

    if (clock_frequency == 32768)
    {
        /* If clock frequency is 32.768kHz, then reference frequency
         * is the same
         */ 
        ref_freq = REF_FREQ_32768HZ;
    }
    else
    {
        /* If clock frequency is 13MHz crystal or 6.5MHz external clock,
         * then set reference frequency to be 50kHz
         */
        ref_freq = REF_FREQ_OTHER;
    }

    /* Calculate PLL */
    return (4 * ((tuning_freq * 1000000) + INTERMEDIATE_FREQ)) / ref_freq; 
}

/****************************************************************
 * Function Name : TEA5767_SetFrequency
 * Description   : Tune to the selected frequency
 * Returns       : 0 on success, -1 on failure
 * Params        @device: the struct contains FM module's i2c file
 *                        descriptor and device address
 *               @tuning_freq: the desired frequency for tuning
 ****************************************************************/
extern int TEA5767_SetFrequency (TEA5767_FM_module *device, float tuning_freq)
{
    /* Calculate the PLL counter from the frequency */
    WORD PLL = getFrequency(tuning_freq);

    /* Set up buffer to write */
    writeBuffer[BYTE_1] = (PLL >> 8) & PLL_MASK_BYTE_1;
    writeBuffer[BYTE_2] = PLL & PLL_MASK_BYTE_2;
    writeBuffer[BYTE_3] = HI_INJECTION;
    writeBuffer[BYTE_4] = XTAL_32768HZ;
    writeBuffer[BYTE_5] = DTC_75US;

    /* Check for Mute state and Standby mode */
    if (mute_state)
    {
        writeBuffer[BYTE_2] |= MUTE_MASK;
    }
    if (standby_mode)
    {
        writeBuffer[BYTE_4] |= STANDBY_ON_MASK;
    }

    /* Write the buffer to registers */
    if (I2C_WriteRegisters(*(device->i2c_bus), writeBuffer, BUFFER_SIZE) < 0)
    {
        perror("ERROR: TEA5767 - Failed to tune to the selected frequency.");
        return -1;
    }
    return 0;
}