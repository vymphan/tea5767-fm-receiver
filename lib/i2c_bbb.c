/*
 * i2c_bbb.c
 * Author: Vy Phan
 * Created: 04/24/2023
 * Last Updated: 04/27/2023
 */


#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdint.h>

#include "i2c_bbb.h"

/****************************************************************
 * Function Name : I2C_OpenBus
 * Description   : Open the I2C bus
 * Returns       : 0 on success, -1 on failure
 * Params        @p_i2c_bus: Pointer to the file descriptor
 *               @p_i2c_dev_path: String path to the i2c bus 
 ****************************************************************/
extern int8_t I2C_OpenBus (int *p_i2c_bus, char *p_i2c_dev_path)
{
    /* open the i2c bus */
    if ((*p_i2c_bus = open(p_i2c_dev_path, O_RDWR)) < 0)
    {
        /* Failed to open the i2c bus */
        perror ("ERROR: I2C - Failed to open the bus.");
        return -1;
    }
    return 0;
}

/****************************************************************
 * Function Name : I2C_ConnectToDevice
 * Description   : Connect to the slave device
 * Returns       : 0 on success, -1 on failure
 * Params        @p_i2c_bus: file descriptor of i2c bus
 *               @slave_addr: 7-bit address of the slave device
 *                       Bit 7 = MSB    Bit 0 = R/W
 ****************************************************************/
extern int8_t I2C_ConnectToDevice (int i2c_bus, BYTE slave_addr)
{
    /* Connect to the slave device through the i2c bus */
    if (ioctl(i2c_bus, I2C_SLAVE, slave_addr) < 0)
    {
        perror("ERROR: I2C - Failed to connect to the slave device.");
        /* Close the bus */
        I2C_Close(i2c_bus);
        return -1;
    }
    return 0;
}

/****************************************************************
 * Function Name : I2C_Init
 * Description   : Open the i2c bus and connect to the device
 * Returns       : 0 on success, -1 on failure
 * Params        @p_i2c_bus: Pointer to the file descriptor
 *               @p_i2c_dev_path: String path to the i2c bus 
 *               @slave_addr: 7-bit addresss of the slave device
 *                          Bit 7 = MSB     Bit 0 = R/W
 ****************************************************************/
extern int8_t I2C_Init (int *p_i2c_bus, char *p_i2c_dev_path, BYTE slave_addr)
{
    /* The initialization includes opening the i2c bus and set
    the slave address */
    if (I2C_OpenBus(p_i2c_bus, p_i2c_dev_path) < 0)
    {
        /* Failed to open the bus */
        perror("ERROR: I2C - Failed to init the slave device.");
        return -1;
    }
    if (I2C_ConnectToDevice(*p_i2c_bus, slave_addr) < 0)
    {
        /* Failed to set the slave address */
        perror("ERROR: I2C - Failed to init the slave device.");
        /* Close the bus */
        I2C_Close(*p_i2c_bus);
        return -1;
    }
    return 0;
}


/****************************************************************
 * Function Name : I2C_Read
 * Description   : Read data from a register
 * Returns       : 0 on success, -1 on failure
 * Params        @i2c_bus: file descriptor of i2c bus
 *               @reg_addr: register's address (hex)
 *               @p_buffer: to store the read data from register
 ****************************************************************/
extern int8_t I2C_Read (int i2c_bus, BYTE reg_addr, BYTE *p_buffer)
{
    int8_t ret = 0;

    /* First, Write the register address*/
    if (write(i2c_bus, &reg_addr, I2C_ONE_BYTE) < 0)
    {
        perror("ERROR: I2C - Failed to reset the read address\n");
        /* Close the bus */
        I2C_Close(i2c_bus);
        return -1;
    }

    /* Read the data byte of the register
    Note: p_buffer is an array of bytes, storing the read data */
    ret = read(i2c_bus, p_buffer, I2C_ONE_BYTE);
    
    if (ret == -1)
    {
        /* Failed to read */
        perror("ERROR: I2C - Failed to read");
        /* Close the bus */
        I2C_Close(i2c_bus);
        return -1;
    }
    return ret;
}

/****************************************************************
 * Function Name : I2C_Write
 * Description   : Write one byte to a register
 * Returns       : 0 on success, -1 on failure
 * Params        @i2c_bus: file descriptor of i2c bus
 *               @reg_addr: register's address (hex)
 *               @data: data byte (hex) to be written on register
 ****************************************************************/
extern int8_t I2C_Write (int i2c_bus, BYTE reg_addr, BYTE data)
{
    /* Prepare buffer to write to register */
    BYTE buffer[2];
    buffer[0] = reg_addr;
    buffer[1] = data;

    int8_t ret = 0;
    /* Write to register */
    ret = write(i2c_bus, buffer, I2C_TWO_BYTES);

    /* If successfully written to registers, the write function will return
    the correct number of bytes that have been written to registers */
    if ((ret == -1) || (ret != I2C_TWO_BYTES))
    {
        /* Failed to write to the register */
        perror("ERROR: I2C - Failed to write to the register.");
        /* Close the bus */
        I2C_Close(i2c_bus);
        return -1;
    }
    return 0;
}

extern int8_t I2C_ReadRegisters (int i2c_bus, BYTE *p_data, uint8_t num_of_bytes)
{
    /* Read the "number of bytes" from the registers and store the given byte array pointer*/
    if (read(i2c_bus, p_data, num_of_bytes) != num_of_bytes)
    {
        /* Failed to read from the registers */
        perror("ERROR: I2C - Failed to read from registers");
        /* Close the bus */
        I2C_Close(i2c_bus);
        return -1;
    }
    return 0;

}

/****************************************************************
 * Function Name : I2C_WriteRegisters
 * Description   : Write to multiple registers
 * Returns       : number of written byte on sucess, -1 on failure
 * Params        @i2c_bus: file descriptor of i2c bus
 *               @p_data: a pointer to the data buffer
 *               @num_of_bytes: the number of bytes to be written
 ****************************************************************/
extern int8_t I2C_WriteRegisters (int i2c_bus, BYTE *p_data, uint8_t num_of_bytes)
{
    int8_t ret = 0;
    /* Write to register */
    ret = write(i2c_bus, p_data, num_of_bytes);

    /* If successfully written to registers, the write function will return
    the correct number of bytes that have been written to registers */
    if ((ret == -1) || (ret != num_of_bytes))
    {
        /* Failed to write to the register */
        perror("ERROR: I2C - Failed to write to the registers.");
        /* Close the bus */
        I2C_Close(i2c_bus);
        return -1;
    }
    return ret;
}

/****************************************************************
 * Function Name : I2C_Close
 * Description   : Close the I2C bus
 * Returns       : 0 on success, -1 on failure
 * Params        : @i2c_bus: file descriptor
 ****************************************************************/
extern int8_t I2C_Close (int i2c_bus)
{
    /* Close the i2c bus */
    if (close(i2c_bus) < 0)
    {
        /* Failed to close */
        perror ("ERROR: I2C - Failed to close bus.");
        return -1;
    }
    return 0;
}
