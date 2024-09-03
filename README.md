
### Pins
Servo PIN: 4
Motor forward PIN: 8
Motor backward PIN: 10
Motor PWM PIN: 11
ESP RX PIN: 2
ESP TX PIN: 3

### Hardware
- HX-2S-D2O
- Arduino Nano
- ESP8266
- L298N
- GY-521
- SG90
- x2 18650 batteries

### ESP07 connection




### Measured battery values 
19.08.2024: 6,43V
26.08.2024: 6,43V #1 2,967V #2 3,452V
26.08.2024: 7,28V #1 3,624V #2 3,642V

### Aufladen-Log
26.08.2024 19:33 - 8,0V 0,5A
26.08.2024 19:39 - 8,0V 0,4A
26.08.2024 19:43 - 8,3V 0,5A (added +0,3V manually)
26.08.2024 19:49 - 8,1V 0,4A
26.08.2024 19:52 - 8,1V 0,4A
26.08.2024 20:13 - 8,1V 0,3A
26.08.2024 20:34 - 8,2V 0,4A


## About power supply (HX-2S-D2O)
Charging Voltage: 7,4V - 8,4V
Tiefentladung Spannung: 2,5V - 3,0V ± 0,05V

### Rust libraries
- avl-hal
    - [github link](https://github.com/Rahix/avr-hal?tab=readme-ov-file)

### Software for circuit modelling
- https://falstad.com/circuit/
- Arduino simulator
   - https://wokwi.com/arduino

### Sources
- Article about a watchdog timer
   - https://arduinodiy.wordpress.com/2015/06/22/flashing-an-led-with-attiny13/
- 


## Build Instructions
1. Install prerequisites as described in the [`avr-hal` README] (`avr-gcc`, `avr-libc`, `avrdude`, [`ravedude`]).

2. Run `cargo build` to build the firmware.

3. Run `cargo run` to flash the firmware to a connected board.  If `ravedude`
   fails to detect your board, check its documentation at
   <https://crates.io/crates/ravedude>.

4. `ravedude` will open a console session after flashing where you can interact
   with the UART console of your board.

[`avr-hal` README]: https://github.com/Rahix/avr-hal#readme
[`ravedude`]: https://crates.io/crates/ravedude

## License
Licensed under either of

 - Apache License, Version 2.0
   ([LICENSE-APACHE](LICENSE-APACHE) or <http://www.apache.org/licenses/LICENSE-2.0>)
 - MIT license
   ([LICENSE-MIT](LICENSE-MIT) or <http://opensource.org/licenses/MIT>)

at your option.

## Known issues
1. If you get `Error: programmer is not responding` error, try to change `.cargo/config.toml`:
   - Ensure you are using correct USB port: e.g. `-P /dev/cu.usbserial-1410`
   - You can try `nano` instead of `nano-new`
   - You can try to use another baud rate `115200` instead of `57600`

## Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, as defined in the Apache-2.0 license, shall
be dual licensed as above, without any additional terms or conditions.