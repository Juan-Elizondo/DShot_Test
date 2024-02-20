# Table of Contents
- [Table of Contents](#table-of-contents)
- [Library Functions](#library-functions)
- [Project Structure](#project-structure)
- [Bidirectional DShot](#bidirectional-dshot)
  - [DShot Peripheral](#dshot-peripheral)
  - [Implementation](#implementation)
    - [ESP32 Configuration](#esp32-configuration)
    - [DShot Encoder](#dshot-encoder)
    - [C++ wrapper](#c-wrapper)
- [Hardware Notes](#hardware-notes)


# Library Functions
- `DShot_ESC::DShot_ESC(const dshot_frequency_t _frequency, bool _bidirectional, gpio_num_t ESC_Pin, bool TXS_Buffer, ESCData& _ESC_data)` - creates a DShot_ESC object with the specified configuration.
  - `_frequency` - DShot frequency (Bitrate) of enumerated type `dshot_frequency_t`
  - `_bidirectional` - `bool` for running bidirectional (inverted) DShot
  - `ESC_Pin` - `gpio_num_t` type for the pin to be used for the peripheral
  - `TXS_Buffer` - `bool` for using timings accounting for hardware buffer. See [Hardware Notes](#hardware-notes).
  - `_ESC_data` - `ESCData` reference to save data into
- `void DShot_ESC::Arm_ESC(void)` - sends the zero throttle command in an infinite loop. This function is called by the constructor.
- `void DShot_ESC::Throttle_Write(uint16_t throttle)` - sends a single DShot command over the wire. If in bidirectional mode, a callback function will return the received data into the `_ESC_data` object.

# Project Structure
This DShot implementation heavily relies on the [Remote Control Transceiver (RMT)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/rmt.html) peripheral to maintain accuracy in the DShot timings. As such, the implementation requires the use of the Espressif Framework. The Arduino framework is not supported.

This library is written using PlatformIO for the ESP32 Dev Module.\
Build settings, monitor settings, and upload settings can be found in the [platformio.ini](platformio.ini) file in the root directory of the project.\
Additional documentation on available settings: [PlatformIO - Espressif ESP32 Dev Module](https://docs.platformio.org/en/latest/boards/espressif32/esp32dev.html).

The project configuration is stored in the [sdkconfig.esp32dev](sdkconfig.esp32dev) file in the root directory. The project configuration can be edited by running `pio run -t menuconfig` from the PlatformIO Core CLI.

The example code is run from [./src/main.cpp](./src/main.cpp), which emulates Arduino behavior by runnign a setup and loop function.\
The main functions are defined within [./lib/Main_Test](./lib/Main_Test).
- The setup function is not used.
- The loop function implements a timer to send a DShot command at 1ms intervals.
  
The DShot peripheral is implemented in the files within [./.lib/DShot](./lib/DShot)

# Bidirectional DShot
## DShot Peripheral
This implementation referenced [DSHOT-the missing Handbook](https://brushlesswhoop.com/dshot-and-bidirectional-dshot/#bidirectional-dshot).\
A quick comment on the DShot timing table: it is easier for me to understand that a bit of value 1 consists of a pulse width of $0.75 \over Bitrate$. For a value of 0, the pulse width is of $0.375 \over Bitrate$.

## Implementation
### ESP32 Configuration
TODO
### DShot Encoder
TODO
### C++ wrapper
TODO

# Hardware Notes

I used a bidirectional buffer ([TXS0101DRL](https://www.ti.com/lit/ds/symlink/txs0101.pdf)) to be able to levelshift the DShot wire up to 5V and interface with other components. Unfortunately, the architecture of the buffer causes the edges of the DShot signal to get skewed, which throws off the timings when transmitting out DShot commands.\
To address the output signal, I tweeked the output pulse width timings. This can be toggled with the `TXS_Buffer` input to the constructor. As far as I can tell, there is no need to implement anything for the input signal.