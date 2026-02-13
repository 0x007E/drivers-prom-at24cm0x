/**
 * @file at24cm0x.c
 *
 * @brief Implementation of the AT24CM0X driver functions.
 *
 * This file contains the implementation of functions to initialize the AT24CM0X device, write and read data from/to eeprom. It utilizes TWI/I2C communication to interact with the AT24CM0X hardware.
 *
 * @author g.raf
 * @date 2026-01-25
 * @version 1.0 Release
 * @copyright
 * Copyright (c) 2026 g.raf
 * Released under the GPLv3 License. (see LICENSE in repository)
 * 
 * @note This file is part of a larger project and subject to the license specified in the repository. For updates and the complete revision history, see the GitHub repository.
 * 
 * @see https://github.com/0x007e/drivers-prom-at24cm0x "AT24CM0X eeprom driver library"
 */

#include "at24cm0x.h"

volatile unsigned char at24cm0x_device_identifier;

/**
 * @brief Initializes the AT24CM0X EEPROM driver.
 * 
 * @details
 * This function initializes the internal configuration of the AT24CM0X driver. If write-protect control is enabled at compile time (`AT24CM0X_WP_CONTROL_EN`), it first activates write-protect by calling @ref at24cm0x_wp with @ref AT24CM0X_WP_Mode_Enabled. It then sets the internal device identifier used for I2C communication to either @ref AT24CM0X_BASE_ADDRESS when multi-device support is enabled (`AT24CM0X_MULTI_DEVICES`), or to @ref AT24CM0X_ADDRESS when operating in single-device mode.
 * 
 * @note This function should be called once during system startup before any AT24CM0X read or write operations are performed.
 */
void at24cm0x_init(void)
{
	#ifdef AT24CM0X_WP_CONTROL_EN
		at24cm0x_wp(AT24CM0X_WP_Mode_Enabled);
	#endif
	
	#ifdef AT24CM0X_MULTI_DEVICES
		at24cm0x_device_identifier = AT24CM0X_BASE_ADDRESS;
	#else
		at24cm0x_device_identifier = AT24CM0X_ADDRESS;
	#endif
}

#ifdef AT24CM0X_MULTI_DEVICES
	/**
	 * @brief Selects the active AT24CM0X device.
	 * 
	 * @details
	 * This function updates the internal device identifier used by the AT24CM0X driver to communicate with a specific EEPROM device on the I2C bus. The given identifier is masked with the configured @ref AT24CM0X_ADDRESS_MASK and combined with @ref AT24CM0X_BASE_ADDRESS to form the effective 7-bit I2C address stored in the internal `at24cm0x_device_identifier`.
	 * 
	 * @param identifier Device selector value used to derive the target AT24CM0X I2C address.
	 */
	void at24cm0x_device(unsigned char identifier)
	{
		at24cm0x_device_identifier = AT24CM0X_BASE_ADDRESS | (identifier & AT24CM0X_ADDRESS_MASK);
	}
#endif

#ifdef AT24CM0X_WRITE_ACKNOWLEDGE_POLLING
	static void at24cm0x_write_acknowledge_polling(void)
	{
		TWI_Error error = TWI_None;
		
		do
		{
			twi_start();
			error = twi_address(at24cm0x_device_identifier, TWI_Write);
		} while (error == TWI_Ack);
		twi_stop();
	}
#endif

static TWI_Error at24cm0x_send_address(unsigned long address)
{
	unsigned char high_byte = (at24cm0x_device_identifier | (AT24CM0X_ADDRESS_HIGH_MASK & (unsigned char)(address>>16)));
	unsigned char middle_byte = (unsigned char)(address>>8);
	unsigned char low_byte = (unsigned char)(address);
	
	TWI_Error error = TWI_None;
	
	error |= twi_address(high_byte, TWI_Write);
	error |= twi_set(middle_byte);
	error |= twi_set(low_byte);
	
	return error;
}

