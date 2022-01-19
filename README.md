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

## Libraries used (also forked at the time of writing)
https://github.com/blackhack/LCD_I2C

https://github.com/FortySevenEffects/arduino_midi_library

https://github.com/khoih-prog/RP2040_PWM
To generate stable output voltage, a high frequency PWM signal is required.
I was unable to use pins GP14-GP15 with this library, only one at a time would enable.
With GP12+14 on separate slices, everything worked. In my MicroPython prototype I was using 14+15.