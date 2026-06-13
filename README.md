Arduino library for writing to V.A.G. FIS / DIS over 3lb line from radio

This is enhanced version of https://github.com/arildlangseid/vw_t4_tcu_temp_to_fis

For supported function check <a href="https://github.com/tomaskovacik/VAGFISWriter/wiki/Supported-functions">this wiki page</a>

credits: https://github.com/arildlangseid/vw_t4_tcu_temp_to_fis

For fullscreen support navigation mode need to be activated in cluster coding: http://wiki.ross-tech.com/wiki/index.php/Audi_A4_(8D)_Instrument_Cluster

for coverting image to array I use this tool: https://www.digole.com/tools/PicturetoC_Hex_converter.php


width = 64, height=88 

## Hardware and Software SPI Support

The library now **automatically detects** whether you're using hardware SPI pins or not:

### Hardware SPI (Recommended - Faster)
When you use the default hardware SPI pins for your board, the library automatically uses hardware SPI for optimal performance (~125 kHz):

**Arduino Uno/Nano:**
- CLK = pin 13 (SCK)
- DATA = pin 11 (MOSI)

**Arduino Mega:**
- CLK = pin 52 (SCK)
- DATA = pin 51 (MOSI)

**STM32:**
- CLK = PA5 or PB3 (SCK)
- DATA = PA7 or PB5 (MOSI)

**ESP32:**
- CLK = pin 18 (default VSPI SCK)
- DATA = pin 23 (default VSPI MOSI)

### Software SPI (Flexible - Any Pins)
When you use **any other digital pins**, the library automatically falls back to software SPI:
- Works with **any digital pins** as CLK and DATA
- Slower than hardware SPI due to digitalWrite overhead
- Useful when hardware SPI pins are already in use by other devices
- See `examples/software_spi_demo` for usage

### Manual Override (Advanced)
If auto-detection doesn't work for your board, you can manually force the SPI mode by defining one of these **BEFORE** including the library:

```cpp
// Force hardware SPI (for STM32 alternate pins, ESP32 remapped pins, etc.)
#define VAGFISWRITER_FORCE_HARDWARE_SPI
#include "VAGFISWriter.h"

// OR force software SPI
#define VAGFISWRITER_FORCE_SOFTWARE_SPI
#include "VAGFISWriter.h"
```

**⚠️ Important Safety Feature:**
When using `VAGFISWRITER_FORCE_HARDWARE_SPI`, the library validates that your specified pins actually match hardware SPI pins:
- ✅ **Pins match** (e.g., 13 & 11 on Uno) → Uses hardware SPI
- ❌ **Pins don't match** (e.g., 12 & 9 on Uno) → Falls back to software SPI with compile warning

This prevents silent failures where you force hardware SPI but use wrong pins.

**Example of the safety check:**
```cpp
#define VAGFISWRITER_FORCE_HARDWARE_SPI
#include "VAGFISWriter.h"

// WRONG PINS - will trigger warning and use software SPI
VAGFISWriter fisWriter(12, 9, 2);  
// Warning: "VAGFISWRITER_FORCE_HARDWARE_SPI specified but pins don't 
//           match hardware SPI pins. Falling back to software SPI."

// CORRECT PINS - will use hardware SPI
VAGFISWriter fisWriter(13, 11, 2);  // ✓ Uses hardware SPI
```

**Use cases for manual override:**
- STM32 with SPI2 or alternate pins (PB13/PB15)
- ESP32 with remapped SPI pins
- Custom boards where macros aren't defined correctly
- When you know the pins are hardware-capable but auto-detection fails

See `examples/stm32_force_hardware_spi` for a complete example.

**Example:**
```cpp
// Hardware SPI (automatic detection)
VAGFISWriter fisWriter(13, 11, 2);  // Uses hardware SPI on Uno/Nano

// Software SPI (automatic detection)
VAGFISWriter fisWriter(10, 9, 2);   // Uses software SPI (any pins)

// Check which mode is active
void setup() {
  fisWriter.begin();
  if (fisWriter.isUsingHardwareSPI()) {
    Serial.println("Hardware SPI active");
  } else {
    Serial.println("Software SPI active");
  }
}
```

### ENA (Enable) Pin
- Can be any digital pin
- For best performance with mode 0 (default), use a pin with hardware interrupt capability:
  - Arduino Uno/Nano: pins 2, 3
  - Arduino Mega: pins 2, 3, 18, 19, 20, 21
  - ESP32: any GPIO pin
- Alternatively, use force mode (4th parameter = 1) with any pin


videos of VAGFISWriter in action:

[VAGFISWriter 1.1](https://odysee.com/@tomaskovacik:9/vagfiswriter-1-1:8?r=Hhrweg5Vp56SqqmmcDxn3QPq9fvHPz8H)

[VAGFISWriter 1.1 speed test](https://odysee.com/@tomaskovacik:9/vagfiswriter-1-1-speed-test:a?r=Hhrweg5Vp56SqqmmcDxn3QPq9fvHPz8H)