/**
 * @brief Writes a single byte to the AT24CM0X EEPROM.
 * 
 * @details
 * This function writes one byte of data to the specified EEPROM address of the AT24CM0X device. It first checks whether the given address is within the valid memory range defined by @ref AT24CM0X_MEMORY_SIZE; if not, it returns @ref AT24CM0X_Status_Address_Error. If write-protect control is enabled (@ref AT24CM0X_WP_CONTROL_EN), the function temporarily disables write-protect before issuing the write and re-enables it afterward. The write operation itself is performed using the TWI/I2C interface: a start condition is sent, the device/address sequence is transmitted via @ref at24cm0x_send_address, the data byte is written, and a stop condition is generated.
 * 
 * After the write, the function either performs write acknowledge polling (@ref AT24CM0X_WRITE_ACKNOWLEDGE_POLLING) or waits for a fixed write cycle time (@ref AT24CM0X_WRITE_CYCLE_MS), depending on the compile-time configuration. If any TWI communication error occurs, it returns @ref AT24CM0X_Status_TWI_Error.
 * 
 * When integrity checking is enabled (@ref AT24CM0X_ENABLE_INTEGRITY_CHECK), the function reads back the byte using @ref at24cm0x_read_byte and compares it with the original value; if they do not match, it returns @ref AT24CM0X_Status_Data_Error. On success, the function returns @ref AT24CM0X_Status_Done.
 * 
 * @param address EEPROM memory address at which the byte will be written.
 * @param data    Data byte to write to the specified address.
 * 
 * @return AT24CM0X_Status Status code indicating the result of the operation.
 */
AT24CM0X_Status at24cm0x_write_byte(unsigned long address, unsigned char data)
{
	TWI_Error error = TWI_None;
	
	if(address >= AT24CM0X_MEMORY_SIZE)
	{
		return AT24CM0X_Status_Address_Error;
	}
	
	#ifdef AT24CM0X_WP_CONTROL_EN
		at24cm0x_wp(AT24CM0X_WP_Mode_Disabled);
	#endif
	
	twi_start();
	error |= at24cm0x_send_address(address);
	error |= twi_set(data);
	twi_stop();
	
	#ifdef AT24CM0X_WRITE_ACKNOWLEDGE_POLLING
		at24cm0x_write_acknowledge_polling();
	#else
		systick_timer_wait_ms(AT24CM0X_WRITE_CYCLE_MS);
	#endif
	
	#ifdef AT24CM0X_WP_CONTROL_EN
		at24cm0x_wp(AT24CM0X_WP_Mode_Enabled);
	#endif
		
	if(error != TWI_None)
	{
		return AT24CM0X_Status_TWI_Error;
	}
	
	#ifdef AT24CM0X_ENABLE_INTEGRITY_CHECK
		unsigned char temp = 0;
		
		if(at24cm0x_read_byte(address, &temp) != AT24CM0X_Status_Done)
		{
			return AT24CM0X_Status_TWI_Error;
		}
		
		if(data != temp)
		{
			return AT24CM0X_Status_Data_Error;
		}
	#endif
	
	return AT24CM0X_Status_Done;
}

/**
 * @brief Writes a sequence of bytes to a single EEPROM page.
 * 
 * @details
 * This function writes a contiguous block of data to one page of the AT24CM0X EEPROM. It first validates the page index against @ref AT24CM0X_PAGES and the data length against @ref AT24CM0X_PAGE_SIZE; if the page is out of range, it returns @ref AT24CM0X_Status_Page_Error, and if the size is zero or not within the allowed page size, it returns @ref AT24CM0X_Status_Size_Error.
 * 
 * The target EEPROM address is calculated from the page index and @ref AT24CM0X_PAGE_SIZE. If write-protect control is enabled (@ref AT24CM0X_WP_CONTROL_EN), write-protect is temporarily disabled before the TWI/I2C transfer and re-enabled afterward. The function then issues a start condition, sends the device/address sequence via @ref at24cm0x_send_address, and transmits each byte from the provided buffer using @ref twi_set, followed by a stop condition.
 * 
 * After the write operation, the function either performs write acknowledge polling (@ref AT24CM0X_WRITE_ACKNOWLEDGE_POLLING) or waits for a fixed write cycle time (@ref AT24CM0X_WRITE_CYCLE_MS), depending on the compile-time configuration. If any TWI error is detected, it returns @ref AT24CM0X_Status_TWI_Error.
 * 
 * When integrity checking is enabled (@ref AT24CM0X_ENABLE_INTEGRITY_CHECK), the function reads back the written data using @ref at24cm0x_read_sequential into a temporary buffer and compares it byte-by-byte with the original data. If any mismatch is found, it returns @ref AT24CM0X_Status_Data_Error. On success, the function returns @ref AT24CM0X_Status_Done.
 * 
 * @param page Page index to write, in the range [0, @ref AT24CM0X_PAGES - 1].
 * @param data Pointer to the buffer containing the data to be written.
 * @param size Number of bytes to write; must be greater than 0 and less than @ref AT24CM0X_PAGE_SIZE.
 * 
 * @return AT24CM0X_Status Status code indicating the result of the operation.
 */
