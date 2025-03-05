/*
 * i2c_bbb.h
 * Author: Vy Phan
 * Created: 04/24/2023
 * Last Updated: 04/27/2023
 */
#ifndef I2C_BBB_H
#define I2C_BBB_H

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdint.h>

#define I2C_ONE_BYTE     1
#define I2C_TWO_BYTES    2
#define I2C_THREE_BYTES  3
#define I2C_FOUR_BYTES   4
#define I2C_FIVE_BYTES   5
#define I2C_SIX_BYTES    6

#define I2C_0_DEV_PATH   "/dev/i2c-0"
#define I2C_1_DEV_PATH   "/dev/i2c-1"
#define I2C_2_DEV_PATH   "/dev/i2c-2"

typedef uint8_t  BYTE;
typedef uint16_t WORD;


/****************************************************************
 * Function Name : I2C_OpenBus
 * Description   : Open the I2C bus
 * Returns       : 0 on success, -1 on failure
 * Params        @p_i2c_bus: Pointer to the file descriptor
 *               @p_i2c_dev_path: String path to the i2c bus 
 ****************************************************************/
extern int8_t I2C_OpenBus (int *p_i2c_bus, char *p_i2c_dev_path);

/****************************************************************
 * Function Name : I2C_ConnectToDevice
 * Description   : Connect to the slave device
 * Returns       : 0 on success, -1 on failure
 * Params        @p_i2c_bus: file descriptor of i2c bus
 *               @slave_addr: 7-bit address of the slave device
 *                       Bit 7 = MSB    Bit 0 = R/W
 ****************************************************************/
extern int8_t I2C_ConnectToDevice (int i2c_bus, BYTE slave_addr);

/****************************************************************
 * Function Name : I2C_Init
 * Description   : Open the i2c bus and connect to the device
 * Returns       : 0 on success, -1 on failure
 * Params        @p_i2c_bus: Pointer to the file descriptor
 *               @p_i2c_dev_path: String path to the i2c bus 
 *               @slave_addr: 7-bit addresss of the slave device
 *                          Bit 7 = MSB     Bit 0 = R/W
 ****************************************************************/
extern int8_t I2C_Init (int *p_i2c_bus, char *p_i2c_dev_path, BYTE slave_addr);

/****************************************************************
 * Function Name : I2C_Read
 * Description   : Read data from a register
 * Returns       : 0 on success, -1 on failure
 * Params        @i2c_bus: file descriptor of i2c bus
 *               @reg_addr: register's address (hex)
 *               @p_buffer: to store the read data from register
 ****************************************************************/
extern int8_t I2C_Read (int i2c_bus, BYTE reg_addr, BYTE *p_buffer);

/****************************************************************
 * Function Name : I2C_Write
 * Description   : Write one byte to a register
 * Returns       : 0 on success, -1 on failure
 * Params        @i2c_bus: file descriptor of i2c bus
 *               @reg_addr: register's address (hex)
 *               @data: data byte (hex) to be written on register
 ****************************************************************/
extern int8_t I2C_Write (int i2c_bus, BYTE reg_addr, BYTE data);

/****************************************************************
 * Function Name : I2C_ReadRegisters
 * Description   : Read data from multiple registers
 * Returns       : 0 on success, -1 on failure
 * Params        @i2c_bus: file descriptor of i2c bus
 *               @start_reg_addr: start register's address (hex)
 *               @p_data: to store the read data from register
 *               @num_of_bytes: the number of bytes to be read
 ****************************************************************/
extern int8_t I2C_ReadRegisters (int i2c_bus, BYTE *p_data, uint8_t num_of_bytes);

/****************************************************************
 * Function Name : I2C_WriteRegisters
 * Description   : Write to multiple registers
 * Returns       : number of written byte on sucess, -1 on failure
 * Params        @i2c_bus: file descriptor of i2c bus
 *               @p_data: a pointer to the data buffer
 *               @num_of_bytes: the number of bytes to be written
 ****************************************************************/
extern int8_t I2C_WriteRegisters (int i2c_bus, BYTE *p_data, uint8_t num_of_bytes);

/****************************************************************
 * Function Name : I2C_Close
 * Description   : Close the I2C bus
 * Returns       : 0 on success, -1 on failure
 * Params        : @i2c_bus: file descriptor
 ****************************************************************/
extern int8_t I2C_Close (int i2c_bus);

#endif
