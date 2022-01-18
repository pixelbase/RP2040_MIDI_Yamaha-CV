/*  RP2040 MIDI to Yamaha-CV
    https://github.com/pixelbase/RP2040_MIDI_Yamaha-CV

Copyright (c) 2022 Viktor Nilsson

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#define VCO1_PIN 12
#define VCO2_PIN 14
#define TRG1_PIN 16
#define TRG2_PIN 17

#define PWM_FREQ 50000

#define INIT_DUTY_CYCLE 25.0
// "tone" at boot

#define INIT_MIDI_CH 1

#include <LCD_I2C.h>
#include <MIDI.h>
#include <RP2040_PWM.h>

enum synthmode {
  MONO, // VCO 1 & 2 identical, Trig 1 & 2 identical
  POLY, // Alternate VCO 1 / 2 and Trig 1 / 2 for 2-tone polyphony
  DUAL, // Dual mono: MIDI ChX -> VCO1, EG1, MIDI ChX+1 -> VCO2, EG2
  QUAD, // Individual MIDI channel per output (VCO1,TRG1,VCO2,TRG2)
  SPLIT // Split keyboard at C3 - left = VCO/Trig 1, right = VCO/Trig 2
};

synthmode mode; // global mode state
int ch; // global base MIDI channel

class Voltage {
public:
  Voltage(int pin) {
    _pin = pin;
  };

  void init(float init_duty) {
    PWM_Instance = new RP2040_PWM(_pin, PWM_FREQ, init_duty);
    PWM_Instance->setPWM();
  };

  void key(int key) {
    PWM_Instance->setPWM(_pin, PWM_FREQ, _duty(key), true);
  };

  void duty(float duty) {
    PWM_Instance->setPWM(_pin, PWM_FREQ, duty, true);
  };
// TODO: void bend() {};

private:
  int _pin;
  RP2040_PWM* PWM_Instance;

  float _duty(int key) {
  // get duty cycle from MIDI key code    
    if (key < 36) {key = 36;};  // below lowest defined key: 36 (duties[0])
    if (key > 36+37-1) {key = 36+36;}; // above highest defined key: 73 (duties[36])
    return duties[key-36];
  };

//TODO: move to file on flash? - check if exists and then read it?

  // TUNING - these are PWM duty cycles mapping to control voltages.
  // C1 = 0.250V, C2 = 0.5V, C3 = 1V, C4 = 2V (full range of the CS-15 keyboard)
  // I measured with a multimeter on the CV-out, then tuned the filtered PWM signal to match.
  float duties[37] = {
    8.94942,  // C1
    9.437705, // C#1
    9.994659, // D1
    10.55924, // D#1
    11.16960, // E1
    11.82574, // F1
    12.51239, // F#1
    13.19905, // G1
    13.96200, // G#1
    14.77073, // A1
    15.59472, // A#1
    16.52552, // B1
    17.39528,
    18.41763,
    19.43999,
    20.55390,
    21.71358,
    22.93430,
    24.24658,
    25.61989,
    27.02372,
    28.53437,
    30.13656,
    31.79980,
    17.39528,
    18.41763,
    19.43999,
    20.55390,
    21.71358,
    22.93430,
    24.24658,
    25.61989,
    27.02372,
    28.53437,
    30.13656,
    31.79980,
    1.882963,
  };
};

class Trigger {
public:
  Trigger(int pin) {
    _pin = pin;
  };

  void init() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, HIGH); // trig by pulling TRGx_PINs low
  };

  void on() {
    digitalWrite(_pin, LOW);
  };

  void off() {
    digitalWrite(_pin, HIGH);
  };

private:
  int _pin;
};

Trigger trg1 = Trigger(TRG1_PIN);
Trigger trg2 = Trigger(TRG2_PIN);
Voltage vco1 = Voltage(VCO1_PIN);
Voltage vco2 = Voltage(VCO2_PIN);

void setup() {
  // for MIDI debugging without having to use Serial or display (which contains delays)
  pinMode(LED_BUILTIN, OUTPUT);

  // init triggers and VCOs (sets pin modes and enables PWM)
// TODO: check if these can be moved to constructor (before) setup() is called..
  trg1.init();
  trg2.init();
  vco1.init(36+12);
  vco2.init(36+12);

  lcd.begin();
  lcd.backlight();
  lcd.print("Yamaha CS-15");
  lcd.setCursor(0, 1);
  lcd.print("MIDI to CV/Gate");

  MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages

  delay(4000);

  ch = INIT_MIDI_CH; // default MIDI channel
  setMode(MONO); // default mode
}

void loop() {
  // Read incoming messages
  MIDI.read();
}

void noteOn_MONO(byte ch, byte key, byte vel) {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(TRG1_PIN, LOW);
  // only if NO NOTE down? - One version with no trg off until last key?
  trg1.off();
  trg2.off();
  delay(10);
  trg1.on();
  trg2.on();
  vco1.key(key);
  vco2.key(key);
};

void noteOff_MONO(byte ch, byte key, byte vel) {
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(TRG1_PIN, HIGH);
  trg1.off();
  trg2.off();
};

void setMode(synthmode newmode) {
  mode = newmode;
  updateLCD();
  switch(mode) {
    default:
    case MONO:
      MIDI.setHandleNoteOn(noteOn_MONO);
      MIDI.setHandleNoteOff(noteOff_MONO);
      break;
  };
};

void updateLCD(){
  // redraw LCD with current MIDI channel and synth mode
  switch(mode) {
    case MONO:
      lcd.setCursor(0,0);
      lcd.print("MONO-mode Ch 01 ");
      lcd.setCursor(0,1);
      lcd.print("Output 1,2 equal");
      break;
    case POLY:
      lcd.setCursor(0,0);
      lcd.print("POLY-mode Ch 01 ");
      lcd.setCursor(0,1);
      lcd.print("Set VCO/F/A 1=2 ");
      break;
    case DUAL:
      lcd.setCursor(0,0);
      lcd.print("DUAL  V1,T1 ch01");
      lcd.setCursor(0,1);
      lcd.print("mono  V2,T2 ch02");
      break;
    case QUAD:
      lcd.setCursor(0,0);
      lcd.print("QUAD V1:01 T1:02");
      lcd.setCursor(0,1);
      lcd.print("mode V2:03 T2:04");
      break;
    default:
      break;
 };
};
