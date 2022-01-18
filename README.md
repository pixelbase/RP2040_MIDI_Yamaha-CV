# RP2040_MIDI_Yamaha-CV

 RP2040 MIDI to Yamaha-CV is C++ code for Raspberry Pi Pico built for
 the official Arduino core "Arduino Mbed OS RP2040 Boards".
 It implements a MIDI to CV/trigger (gate) converter for the Yamaha-CS15
 analog synth and should work for similar Yamaha-synthesizers.

 It has N modes which are (not yet) selected with a pushbutton.
 MIDI channel select is (not yet) performed with another pushbutton.
 Since there is no EEPROM these are reset every power-on.

I use 50 kHz PWM frequency, tested with 2 chained passive LP-filters per VCO
 each using a 100 Ohm resistor and a 100 uF capacitor.
 higher frequency results in lower bit-depth, lower frequency
 results in unstable voltage or needs more filtering (slowe rise-time)

## Libraries used
https://github.com/blackhack/LCD_I2C
I2C default pins in Arduino MBED core for Raspberry Pi Pico is GP6, GP7.
Default in Pico docs is GP4,GP5. At time of writing not changeable with API.
LCD_I2C lcd(0x27, 16, 2); // Default address of most PCF8574 modules, change accordingly

https://github.com/FortySevenEffects/arduino_midi_library
Create and bind the MIDI interface to the default HW Serial port (UART0 on GP0,GP1)
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

https://github.com/khoih-prog/RP2040_PWM
To generate stable output voltage, a high frequency PWM signal is required