AT24CM0X_Status at24cm0x_write_page(unsigned int page, unsigned char *data, unsigned int size)
{
	TWI_Error error = TWI_None;

	if(page >= AT24CM0X_PAGES)
	{
		return AT24CM0X_Status_Page_Error;
	}
	
	if(size == 0 || size >= AT24CM0X_PAGE_SIZE)
	{
		return AT24CM0X_Status_Size_Error;
	}
	
	unsigned long address = (page * AT24CM0X_PAGE_SIZE);
	
	#ifdef AT24CM0X_WP_CONTROL_EN
		at24cm0x_wp(AT24CM0X_WP_Mode_Disabled);
	#endif
	
	twi_start();
	at24cm0x_send_address(address);
	
	for (unsigned int i=0; i < size; i++)
	{
		error |= twi_set(*(data + i));
	}
	twi_stop();
	
	#ifdef AT24CM0X_WRITE_ACKNOWLEDGE_POLLING
		at24cm0x_write_acknowledge_polling();
	#else
		systick_timer_wait_ms(AT24CM0X_WRITE_CYCLE_MS);
	#endif
	
	#ifdef AT24CM0X_WP_CONTROL_EN
		at24cm0x_wp(AT24CM0X_WP_Mode_Enabled);
	#endif
	
	#ifdef AT24CM0X_ENABLE_INTEGRITY_CHECK
		unsigned char buffer[AT24CM0X_PAGE_SIZE];
		
		if(at24cm0x_read_sequential(address, buffer, size) != AT24CM0X_Status_Done)
		{
			return AT24CM0X_Status_TWI_Error;
		}
		
		for (unsigned int i=0; i < size; i++)
		{
			if(buffer[i] != *(data + i))
			{
				return AT24CM0X_Status_Data_Error;
			}
		}
	#endif
	
	if(error != TWI_None)
	{
		return AT24CM0X_Status_TWI_Error;
	}
	return AT24CM0X_Status_Done;
}

/**
 * @brief Reads the current byte from the AT24CM0X EEPROM.
 * 
 * @details
 * This function reads the byte currently addressed by the AT24CM0X internal address counter using a TWI/I2C read operation. It sends a start condition, addresses the device in read mode using the internally stored `at24cm0x_device_identifier`, reads one byte into the provided buffer, and then issues a stop condition.
 * 
 * If any TWI communication error occurs during the sequence, the function returns @ref AT24CM0X_Status_TWI_Error. On success, the received byte is stored at the location pointed to by @p data and the function returns
 * @ref AT24CM0X_Status_Done.
 * 
 * @param data Pointer to a byte where the received data will be stored.
 * 
 * @return AT24CM0X_Status Status code indicating the result of the operation.
 */
AT24CM0X_Status at24cm0x_read_current_byte(unsigned char *data)
{
	TWI_Error error = TWI_None;
	
	twi_start();
	error |= twi_address(at24cm0x_device_identifier, TWI_Read);
	error |= twi_get(data, TWI_NACK);
	twi_stop();
	
	if(error != TWI_None)
	{
		return AT24CM0X_Status_TWI_Error;
	}
	return AT24CM0X_Status_Done;
}

