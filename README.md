[![Version: 1.0 Release](https://img.shields.io/badge/Version-1.0%20Release-green.svg)](https://github.com/0x007e/drivers-prom-at24cm0x) ![Build](https://github.com/0x007e/drivers-prom-at24cm0x/actions/workflows/release.yml/badge.svg) [![License GPLv3](https://img.shields.io/badge/License-GPLv3-lightgrey)](https://www.gnu.org/licenses/gpl-3.0.html)

# `AT24CM0X EEPROM Driver`

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/0x007E/drivers-prom-at24cm0x)

This hardware abstracted driver can be used to interact with an [AT24CM0X](#additional-information) over `TWI`/`I2C`. The hardware layer is fully abstract an can be switched between different plattforms. The `TWI`/`I2C` library has to impelement the [twi.h](https://0x007e.github.io/drivers-prom-at24cm0x/twi_8c.html)-header used in this repository.

## File Structure

![File Structure](https://0x007e.github.io/drivers-prom-at24cm0x/at24cm0x_8c__incl.png)

```
drivers/
└── prom/
    └── at24cm0x/
        ├── at24cm0x.c
        └── at24cm0x.h

hal/
├── common/
|   ├── defines/
|   |   └── TWI_defines.h
|   └── enums/
|       └── TWI_enums.h
└── avr0/
    └── twi/
        ├── twi.c
        └── twi.h

utils/
├── macros/
|   └── stringify.h
└── systick/
    ├── systick.c
    └── systick.h
```

> The plattform `avr0` can completely be exchanged with any other hardware abstraction library.

## Downloads

The library can be downloaded (`zip` or `tar`), cloned or used as submodule in a project.

| Type      | File               | Description              |
|:---------:|:------------------:|:-------------------------|
| Library   | [zip](https://github.com/0x007E/drivers-prom-at24cm0x/releases/latest/download/library.zip) / [tar](https://github.com/0x007E/drivers-prom-at24cm0x/releases/latest/download/library.tar.gz) | AT24CM0X eeprom library including all required libraries (including `hal-avr0-twi`). |

### Using with `git clone`

```sh
mkdir -p ./drivers/prom/
git clone https://github.com/0x007E/drivers-prom-at24cm0x.git ./drivers/prom
mv ./drivers/prom/drivers-prom-at24cm0x ./drivers/prom/at24cm0x

mkdir -p ./hal/
git clone https://github.com/0x007E/hal-common.git ./hal
mv ./hal/hal-common ./hal/common

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# Hardware abstraction layer of TWI (Must fit the used plattform)
mkdir -p ./hal/avr0/
git clone https://github.com/0x007E/hal-avr0-twi.git ./hal/avr0
mv ./hal/avr0/hal-avr0-twi ./hal/avr0/twi
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

mkdir -p ./utils/
git clone https://github.com/0x007E/utils-macros.git  ./utils
git clone https://github.com/0x007E/utils-systick.git ./utils
mv ./utils/utils-macros  ./utils/macros
mv ./utils/utils-systick ./utils/systick
```

### Using as `git submodule`

```sh
git submodule add https://github.com/0x007E/drivers-prom-at24cm0x.git drivers/prom/at24cm0x
git submodule add https://github.com/0x007E/hal-common.git            hal/common

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# Hardware abstraction layer of TWI (Must fit the used plattform)
git submodule add https://github.com/0x007E/hal-avr0-twi.git          hal/avr0/twi
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

git submodule add https://github.com/0x007E/utils-macros.git          utils/macros
git submodule add https://github.com/0x007E/utils-systick.git         utils/systick
```

## Programming

```c
#include "../hal/PLATFORM/twi/twi.h"

#include "../lib/drivers/prom/at24cm0x/at24cm0x.h"
#include "../lib/utils/systick/systick.h"

// Called every ~ milli-/microsecond!
ISR(...)
{
	systick_tick();
}

void systick_timer_wait_ms(unsigned int ms)
{
	systick_timer_wait(ms);
}

// Only necessary if AT24CM0X_WP_CONTROL_EN define is set!
// If the EEPROM WP Pin is connectet to the microcontroller the
// EEPROM library can enable/disable the WP pin when necessary.
// This prevents the EEPROM to be written accidentally through
// the twi bus.
void at24cm0x_wp(AT24CM0X_WP_Mode mode)
{
	if(mode)
	{
		// Set WP_PIN low (write protect disabled)
		return;
	}
	// Set WP_PIN high (write protect enabled)
}

int main(void)
{
	systick_init();
	twi_init();

    at24cm0x_init();

    // If more AT24CM0X devices share the same bus the AT24CM0X_MULTI_DEVICES
    // define can be enabled. This enables the possibility to select different
    // AT24CM0X devices on the bus!
    at24cm0x_device(AT24CM0X_BASE_ADDRESS | DEVICE_A_PINS);
    
    if(at24cm0x_write_byte(0x00000000, 0x01) == AT24CM0X_Status_Done)
    {
        // Byte written!
    }

    char buffer[] = "Sample text into buffer";

    if(at24cm0x_write_page(0x00000000UL, (unsigned char *)buffer, sizeof(buffer)/sizeof(buffer[0]) == AT24CM0X_Status_Done))
    {
        // Buffer written!
    }

    if(at24cm0x_read_sequential(0x00000000UL, (unsigned char*)buffer, sizeof(buffer)/sizeof(buffer[0])) == AT24CM0X_Status_Done)
    {
        // Output -> buffer data
    }

    unsigned char temp = '\0';

    if(at24cm0x_read_byte(0x00000000UL, &temp) == AT24CM0X_Status_Done)
    {
        // Output -> single byte
    }

    if(at24cm0x_read_current_byte(&temp) == AT24CM0X_Status_Done)
    {
        // Output -> current address
    }
}
```

# Additional Information

| Type       | Link               | Description              |
|:----------:|:------------------:|:-------------------------|
| AT24CM01 | [pdf](https://ww1.microchip.com/downloads/en/DeviceDoc/AT24CM01-I2C-Compatible-Two-Wire-Serial-EEPROM-Data-Sheet-20006170A.pdf) | AT24CM01 TWI/I2C EEPROM |
| AT24CM02 | [pdf](https://ww1.microchip.com/downloads/en/DeviceDoc/AT24CM02-I2C-Compatible-Two-Wire-Serial-EEPROM-2-Mbit-262,144x8-20006197C.pdf) | AT24CM02 TWI/I2C EEPROM |

---

R. GAECHTER