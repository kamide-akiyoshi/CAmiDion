
/////////////////////////////////////////////////////////
// Configuration for CAmiDion

// LCD //////////////////////////////////////////////////
//
// Power source
//   Comment it out if you want to supply 5v (USB bus-power or NiMH battery x 4)
//   If you want to supply lower voltage from alkaline battery x 3 (4.5v), define this
//#define ALKALINE_BATTERY_3
//
// Interface type
//   Select one interface to connect your LCD
//   If you do not have LCD, you can comment all these out
#define USE_LCD_VIA_I2C      // I2C LCD (ST7032i compatible LCD etc.)
//#define USE_LCD_VIA_74164    // HD44780 compatible LCD via 74*164 SIPO
//#define USE_LCD_VIA_PARALLEL // HD44780 compatible LCD vid parallel interface
//
// Screen size
//   If you want to change the number of character
//   rows/columns on LCD, define like this
//#define LCD_COLS 16    // default is 8 in I2C LCD, 16 in HD44780
//#define LCD_ROWS 2     // default is 2

// LED /////////////////////////////////////////////////
#define USE_LED  // enables LED

// PWMDAC Synthesizer //////////////////////////////////
//#define PWMDAC_OUTPUT_PIN 3   // PWMDAC_Synth output pin# in Arduino (default: 3)
//#define PWMDAC_POLYPHONY 6    // PWMDAC_Synth polyphony (default: 6 polyphonic)
//#define PWMDAC_NOTE_A_FREQUENCY 440  // PWMDAC_Synth note-A frequency (default: 440Hz)

// Octave slider ///////////////////////////////////////
//   If you do not have octave slider, comment this out
#define OCTAVE_ANALOG_PIN 0  // Analog pin# in Arduino

// Buttons /////////////////////////////////////////////
// Anti-chattering
//   How many times of coutinuous button-off detections
//   required to treat as "button released"
#define BUTTON_RELEASE_WAIT_TIMES 20  // [0..255]

// MIDI ////////////////////////////////////////////////
#define USE_MIDI_IN        // enables MIDI IN
#define USE_MIDI_OUT       // enables MIDI OUT
#define MIDI_ENABLE_PIN 2  // MIDI-enable pin# in Arduino
// Velocities
#define NOTE_VELOCITY 100
#define ARPEGGIO_VELOCITY 100
#define DRUM_VELOCITY 100
// Drum
#define DRUM_NOTE_NUMBER 36   // Bass Drum 1
#define DRUM_MIDI_CHANNEL 10

////////////////////////////////////////////////////////
// Automatic definition
#if defined(USE_MIDI_IN) || defined(USE_MIDI_OUT)
#define USE_MIDI
#endif
#if defined(USE_LCD_VIA_I2C) \
 || defined(USE_LCD_VIA_74164) \
 || defined(USE_LCD_VIA_PARALLEL)
#define USE_LCD
#endif