/**
 * @brief Reads a single byte from the AT24CM0X EEPROM.
 * 
 * @details
 * This function reads one byte of data from the specified EEPROM address of the AT24CM0X device. It first checks whether the given address is within the valid memory range defined by @ref AT24CM0X_MEMORY_SIZE; if not, it returns @ref AT24CM0X_Status_Address_Error.
 * 
 * The read operation is performed using a random-read sequence: a start condition is sent, the target address is set via @ref at24cm0x_send_address, and a stop condition is issued. Then a repeated start is generated, the device is addressed in read mode using the current @ref at24cm0x_device_identifier, and a single byte is read with @ref twi_get, followed by a stop condition.
 * 
 * If any TWI communication error occurs, the function returns
 * @ref AT24CM0X_Status_TWI_Error. On success, the received byte is stored at the location pointed to by @p data and the function returns @ref AT24CM0X_Status_Done.
 * 
 * @param address EEPROM memory address from which the byte will be read.
 * @param data    Pointer to a byte where the received data will be stored.
 * 
 * @return AT24CM0X_Status Status code indicating the result of the operation.
 */
AT24CM0X_Status at24cm0x_read_byte(unsigned long address, unsigned char *data)
{
	if(address >= AT24CM0X_MEMORY_SIZE)
	{
		return AT24CM0X_Status_Address_Error;
	}
	
	AT24CM0X_Status status = AT24CM0X_Status_Done;
	TWI_Error error = TWI_None;
	
	twi_start();
	error |= at24cm0x_send_address(address);
	twi_stop();
	
	twi_start();
	error |= twi_address(at24cm0x_device_identifier, TWI_Read);
	error |= twi_get(data, TWI_NACK);
	twi_stop();
	
	if(error != TWI_None || status != AT24CM0X_Status_Done)
	{
		return AT24CM0X_Status_TWI_Error;
	}
	return AT24CM0X_Status_Done;
}

/**
 * @brief Reads a sequence of bytes from the AT24CM0X EEPROM.
 * 
 * @details
 * This function reads a contiguous block of data starting at the specified EEPROM address of the AT24CM0X device. It first verifies that the start address is within the valid memory range defined by @ref AT24CM0X_MEMORY_SIZE; if not, it returns @ref AT24CM0X_Status_Address_Error. If the requested size is zero, it returns @ref AT24CM0X_Status_Size_Error.
 * 
 * The read operation is performed using a random-read followed by a sequential read: a start condition is sent, the target address is set via @ref at24cm0x_send_address, and a stop condition is issued. Then a repeated start is generated, the device is addressed in read mode using the current @ref at24cm0x_device_identifier, and @p size bytes are read into the buffer pointed to by @p data. All bytes except the last are read with an ACK; the final byte is read with NACK to terminate the transfer, followed by a stop condition.
 * 
 * If any TWI communication error occurs, the function returns @ref AT24CM0X_Status_TWI_Error. On success, the received data block is stored in the provided buffer and the function returns @ref AT24CM0X_Status_Done.
 * 
 * @param address Start EEPROM memory address from which the data will be read.
 * @param data    Pointer to the buffer where the received data will be stored.
 * @param size    Number of bytes to read; must be greater than 0.
 * 
 * @return AT24CM0X_Status Status code indicating the result of the operation.
 */
AT24CM0X_Status at24cm0x_read_sequential(unsigned long address, unsigned char *data, unsigned int size)
{
	if(address >= AT24CM0X_MEMORY_SIZE)
	{
		return AT24CM0X_Status_Address_Error;
	}
	
	if(size == 0)
	{
		return AT24CM0X_Status_Size_Error;
	}
	
	AT24CM0X_Status status = AT24CM0X_Status_Done;
	TWI_Error error = TWI_None;
	
	twi_start();
	error |= at24cm0x_send_address(address);
	twi_stop();
	
	twi_start();
	error |= twi_address(at24cm0x_device_identifier, TWI_Read);
	
	for (unsigned int i=0; i < size; i++)
	{
		TWI_Operation ack = TWI_Ack;
		
		if(i >= (size - 1))
		{
			ack = TWI_NACK;
		}
		error |= twi_get((data + i), ack);
	}
	twi_stop();
	
	if(error != TWI_None || status != AT24CM0X_Status_Done)
	{
		return AT24CM0X_Status_TWI_Error;
	}
	return AT24CM0X_Status_Done;
}