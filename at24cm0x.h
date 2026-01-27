/**
 * @file at24cm0x.h
 * @brief Header file with declarations and macros for driving an at24cm0x.
 * 
 * This file provides function prototypes, type definitions, and constants for communication with an at24cm0x eeprom chip.
 * 
 * @author g.raf
 * @date 2026-01-24
 * @version 1.0 Release
 * @copyright
 * Copyright (c) 2026 g.raf
 * Released under the GPLv3 License. (see LICENSE in repository)
 * 
 * @note This file is part of a larger project and subject to the license specified in the repository. For updates and the complete revision history, see the GitHub repository.
 * 
 * @see https://github.com/0x007e/driver-prom-at24cm0x "AT24CM0X eeprom driver library"
 */

#ifndef AT24CM0X_H_
#define AT24CM0X_H_
	
	#ifndef AT24CM0X_HAL_PLATFORM
		/**
		 * @def AT24CM0X_HAL_PLATFORM
		 * @brief Sets the target platform for the AT24CM0X hardware abstraction layer (HAL), e.g., avr or avr0
		 * 
		 * @details
		 * Define this macro with the name of the target platform to select the corresponding platform-specific HAL implementation (such as TWI communication functions) for the AT24CM0X EEPROM driver. Common values are avr (classic AVR architecture) or avr0 (AVR0/1 series).
		 * 
		 * @note Set this macro as a global compiler symbol to ensure that the correct HAL implementation is used across the entire project.
		*/
		#define AT24CM0X_HAL_PLATFORM avr0
	#endif

	#ifndef AT24CM0X_MEMORY_SIZE
		/** 
		 * @def AT24CM0X_MEMORY_SIZE
		 * @brief Total AT24CM0X EEPROM memory size in bytes.
		 *
		 *  This macro defines the overall storage capacity of the AT24CM0X device and is used for address calculations and for validating the accessible address range for read and write operations within the driver.
		 *
		 * @note The default value `262144UL` corresponds to a 2 Mbit (256 KB) EEPROM. Adjust this value to match the actual device variant in use (for example, 131072UL for 1 Mbit).
		 */
		#define AT24CM0X_MEMORY_SIZE 262144UL
	#endif

	#ifndef AT24CM0X_PAGES
		/** 
		 * @def AT24CM0X_PAGES
		 * @brief Number of memory pages of the AT24CM0X EEPROM device.
		 *
		 *  This macro defines how many pages the EEPROM memory is divided into. It is used for page-based addressing and validation in page write and page read operations within the driver.
		 *
		 *  @note The default value `1024UL` corresponds to 1024 pages in total. Make sure this value matches the actual page layout of the selected device.
		 */
		#define AT24CM0X_PAGES 1024UL
	#endif
	
	#ifndef AT24CM0X_PAGE_SIZE
		/** 
		 * @def AT24CM0X_PAGE_SIZE
		 * @brief Size of a single EEPROM page in bytes.
		 *
		 *  This macro defines how many bytes are contained in one memory page of the AT24CM0X device. It is used for page boundary calculations and for validating buffer sizes in page write operations.
		 *
		 * @note The default value `256UL` corresponds to 256 bytes per page. Adjust this value if a device with a different page size is used.
		 */
		#define AT24CM0X_PAGE_SIZE 256UL
	#endif
	
	#ifndef AT24CM0X_ENABLE_INTEGRITY_CHECK
		/** 
		 * @def AT24CM0X_ENABLE_INTEGRITY_CHECK
		 * @brief Enables additional data integrity checks for EEPROM transfers.
		 *
		 *  When this macro is defined, the driver can perform extra verification steps (such as consistency or bounds checks) on read and write operations to improve robustness.
		 *
		 * @note By default this macro is commented out, meaning integrity checks are disabled to minimize overhead. Uncomment or define it as a global compiler symbol to enable integrity checking in the driver.
		 */
		//#define AT24CM0X_ENABLE_INTEGRITY_CHECK

		#ifdef _DOXYGEN_    // Used for documentation, can be ignored
            #define AT24CM0X_ENABLE_INTEGRITY_CHECK
        #endif
	#endif
	
	#ifndef AT24CM0X_WRITE_ACKNOWLEDGE_POLLING
		/** 
		 * @def AT24CM0X_WRITE_ACKNOWLEDGE_POLLING
		 * @brief Enables write acknowledge polling after EEPROM write operations.
		 *
		 *  When this macro is defined, the driver performs acknowledge polling after a write operation to the AT24CM0X device, repeatedly checking for the device's ACK to determine when the internal write cycle has completed. This can reduce overall write latency by allowing the driver to proceed as soon as the device has completed.
		 *
		 *  @note By default this macro is commented out, so acknowledge polling is disabled and the driver must rely on fixed write cycle delays. Uncomment or define it as a global compiler symbol to enable acknowledge polling.
		 */
		//#define AT24CM0X_WRITE_ACKNOWLEDGE_POLLING

		#ifdef _DOXYGEN_    // Used for documentation, can be ignored
            #define AT24CM0X_WRITE_ACKNOWLEDGE_POLLING
        #endif
	#endif
	
	/** 
	 * @def AT24CM0X_BASE_ADDRESS
	 * @brief Base I2C address of the AT24CM0X EEPROM device.
	 *
	 *  This macro defines the 7-bit base address used by the AT24CM0X on the I2C bus before applying any address pin or bank-dependent bits.
	 *
	 *  @note The default value `0x50` is commonly used for I2C EEPROM devices.
	 */
	#ifndef AT24CM0X_BASE_ADDRESS
		#define AT24CM0X_BASE_ADDRESS 0x50

		/**
		 * @def AT24CM0X_ADDRESS_MASK
		 * @brief Mask for hardware address pin bits within the I2C address.
		 *
		 *  This macro defines the bit mask used to apply or extract the hardware address pin bits (such as A2/A1) to/from the AT24CM0X I2C address. The mask depends on the configured memory size.
		 *
		 * @note For devices smaller than 2 Mbit (`AT24CM0X_MEMORY_SIZE < 262144UL`), the value `0x06` covers the A2 and A1 bits. For 2 Mbit devices and above, the value `0x04` only covers the A2 bit.
		 */
		#ifndef AT24CM0X_ADDRESS_MASK
			#if AT24CM0X_MEMORY_SIZE < 262144UL
				#define AT24CM0X_ADDRESS_MASK 0x06
			#else
				#define AT24CM0X_ADDRESS_MASK 0x04
			#endif
		#endif

		/**
		 * @def AT24CM0X_ADDRESS_HIGH_MASK
		 * @brief Mask for high-order address bits used for bank selection.
		 *
		 *  This macro defines the bit mask used to encode or decode high-order memory address bits that are mapped into the I2C address for larger AT24CM0X devices. The mask depends on the configured memory size.
		 *
		 * @note For devices smaller than 2 Mbit (`AT24CM0X_MEMORY_SIZE < 262144UL`),
		 *  the value `0x01` enables a single high-order address bit. For 2 Mbit devices and above, the value `0x03` enables two high-order address bits for multiple banks.
		 */
		#ifndef AT24CM0X_ADDRESS_HIGH_MASK
			#if AT24CM0X_MEMORY_SIZE < 262144UL
				#define AT24CM0X_ADDRESS_HIGH_MASK 0x01
			#else
				#define AT24CM0X_ADDRESS_HIGH_MASK 0x03
			#endif
		#endif
	#endif
	
	#ifndef AT24CM0X_MULTI_DEVICES
		/** @def AT24CM0X_MULTI_DEVICES
		 *  @brief Enables support for multiple AT24CM0X devices on the I2C bus.
		 *
		 *  When this macro is defined, the driver provides mechanisms to select and address more than one AT24CM0X device, distinguished by their I2C address or identifier.
		 *
		 * @note By default this macro is commented out, meaning the driver operates with a single EEPROM device instance. Uncomment or define it as a global compiler symbol to enable multi-device support.
		 */
		//#define AT24CM0X_MULTI_DEVICES

		#ifdef _DOXYGEN_    // Used for documentation, can be ignored
            #define AT24CM0X_MULTI_DEVICES
        #endif
	#endif
	
	#ifndef AT24CM0X_MULTI_DEVICES
		/** 
		 * @def AT24CM0X_ADDRESS_A2
		 * @brief Logical level of the A2 hardware address pin (0 or 1).
		 *
		 *  This macro defines the value of the A2 pin that is encoded into the AT24CM0X I2C address when multi-device support is disabled.
		 *
		 * @note The default value `1` assumes the A2 pin is tied high. Adjust this value to match the actual hardware wiring of the A2 pin.
		 */
		#ifndef AT24CM0X_ADDRESS_A2
			#define AT24CM0X_ADDRESS_A2 1
		#endif

		#ifndef AT24CM0X_ADDRESS

			/** 
			 * @def AT24CM0X_ADDRESS_A1
			 * @brief Logical level of the A1 hardware address pin (0 or 1).
			 *
			 *  This macro defines the value of the A1 pin that is encoded into the AT24CM0X I2C address for devices smaller than 2 Mbit (`AT24CM0X_MEMORY_SIZE < 262144UL`).
			 *
			 * @note The default value `1` assumes the A1 pin is tied high. Adjust this value to match the actual hardware wiring of the A1 pin.
			 */
			#if AT24CM0X_MEMORY_SIZE < 262144UL
				#ifndef AT24CM0X_ADDRESS_A1
					#define AT24CM0X_ADDRESS_A1 1
				#endif

				/** 
				 * @def AT24CM0X_ADDRESS
				 * @brief I2C address of the single AT24CM0X device instance.
				 *
				 *  This macro computes the 7-bit I2C address for the EEPROM from the base address and the configured A2/A1 hardware address bits when multi-device support is disabled.
				 *
				 * @note For devices smaller than 2 Mbit, both A2 and A1 are used: `(AT24CM0X_BASE_ADDRESS | (AT24CM0X_ADDRESS_A2 << 2) | (AT24CM0X_ADDRESS_A1 << 1))`.
				 */
				#define AT24CM0X_ADDRESS (AT24CM0X_BASE_ADDRESS | (AT24CM0X_ADDRESS_A2<<2) | (AT24CM0X_ADDRESS_A1<<1))
			#else
				/** 
				 * @def AT24CM0X_ADDRESS
				 * @brief I2C address of the single AT24CM0X device instance.
				 *
				 *  This macro computes the 7-bit I2C address for the EEPROM from the base address and the configured A2 hardware address bit for 2 Mbit devices and above.
				 *
				 * @note For these devices only A2 is used: `(AT24CM0X_BASE_ADDRESS | (AT24CM0X_ADDRESS_A2 << 2))`.
				 */
				#define AT24CM0X_ADDRESS (AT24CM0X_BASE_ADDRESS | (AT24CM0X_ADDRESS_A2<<2))
			#endif

		#endif
	#endif
	
	#ifndef AT24CM0X_WP_CONTROL_EN
		/** 
		 * @def AT24CM0X_WP_CONTROL_EN
		 * @brief Enables software control of the EEPROM write-protect (WP) feature.
		 *
		 *  When this macro is defined, the driver provides an interface to control the WP state via @ref at24cm0x_wp, so that write access to the device can be enabled or disabled in software.
		 *
		 * @note By default this macro is disabled, in cause of WP pin is permanently tied to a fixed level in hardware.
		 */
		//#define AT24CM0X_WP_CONTROL_EN

		#ifdef _DOXYGEN_    // Used for documentation, can be ignored
			#define AT24CM0X_WP_CONTROL_EN
		#endif
	#endif
	
	#ifndef AT24CM0X_WRITE_CYCLE_MS
		/**
		 * @def AT24CM0X_WRITE_CYCLE_MS
		 * @brief Write cycle time of the AT24CM0X device in milliseconds.
		 *
		 *  This macro defines the typical time required by the EEPROM to complete an internal write cycle after a write operation has been issued.
		 *
		 * @note The default value `10UL` represents a 10 ms write cycle time, which is commonly specified for AT24-series EEPROMs. Adjust this value if the datasheet of the specific device variant indicates a different timing.
		 */
		#define AT24CM0X_WRITE_CYCLE_MS 10UL
	#endif
	
	#include "../../../utils/macros/stringify.h"

	#include _STR(../../../hal/AT24CM0X_HAL_PLATFORM/twi/twi.h)
	
	#include "../../../utils/systick/systick.h"

	#ifdef AT24CM0X_WP_CONTROL_EN
		/**
		 * @enum AT24CM0X_WP_Mode_t
		 * @brief Specifies the write-protect mode for the AT24CM0X device.
		 *
		 * @details
		 * This enumeration defines the possible states of the EEPROM write-protect (WP) control as used by the AT24CM0X driver.
		 */
		enum AT24CM0X_WP_Mode_t
		{
			AT24CM0X_WP_Mode_Enabled = 0,  /**< Write-protect enabled, write access is blocked. */
			AT24CM0X_WP_Mode_Disabled      /**< Write-protect disabled, write access is allowed. */
		};
		/**
		 * @typedef AT24CM0X_WP_Mode
		 * @brief Alias for enum AT24CM0X_WP_Mode_t representing WP control modes.
		 */
		typedef enum AT24CM0X_WP_Mode_t AT24CM0X_WP_Mode;
	#endif
	
	/**
	 * @enum AT24CM0X_Status_t
	 * @brief Status codes returned by AT24CM0X driver operations.
	 *
	 * @details
	 * This enumeration defines the possible result codes for EEPROM operations such as initialization, read, and write functions in the AT24CM0X driver.
	 */
	enum AT24CM0X_Status_t
	{
		AT24CM0X_Status_Done = 0,        /**< Operation completed successfully. */
		AT24CM0X_Status_Address_Error,   /**< Invalid or out-of-range memory address. */
		AT24CM0X_Status_Page_Error,      /**< Invalid page number or page boundary issue. */
		AT24CM0X_Status_Size_Error,      /**< Invalid data length or size parameter. */
		AT24CM0X_Status_Data_Error,      /**< Data-related error, such as null pointer or corrupt data. */
		AT24CM0X_Status_TWI_Error,       /**< Error in the underlying TWI/I2C communication. */
		AT24CM0X_Status_General_Error    /**< Unspecified or unexpected general error. */
	};
	/**
	 * @typedef AT24CM0X_Status
	 * @brief Alias for enum AT24CM0X_Status_t representing driver status codes.
	 */
	typedef enum AT24CM0X_Status_t AT24CM0X_Status;
	
	           void at24cm0x_init(void);
	
	#ifdef AT24CM0X_MULTI_DEVICES
		       void at24cm0x_device(unsigned char identifier);
	#endif
	
	#ifdef AT24CM0X_WP_CONTROL_EN
		       void at24cm0x_wp(AT24CM0X_WP_Mode mode);
	#endif
	
	AT24CM0X_Status at24cm0x_write_byte(unsigned long address, unsigned char data);
	AT24CM0X_Status at24cm0x_write_page(unsigned int page, unsigned char *data, unsigned int size);
	AT24CM0X_Status at24cm0x_read_current_byte(unsigned char *data);
	AT24CM0X_Status at24cm0x_read_byte(unsigned long address, unsigned char *data);
	AT24CM0X_Status at24cm0x_read_sequential(unsigned long address, unsigned char *data, unsigned int size);

#endif /* AT24CM0X_H_ */