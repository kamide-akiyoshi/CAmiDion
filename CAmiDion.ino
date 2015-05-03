//
// CAmiDion - Musical Chord Instrument
//  ver.20150503
//  http://www.yk.rim.or.jp/~kamide/music/chordhelper/hardware/
//  http://kamide.b.sourceforge.jp/camidion/
//  by Akiyoshi Kamide (Twitter: @akiyoshi_kamide)
//

/////////////////////////////////
// Function enable/disable
//
// LCD (74164 or I2C)
#define USE_I2C_LCD // 8x2 I2C LCD (ST7032i compatible)
#define LCD_COLS 8
//#define USE_74164_FOR_LCD // 16x2 LCD (HD44780 compatible) with 74xx164 SIPO
//#define LCD_COLS 16
#define LCD_ROWS 2
//
#define USE_LED
#define USE_MIDI_IN
#define USE_MIDI_OUT
//
// OCTAVE slider
#define OCTAVE_ANALOG_PIN   0
//
// PWM DAC Synthesizer lib
#define PWMDAC_OUTPUT_PIN  3
#include "PWMDAC_Synth.h"
//
/////////////////////////////////

// MIDI
#if defined(USE_MIDI_IN) || defined(USE_MIDI_OUT)
#define USE_MIDI
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#define MIDI_ENABLE_PIN     2
#endif

// Wavetable
PROGMEM const byte shepardToneSawtoothWavetable[] = {
0,	6,	6,	12,	6,	12,	12,	18,
5,	11,	11,	18,	11,	17,	17,	24,
5,	11,	11,	17,	11,	17,	17,	23,
11,	17,	17,	23,	17,	23,	23,	29,
5,	11,	11,	17,	11,	17,	17,	23,
11,	17,	17,	23,	17,	23,	23,	29,
11,	17,	17,	23,	17,	23,	23,	29,
16,	23,	23,	29,	22,	29,	29,	35,
4,	10,	10,	16,	10,	16,	16,	22,
10,	16,	16,	22,	16,	22,	22,	28,
10,	16,	16,	22,	16,	22,	22,	28,
16,	22,	22,	28,	22,	28,	28,	34,
9,	15,	15,	22,	15,	21,	21,	28,
15,	21,	21,	27,	21,	27,	27,	33,
15,	21,	21,	27,	21,	27,	27,	33,
21,	27,	27,	33,	27,	33,	33,	39,
3,	9,	9,	15,	9,	15,	15,	21,
8,	15,	15,	21,	14,	21,	21,	27,
8,	14,	14,	20,	14,	20,	20,	26,
14,	20,	20,	26,	20,	26,	26,	32,
8,	14,	14,	20,	14,	20,	20,	26,
14,	20,	20,	26,	20,	26,	26,	32,
14,	20,	20,	26,	20,	26,	26,	32,
20,	26,	26,	32,	26,	32,	32,	38,
7,	13,	13,	19,	13,	19,	19,	25,
13,	19,	19,	25,	19,	25,	25,	31,
13,	19,	19,	25,	19,	25,	25,	31,
19,	25,	25,	31,	25,	31,	31,	37,
12,	19,	19,	25,	18,	25,	25,	31,
18,	24,	24,	30,	24,	30,	30,	36,
18,	24,	24,	30,	24,	30,	30,	36,
24,	30,	30,	36,	30,	36,	36,	42,
};
PROGMEM const byte guitarWavetable[] = {
23,	24,	24,	25,	25,	25,	26,	26,
26,	27,	27,	28,	29,	30,	32,	33,
34,	35,	36,	36,	36,	36,	36,	37,
37,	37,	38,	39,	39,	40,	40,	40,
40,	40,	39,	39,	40,	40,	40,	41,
41,	41,	41,	40,	41,	41,	41,	41,
42,	42,	42,	42,	41,	41,	41,	41,
42,	41,	41,	40,	40,	40,	39,	39,
38,	37,	37,	37,	37,	36,	36,	36,
35,	34,	33,	32,	31,	30,	30,	30,
30,	30,	30,	30,	30,	29,	28,	27,
27,	27,	26,	26,	26,	26,	25,	24,
24,	23,	23,	23,	23,	22,	22,	22,
22,	21,	21,	21,	21,	21,	21,	21,
20,	21,	20,	20,	19,	19,	18,	18,
18,	18,	18,	18,	18,	18,	17,	16,
16,	15,	14,	14,	15,	15,	16,	16,
16,	16,	17,	17,	17,	17,	17,	17,
18,	18,	18,	18,	17,	17,	17,	17,
17,	17,	18,	18,	17,	17,	16,	15,
15,	14,	14,	15,	15,	15,	16,	16,
16,	16,	16,	16,	16,	16,	16,	16,
16,	16,	16,	15,	15,	15,	15,	15,
14,	14,	13,	12,	11,	10,	9,	9,
9,	9,	8,	8,	8,	7,	6,	6,
5,	4,	3,	2,	2,	3,	3,	3,
3,	4,	4,	4,	4,	4,	3,	3,
2,	2,	2,	2,	2,	1,	2,	2,
3,	4,	4,	4,	5,	5,	5,	6,
6,	7,	9,	10,	11,	11,	12,	12,
13,	13,	14,	14,	14,	15,	15,	16,
17,	17,	18,	19,	20,	21,	21,	22,
};
PROGMEM const byte randomWavetable[] = {
39,	22,	21,	9,	23,	13,	28,	31,
15,	30,	40,	8,	29,	26,	27,	8,
34,	30,	4,	22,	39,	25,	35,	33,
38,	17,	7,	38,	18,	24,	12,	9,
8,	36,	27,	17,	33,	0,	13,	35,
20,	15,	34,	0,	34,	6,	29,	38,
30,	20,	16,	39,	26,	18,	28,	28,
24,	38,	27,	31,	25,	14,	9,	25,
31,	13,	3,	17,	29,	23,	18,	6,
12,	20,	30,	27,	1,	40,	9,	19,
26,	21,	4,	25,	1,	16,	11,	18,
15,	23,	30,	7,	37,	23,	11,	19,
30,	36,	7,	9,	17,	27,	8,	41,
4,	9,	26,	0,	24,	18,	6,	15,
30,	23,	7,	9,	9,	41,	1,	0,
33,	9,	34,	18,	19,	22,	25,	16,
22,	31,	41,	21,	27,	35,	38,	32,
17,	16,	10,	39,	36,	9,	37,	13,
16,	12,	10,	14,	21,	12,	19,	12,
11,	19,	0,	32,	13,	27,	32,	10,
18,	5,	22,	9,	6,	33,	29,	2,
4,	2,	17,	0,	38,	14,	18,	16,
25,	10,	8,	3,	3,	39,	3,	5,
7,	22,	6,	11,	16,	23,	23,	5,
3,	23,	22,	25,	17,	7,	19,	29,
41,	0,	10,	15,	14,	41,	6,	40,
8,	0,	19,	16,	30,	18,	2,	24,
17,	37,	11,	6,	1,	33,	10,	31,
33,	19,	20,	23,	24,	41,	4,	40,
29,	12,	4,	11,	38,	17,	41,	8,
15,	37,	19,	34,	36,	8,	19,	10,
32,	14,	19,	1,	1,	35,	35,	3,
};
PROGMEM const byte * const wavetables[] = {
#ifdef OCTAVE_ANALOG_PIN
  PWM_SYNTH.sawtoothWavetable,
  PWM_SYNTH.squareWavetable,
  guitarWavetable,
  PWM_SYNTH.sineWavetable,
  PWM_SYNTH.triangleWavetable,
#endif
  PWM_SYNTH.shepardToneSineWavetable,
  shepardToneSawtoothWavetable,
  randomWavetable,
};

class MusicalNote {
  protected:
    char co5_value;  // Circle of Fifths value
    char note_value; // Chromatic note value
  public:
    static int getLowerBound() {
#ifdef OCTAVE_ANALOG_PIN
      return map(analogRead(OCTAVE_ANALOG_PIN), 0, 1023, 0, 127-11);
#else
      return 36;
#endif
    }
    static int shiftOctave(int note) { return shiftOctave(note, getLowerBound()); }
    static int shiftOctave(int note, int lower_bound) {
      return PWMDACSynth::musicalConstrain12(note, lower_bound, lower_bound + 11);
    }
    static int shiftOctave(int note, int lower_bound, int chromatic_offset) {
      return shiftOctave( note, (
        chromatic_offset == 0 ?
        lower_bound :
        PWMDACSynth::musicalConstrain12(lower_bound + chromatic_offset, 0, 127-11)
      ) );
    }
    MusicalNote() { note_value = co5_value = 0; }
    MusicalNote(char co5) { setCo5(co5); }
    void setCo5(char co5) {
      co5_value = co5;
      note_value = PWMDACSynth::musicalMod12( co5 + (co5 & 1) * 6 );
    }
    char getCo5()  { return  co5_value; }
    char getNote() { return note_value; }
    char getOctaveShiftedNote() { return shiftOctave(note_value); }
    char getOctaveShiftedNote(int chromatic_offset) {
      return shiftOctave( note_value, getLowerBound(), chromatic_offset );
    }
    char *print(char *bufp, char offset = 0) {
      char co5 = co5_value + offset + 1;
      *bufp++ = "FCGDAEB"[PWMDACSynth::musicalMod7(co5)];
      if (co5 < 0) {
        *bufp++ = 'b'; // flat or double flat
        if (co5 < -7) *bufp++ = 'b'; // double flat
      }
      else if (co5 >= 14) *bufp++ = 'x'; // double sharp
      else if (co5 >=  7) *bufp++ = '#'; // sharp
      return bufp;
    }
};

class KeySignature : public MusicalNote {
  public:
    void shift(char offset, char v_min = -7, char v_max = 7) {
      offset += co5_value;
      if      (offset > v_max) offset -= 12;
      else if (offset < v_min) offset += 12;
      MusicalNote::setCo5(offset);
    }
    char *print(char *bufp) {
      char n = abs(co5_value);
      if (n == 0) return bufp;
      char b = co5_value < 0 ? 'b':'#';
      if (n < 5) do { *bufp++ = b; } while(--n);
      else { *bufp++ = '0'+n; *bufp++ = b; }
      return bufp;
    }
    char *printSymbol(char *bufp) {
      bufp = MusicalNote::print(bufp);
      *bufp++ = '/';
      bufp = MusicalNote::print(bufp,3);
      *bufp++ = 'm';
      return bufp;
    }
};

class MusicalChord : public MusicalNote {
   protected:
    char offset3;   // offset of major 3rd
    char offset5;   // offset of perfect 5th
    char offset7;   // 0:none -1:M7 -2:7th -3:6th/dim7th
    boolean has9;   // add9 extended
   public:
     MusicalChord() : MusicalNote() {
      offset3 = offset5 = offset7 = 0; has9 = false;
    }
    MusicalChord(
      KeySignature key_sig,
      char offset = 0, char offset3 = 0,
      char offset5 = 0, char offset7 = 0,
      boolean has9 = false
    ) {
      offset += key_sig.getCo5();
      if( (this->offset3 = offset3) < 0 ) offset += 3;
      MusicalNote::setCo5(offset);
      this->offset5 = offset5;
      this->offset7 = offset7;
      this->has9 = has9;
    }
    char get3rdNote() { return note_value +  4 + offset3; }
    char get5thNote() { return note_value +  7 + offset5; }
    char get7thNote() { return note_value + 12 + offset7; }
    char get9thNote() { return note_value + (has9?14:12); }
    char getNote(char i) { return getNote( i, getLowerBound() ); }
    char getNote(char i, int lower_bound) {
      switch(i) {
      case 0: return shiftOctave( note_value, lower_bound );
      case 1: return shiftOctave( get3rdNote(), lower_bound );
      case 2: return shiftOctave( get5thNote(), lower_bound );
      case 3: return offset7==0?
        shiftOctave( get5thNote(), lower_bound, -12 ) :
        shiftOctave( get7thNote(), lower_bound );
      case 4: return has9?
         shiftOctave( get9thNote(), lower_bound ):
         shiftOctave( get3rdNote(), lower_bound, -12 );
      case 5: return shiftOctave( note_value, lower_bound, -12 );
      }
    }
    char *toNotes( char *notes, size_t n_notes, int lower_bound ) {
      for(int i=0; i<n_notes; i++) notes[i] = getNote(i, lower_bound);
      return notes;
    }
    char *print(char *bufp) {
      bufp = MusicalNote::print(bufp);
      if( offset3 < 0 && offset5 < 0 && offset7 == -3 ) {
        memcpy(bufp,"dim",3); bufp += 3;
        *bufp++ = (has9 ? '9':'7');
      }
      else {
        if( offset3 < 0 ) *bufp++ = 'm';
        if( offset5 > 0 ) { memcpy(bufp,"aug",3); bufp += 3; }
        switch(offset7) {
          case  0: if(has9) { memcpy(bufp,"add9",4); bufp += 4; } break;
          case -1: *bufp++ = 'M'; /* FALLTHROUGH */
          case -2: *bufp++ = (has9 ? '9':'7'); break;
          case -3: *bufp++ = '6'; if(has9) *bufp++ = '9'; break;
        }
        if( offset3 > 0 ) { memcpy(bufp,"sus4",4); bufp += 4; }
        if( offset5 < 0 ) { memcpy(bufp,"(-5)",4); bufp += 4; }
      }
      return bufp;
    }
};

byte current_midi_channel = 1; // 1==CH1, ...

////////// LCD /////////////
#if defined(USE_74164_FOR_LCD) || defined(USE_I2C_LCD)
#define USE_LCD
#endif
#ifdef USE_LCD

#ifdef USE_74164_FOR_LCD
#include <LcdCore.h>
#include <Lcd74HC164.h>
#define LCD_PARENT_CLASS Lcd74HC164
// data,clock,enable
#define LCD_CONSTRUCTOR_ARGS  18,19,4
#elif defined(USE_I2C_LCD)
#include <I2CLiquidCrystal.h>
#include <Wire.h>
#define LCD_PARENT_CLASS I2CLiquidCrystal
// contrast(0-63),5V/3.3V
//
// USB bus-power or NiMH battery x 4 (5v)
#define LCD_CONSTRUCTOR_ARGS  (uint8_t)63,(bool)true
//
// Alkaline battery x 3 (4.5v)
//#define LCD_CONSTRUCTOR_ARGS  (uint8_t)13,(bool)false
//
#else // Parallel
#include <LiquidCrystal.h>
#define LCD_PARENT_CLASS LiquidCrystal
// rs,enable,d4,d5,d6,d7
#define LCD_CONSTRUCTOR_ARGS  14,15,16,17,18,19
#endif

class CAmiDionLCD : public LCD_PARENT_CLASS {
  protected:
    static const char NATURAL = 1;
    char line_buf[24];
    char *bufp;
    MusicalChord current_chord;
    void clearChord() { current_chord.setCo5(0x7F); }
    void clearLineBuffer() {
      memset( line_buf, ' ', LCD_COLS + 1 );
      bufp = line_buf;
    }
    byte printLineBuffer() {
      line_buf[LCD_COLS] = '\0';
      print(line_buf);
      clearLineBuffer();
    }
    void setString(char *str, byte len) {
      memcpy( bufp, str, len ); bufp += len;
    }
    void setString(char *str) { setString(str,strlen(str)); }
    void setHex(byte value) {
      *bufp++ = value + (value < 10 ?'0':'A'-10);
    }
  public:
    CAmiDionLCD() : LCD_PARENT_CLASS( LCD_CONSTRUCTOR_ARGS ) {
      current_chord = MusicalChord();
      clearChord();
    }
    void begin(byte cols, byte rows) {
      LCD_PARENT_CLASS::begin(cols,rows);
      PROGMEM static const uint8_t natural[] = {
        B01000,
        B01000,
        B01110,
        B01010,
        B01110,
        B00010,
        B00010,
        B00000,
      };
      createChar_P(NATURAL, natural);
      setCursor(0,0);
#if LCD_COLS >= 16
      print("*** CAmiDion ***");
#else
      print("CAmiDion");
#endif
      clearLineBuffer();
    }
    void createChar_P(uint8_t num, PROGMEM const uint8_t data[]) {
      uint8_t pattern_buf[8];
      memcpy_P( pattern_buf, data, sizeof(pattern_buf) );
      createChar(num, pattern_buf);
    }
    void printKeySignature(KeySignature keysig, boolean is_changing) {
      setString("Key",3);
      *bufp++ = (is_changing ? '>' : ':');
      bufp = keysig.print(bufp);
      if( keysig.getCo5() == 0 ) *bufp++ = NATURAL;
#if LCD_COLS >= 15
      *bufp++ = '(';
      bufp = keysig.printSymbol(bufp);
      *bufp++ = ')';
#endif
      setCursor(0,1);
      printLineBuffer();
    }
    void printChord() {
      setString("Chord:",6);
      printLineBuffer();
      clearChord();
    }
    void printChord(MusicalChord chord) {
      if( ! memcmp( &current_chord, &chord, sizeof(chord) ) ) return;
      current_chord = chord;
#if LCD_COLS >= 15
      setString("Chord:",6);
#endif
      bufp = chord.print(bufp);
      setCursor(0,0);
      printLineBuffer();
    }
    void printTempo(unsigned int bpm, boolean is_changing) {
      *bufp++ = 'T';
#if LCD_COLS >= 15
      setString("empo",4);
#endif
      bufp += sprintf(bufp,"%c%ubpm    ",is_changing?'>':':',bpm);
      setCursor(0,0);
      printLineBuffer();
      clearChord();
    }
    void printEnvelope() {
      EnvelopeParam *ep = &(PWM_SYNTH.getChannel(current_midi_channel)->env_param);
      *bufp++ = 'a';
      setHex(0xF - PWM_SYNTH.log2(ep->attack_speed));
      *bufp++ = 'd';
      setHex(ep->decay_time);
      *bufp++ = 's';
      setHex(ep->sustain_level >> 12);
      *bufp++ = 'r';
      setHex(ep->release_time);
      setCursor(0,1);
      printLineBuffer();
    }
    void printWaveform(
      char midi_channel,
      PROGMEM const byte wavetable[],
      boolean channel_is_changing
    ) {
      PROGMEM static const uint8_t sawtooth_left[] = {
        B00000,
        B00000,
        B00000,
        B00011,
        B01100,
        B10000,
        B00000,
        B00000,
      };
      PROGMEM static const uint8_t sawtooth_right[] = {
        B00011,
        B01101,
        B10001,
        B00001,
        B00001,
        B00001,
        B00001,
        B00000,
      };
      PROGMEM static const uint8_t square_up[] = {
        B00111,
        B00100,
        B00100,
        B00100,
        B00100,
        B00100,
        B11100,
        B00000,
      };
      PROGMEM static const uint8_t square_down[] = {
        B11100,
        B00100,
        B00100,
        B00100,
        B00100,
        B00100,
        B00111,
        B00000,
      };
      PROGMEM static const uint8_t sine_up[] = {
        B00110,
        B01001,
        B10000,
        B10000,
        B00000,
        B00000,
        B00000,
        B00000,
      };
      PROGMEM static const uint8_t sine_down[] = {
        B00000,
        B00000,
        B00000,
        B10000,
        B10000,
        B01001,
        B00110,
        B00000,
      };
      PROGMEM static const uint8_t random_pattern[] = {
        B10101,
        B01010,
        B10101,
        B01010,
        B10101,
        B01010,
        B10101,
        B00000,
      };
      PROGMEM static const uint8_t backslash_pattern[] = {
        B00000,
        B10000,
        B01000,
        B00100,
        B00010,
        B00001,
        B00000,
        B00000,
      };
      static const char wavename2[]  PROGMEM = "\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02";
      static const char wavename23[] PROGMEM = "\x02\x03\x02\x03\x02\x03\x02\x03\x02\x03";
      static const char wavename_triangle[] PROGMEM = "/\x02/\x02/\x02/\x02/\x02";
      static const char wavename_guitar[] PROGMEM = "Guitar";
      static const char wavename_shepard[] PROGMEM = "Shepard Tone";
      static const char wavename_triangle_shepard[] PROGMEM = "\x02\x03Shepard";
      *bufp++ = 'C';
      *bufp++ = 'h';
      if( midi_channel >= 10 ) {
        *bufp++ = '1'; midi_channel -= 10;
      }
      *bufp++ = midi_channel + '0';
      *bufp++ = ( channel_is_changing ? '>' : ':' );
      const char PROGMEM *wavename;
      if( wavetable == PWM_SYNTH.sawtoothWavetable ) {
        createChar_P( 2, sawtooth_left );
        createChar_P( 3, sawtooth_right );
        wavename = wavename23;
      }
      else if( wavetable == PWM_SYNTH.squareWavetable ) {
        createChar_P( 2, square_up );
        createChar_P( 3, square_down );
        wavename = wavename23;
      }
      else if( wavetable == guitarWavetable ) {
        wavename = wavename_guitar;
      }
      else if( wavetable == PWM_SYNTH.sineWavetable ) {
        createChar_P( 2, sine_up );
        createChar_P( 3, sine_down );
        wavename = wavename23;
      }
      else if( wavetable == PWM_SYNTH.triangleWavetable ) {
        createChar_P( 2, backslash_pattern );
        wavename = wavename_triangle;
      }
      else if( wavetable == PWM_SYNTH.shepardToneSineWavetable ) {
        wavename = wavename_shepard;
      }
      else if( wavetable == shepardToneSawtoothWavetable ) {
        createChar_P( 2, sawtooth_left );
        createChar_P( 3, sawtooth_right );
        wavename = wavename_triangle_shepard;
      }
      else if( wavetable == randomWavetable ) {
        createChar_P( 2, random_pattern );
        wavename = wavename2;
      }
      size_t len = strlen_P(wavename);
      if( len > LCD_COLS ) len = LCD_COLS;
      memcpy_P( bufp, wavename, len );
      bufp += len;
      setCursor(0,0);
      printLineBuffer();
      clearChord();
    }
};
CAmiDionLCD lcd = CAmiDionLCD();

#endif // USE_LCD

//////////////////////////////
// LED buffer (in HEX)
//
//       3       5 7   A C E
//     0 1 2    4 6 8 9 B D F
//////////////////////////////
#ifdef USE_LED
class LedStatus {
  protected:
    union _LEDs {
      unsigned int value16;
      byte values8[2];
    } leds;
  public:
    static const byte LEFT = 0;
    static const byte CENTER = 1;
    static const byte RIGHT = 2;
    static const byte UPPER = 3;
    LedStatus(unsigned int value16) { this->leds.value16 = value16;  }
    boolean isOn(byte i, byte mask) { return leds.values8[i] & mask; }
    void setOn(byte pos)  { sbi(leds.values8[0], pos); }
    void setOff(byte pos) { cbi(leds.values8[0], pos); }
    void set(boolean isOn, byte pos) { if(isOn) setOn(pos); else setOff(pos); }
    void allNotesOff() { leds.value16 &= 0x000F; }
    void noteOn(byte pitch) { leds.value16 |= 0x0010 << pitch; }
    void noteOff(byte pitch) { leds.value16 &= ~(0x0010 << pitch); }
    void showNote(byte pitch) { allNotesOff(); noteOn(pitch); }
    void showMidiChannel(byte ch0) { leds.value16 = 1 << ch0; }
    void show(KeySignature ks) { showNote(ks.getNote()); }
};
LedStatus led_main = LedStatus(0x0004); // One-push chord mode
LedStatus led_key  = LedStatus(0x0011); // C major key, hold arpeggio
LedStatus led_ctrl = LedStatus(0x0001); // Ch.1 (0..15 = MIDI Channel 1..16)
class NoteCounter {
  private:
    byte counts[12];
  public:
    NoteCounter() { memset(counts,0,NumberOf(counts)); }
    void noteOff(byte pitch) {
      pitch = PWMDACSynth::musicalMod12(pitch);
      if (--counts[pitch] == 0) led_main.noteOff(pitch);
    }
    void noteOn(byte pitch) {
      pitch = PWMDACSynth::musicalMod12(pitch);
      if (counts[pitch]++ == 0) led_main.noteOn(pitch);
    }
};
NoteCounter noteCounter = NoteCounter();
#endif

/////////////////////
// Waveform manager
/////////////////////
class WaveformController {
  protected:
    char selected_waveforms[16];
  public:
    WaveformController() {
      memset(selected_waveforms, 0, sizeof(selected_waveforms));
    }
    void clear() {
      PWM_SYNTH.setWave( (byte *)pgm_read_word(wavetables) );
      change(10, NumberOf(wavetables)-1); // set MIDI Ch.10 to random wave noise
    }
    void show(boolean channel_is_changing = false) {
#ifdef USE_LCD
      lcd.printWaveform(
        current_midi_channel,
        (PROGMEM const byte *)pgm_read_word(
          wavetables + selected_waveforms[current_midi_channel - 1]
        ),
        channel_is_changing
      );
#endif
    }
    void change(char offset) { change(current_midi_channel,offset); }
    void change(byte midi_channel, char offset) {
      char *selected_waveform = selected_waveforms + (midi_channel - 1);
      *selected_waveform += offset;
      if (*selected_waveform < 0)
        *selected_waveform += NumberOf(wavetables);
      else if (*selected_waveform >= NumberOf(wavetables))
        *selected_waveform -= NumberOf(wavetables);
      PWM_SYNTH.getChannel(midi_channel)->wavetable
        = (PROGMEM const byte *)pgm_read_word(wavetables + *selected_waveform);
      if(midi_channel == current_midi_channel) show();
    }
};
WaveformController waveform = WaveformController();

void changeMidiChannel(char offset) {
  char ch0 = current_midi_channel + offset - 1;
  current_midi_channel = (ch0 &= 0xF) + 1;
#ifdef USE_LED
  led_ctrl.showMidiChannel(ch0);
#endif
  waveform.show(true);
#ifdef USE_LCD
  lcd.printEnvelope();
#endif
}

KeySignature key_signature = KeySignature();

////////////////////////////////////////////////////////////////////
// Button matrix anode6(+1) - cathode8(+1)
//
//   1-0                            2-0 2-1 2-2
// 0-0 1-1    2-3 2-4 2-5 2-6 2-7 | 5-0 5-1 5-2 5-3 5-4 5-5 5-6 5-7
//   0-1 1-2  1-3 1-4 1-5 1-6 1-7 | 4-0 4-1 4-2 4-3 4-4 4-5 4-6 4-7
//     0-2    0-3 0-4 0-5 0-6 0-7 | 3-0 3-1 3-2 3-3 3-4 3-5 3-6 3-7
//
////////////////////////////////////////////////////////////////////
class Button {
  public:
    char cathode8, anode6;
    void set(Button *button_p) {
      cathode8 = button_p->cathode8;
      anode6 = button_p->anode6;
    }
    boolean equals(Button *button_p) {
      return cathode8 == button_p->cathode8 && anode6 == button_p->anode6;
    }
};

/////////////////////////////
// Metronome
/////////////////////////////
#define US_PER_MINUTE  (60000000ul) // microseconds per minute
#define NULL_BUTTON 0x40
class Metronome {
  protected:
    // metronome
    unsigned long us_timeout;
    unsigned long us_interval;
    unsigned long us_tapped;
    unsigned int bpm;
    byte resolution;
    byte count;
    boolean is_started;
    // drum
    char drum_note;
    boolean drum_on;
    boolean isArpeggioMode();
    void drumOff() {
      if( drum_note >= 0 ) {
        PWM_SYNTH.noteOff(10, drum_note, 100);
        drum_note = -1;
      }
#ifdef USE_LED
      led_main.setOff(LedStatus::UPPER);
      led_key.setOff(LedStatus::UPPER);
      if(isArpeggioMode()) led_main.setOff(LedStatus::LEFT);
      if(drum_on) led_main.setOff(LedStatus::CENTER);
#endif
    }
    void drumOn() {
      if(drum_on) PWM_SYNTH.noteOn(10, drum_note = 0, 100);
#ifdef USE_LED
      led_main.setOn(LedStatus::UPPER);
      led_key.setOn(LedStatus::UPPER);
      if (isArpeggioMode()) led_main.setOn(LedStatus::LEFT);
      if (drum_on) led_main.setOn(LedStatus::CENTER);
#endif
    }
    // arpeggiator
    Button button;
    MusicalChord chord;
    char arpeggiating_note;
    char arpeggiating_midi_channel;
    void noteOff() {
      HandleNoteOff(arpeggiating_midi_channel, arpeggiating_note, 100);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOff(arpeggiating_note, 100, arpeggiating_midi_channel);
#endif
    }
    void noteOn() {
      HandleNoteOn(arpeggiating_midi_channel, arpeggiating_note, 100);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOn(arpeggiating_note, 100, arpeggiating_midi_channel);
#endif
    }
    void adjustInterval() {
      us_interval = (unsigned long)(US_PER_MINUTE / resolution / bpm);
    }
    void adjustBpm() {
      bpm = (unsigned int)(US_PER_MINUTE / resolution / us_interval);
    }
  public:
    Metronome() {
      chord = MusicalChord();
      button = Button();
      button.cathode8 = NULL_BUTTON;
      arpeggiating_midi_channel = -1;
      us_tapped = 0xFFFFFFFF;
      resolution = 4;
      setBpm(120);
      drum_on = false;
      drum_note = -1;
      start();
    }
    unsigned long getIntervalMicros() { return us_interval; }
    void setIntervalMicros(unsigned long us_interval) {
      this->us_interval = us_interval;
      adjustBpm();
    }
    byte getResolution() { return resolution; }
    void setResolution(byte resolution) {
      this->resolution = resolution;
      adjustInterval();
    }
    unsigned int getBpm() { return bpm; }
    void setBpm(unsigned int bpm) { this->bpm = bpm; adjustInterval(); }
    boolean isDrumOn() { return drum_on; }
    void toggleDrum() {
      drum_on = ! drum_on;
      if( drum_on ) {
        start();
#ifdef USE_LED
        led_main.setOn(LedStatus::CENTER);
        led_key.setOn(LedStatus::CENTER);
      } else {
        led_main.setOff(LedStatus::CENTER);
        led_key.setOff(LedStatus::CENTER);
#endif
      }
    }
    boolean isStarted() { return is_started; }
    boolean isRunning() { return us_timeout < 0xFFFFFFFF; }
    void  stop() { is_started = false; }
    void start() { is_started = true; if( ! isRunning() ) sync(); }
    void update() { if( micros() >= us_timeout ) timeout(); }
    void sync( unsigned long us = micros() ) {
      us_timeout = us; count = 0; is_started = true;
    }
    void tap( unsigned long us = micros() ) {
      if( us_tapped < 0xFFFFFFFF ) {
        unsigned long us_diff = us - us_tapped;
        if( us_diff < 1500000 ) setIntervalMicros(us_diff / resolution);
      }
      sync(us_tapped = us);
    }
    void setChord(MusicalChord chord) { this->chord = chord; }
    void setButton(Button *button_p) { button.set(button_p); }
    void resetButton() { button.cathode8 = NULL_BUTTON; }
    void resetButton(Button *button_p) {
      if( button.equals(button_p) ) resetButton();
    }
    void timeout() {
      if( arpeggiating_midi_channel >= 0 ) {
        noteOff();
        arpeggiating_midi_channel = -1;
      }
      if( button.cathode8 != NULL_BUTTON ) {
        arpeggiating_note = chord.getNote(random(6));
        arpeggiating_midi_channel = current_midi_channel;
        noteOn();
      }
      switch(count) {
        case 0: drumOn();  break;
        case 1: drumOff(); break;
      }
      if(count < resolution - 1) count++;
      else if( is_started ) count = 0;
      else { us_timeout = 0xFFFFFFFF; return; }
      us_timeout += us_interval;
    }
};
Metronome metronome = Metronome();

class ActiveNotes {
  protected:
    size_t n_notes;
    char *notes;
    byte midi_channel;
    void noteOff(char note) {
      HandleNoteOff(midi_channel, note, 100);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOff(note, 100, midi_channel);
#endif
    }
    void noteOn(char note) {
      HandleNoteOn(midi_channel, note, 100);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOn(note, 100, midi_channel);
#endif
    }
    void noteOff() {
      char *p = notes;
      for( size_t n = n_notes; n>0; n-- ) noteOff(*p++);
    }
    void noteOn() {
      char *p = notes;
      for( size_t n = n_notes; n>0; n-- ) noteOn(*p++);
    }
  public:
    ActiveNotes() { n_notes = 0; }
    boolean isActivated() { return n_notes > 0 ; }
    size_t activate(byte midi_channel, char note) {
      if (isActivated()) return 0;
      notes = (char *)malloc(sizeof(char));
      if (notes == NULL) return 0;
      n_notes = 1;
      *notes = note;
      this->midi_channel = midi_channel;
      noteOn();
      return n_notes;
    }
    size_t activate(byte midi_channel, MusicalChord *chord) {
      if (isActivated()) return 0;
      n_notes = 6;
      notes = (char *)malloc(n_notes * sizeof(char));
      if (notes == NULL) { n_notes = 0; return 0; }
      chord->toNotes(notes, n_notes, chord->getLowerBound());
      this->midi_channel = midi_channel;
      noteOn();
      return n_notes;
   }
    void deactivate() {
      if( ! isActivated() ) return;
      noteOff(); free(notes); n_notes = 0;
    }
};

class ActiveNoteButton : public Button {
  private:
    ActiveNotes active_notes;
  public:
    ActiveNoteButton() { active_notes = ActiveNotes(); }
    ActiveNotes *getActiveNotes() { return &active_notes; }
    boolean deactivate(Button *button_p) {
      if( ! equals(button_p) ) return false;
      active_notes.deactivate(); return true;
    }
};

class ActiveNoteButtons {
  protected:
    ActiveNoteButton buttons[8];
    byte hold_mode;
    byte note_mode;
  public:
    static const byte MODE_MONO = 0;
    static const byte MODE_POLY = 1;
    static const byte MODE_ARPEGGIO = 2;
    static const byte MODE_POLY_ARPEGGIO = 3;
    static const byte NUM_MODES = 4;
    ActiveNoteButtons() {
      for(char i=0; i<NumberOf(buttons); i++) buttons[i] = ActiveNoteButton();
      hold_mode = MODE_ARPEGGIO;
      note_mode = MODE_POLY;
    }
    ActiveNoteButton *getFreeEntry() {
      ActiveNoteButton *bp = buttons;
      for( char i=NumberOf(buttons); i>0; i--, bp++ )
        if( !(bp->getActiveNotes()->isActivated()) ) return bp;
      return NULL;
    }
    void deactivateAll() {
      ActiveNoteButton *bp = buttons;
      for( char i=NumberOf(buttons); i>0; i--, bp++ )
        bp->getActiveNotes()->deactivate();
    }
    void deactivate(Button *button_p) {
      ActiveNoteButton *bp = buttons;
      for( char i=NumberOf(buttons); i>0; i--, bp++ )
        if( bp->deactivate(button_p) ) return;
    }
    boolean isHoldMode(byte mode_mask) { return hold_mode & mode_mask; }
    void toggleHoldMode(byte mode_mask) {
      hold_mode ^= mode_mask;
#ifdef USE_LED
      byte led_position = (mode_mask == MODE_POLY ? LedStatus::RIGHT : LedStatus::LEFT);
#endif
      if( isHoldMode(mode_mask) ) {
#ifdef USE_LED
        led_key.setOn(led_position);
#endif
      }
      else {
        if(mode_mask == MODE_POLY) deactivateAll();
        else metronome.resetButton();
#ifdef USE_LED
        led_key.setOff(led_position);
#endif
      }
    }
    char getNoteMode() { return note_mode; }
    void changePolyMode(byte mode_mask) {
      note_mode ^= mode_mask;
      if(mode_mask == MODE_POLY) deactivateAll();
      else metronome.resetButton();
#ifdef USE_LED
      led_main.set( note_mode & MODE_POLY, LedStatus::RIGHT );
      led_main.set( note_mode & MODE_ARPEGGIO, LedStatus::LEFT );
#endif
#ifdef USE_LCD
      lcd.setCursor(0,0);
      if(note_mode) lcd.printChord(); else waveform.show(false);
#endif
    }
};
ActiveNoteButtons active_note_buttons = ActiveNoteButtons();
boolean Metronome::isArpeggioMode() {
  return active_note_buttons.getNoteMode() & ActiveNoteButtons::MODE_ARPEGGIO;
}

class Buttons {
  protected:
    byte anode6_bits_array[8]; // anode 6-bits low-active input buffer x cachode 8-bits
    char offset5; // -1:dim(-5) 0:P5 +1:aug(+5)
    char offset7; // 0:octave -1:M7 -2:7th -3:6th(dim7)
    boolean has9; // add9
    boolean ctrl;
    boolean key;
    LedStatus *led_view;
    void pressed(Button *button_p) {
      char x_pos, y_pos;
      if( button_p->anode6 >= 2 ) {
        x_pos = button_p->cathode8;
        y_pos = button_p->anode6 - 3;
      }
      else if( button_p->cathode8 >= 2 ) {
        x_pos = button_p->cathode8 - 8;
        y_pos = button_p->anode6;
      }
      else {
        switch(button_p->cathode8) {
          case -1:
            switch(button_p->anode6) {
              case -1: // Key/Tempo
                key = true;
#ifdef USE_LED
                led_view = &led_key;
                led_key.showNote(key_signature.getNote());
#endif
#ifdef USE_LCD
                lcd.printTempo( metronome.getBpm(), has9 );
                lcd.printKeySignature( key_signature, ! has9 );
#endif
                break;
              case 0: // MIDI Ch.
                ctrl = true;
#ifdef USE_LED
                led_view = &led_ctrl;
#endif
#ifdef USE_LCD
                changeMidiChannel(0);
#endif
                break;
              case 1: // Arpeggiator
                if(key) {
                  if(has9)
                    metronome.setResolution(metronome.getResolution()==4?3:4);
                  else
                    active_note_buttons.toggleHoldMode(ActiveNoteButtons::MODE_ARPEGGIO);
                }
                else
                  active_note_buttons.changePolyMode(ActiveNoteButtons::MODE_ARPEGGIO);
                break;
            }
            break;
          case 0:
            switch(button_p->anode6) {
              case -1: // add9
                has9 = true;
#ifdef USE_LCD
                if(key) {
                  lcd.printTempo( metronome.getBpm(), true );
                  lcd.printKeySignature( key_signature, false );
                }
#endif
                break;
              case  0: // M7
                offset7 -= 1; break;
              case  1: // Rhythm
                if(key) metronome.tap(); else metronome.toggleDrum();
#ifdef USE_LCD
                lcd.printTempo( metronome.getBpm(), false );
#endif
                break;
            }
            break;
          case 1:
            switch(button_p->anode6) {
              case -1: offset5 = -1; break; // -5/+5
              case  0: offset7 -= 2; break; // 7th
              case  1: // Poly/Mono
                if(key)
                  active_note_buttons.toggleHoldMode(ActiveNoteButtons::MODE_POLY);
                else
                  active_note_buttons.changePolyMode(ActiveNoteButtons::MODE_POLY);
                break;
            }
            break;
        }
        return;
      }
      if( key ) {
        if( has9 ) {
          if( x_pos == 0 ) {
            if( y_pos ) metronome.setBpm(metronome.getBpm() + y_pos);
            metronome.sync();
#ifdef USE_LCD
            lcd.printTempo(metronome.getBpm(),true);
#endif
          }
          return;
        }
        if(x_pos) key_signature.shift(x_pos);
        else key_signature.shift( 7 * y_pos, -5, 6 );
#ifdef USE_LED
        led_key.show(key_signature);
#endif
#ifdef USE_LCD
        lcd.printKeySignature( key_signature, true );
#endif
        return;
      }
      if( ctrl ) {
        if( y_pos == 0 ) {
          if( x_pos >= -1 && x_pos <= 1 )
            changeMidiChannel(x_pos);
        }
        else switch(x_pos) {
          case 0: waveform.change(y_pos); break;
          case 1: // Attack
            {
              EnvelopeParam *ep = &(PWM_SYNTH.getChannel(current_midi_channel)->env_param);
              unsigned int *asp = &(ep->attack_speed);
              if( y_pos > 0 ) { if( *asp <= 0x0001 ) *asp = 0x8000; else *asp >>= 1; }
              else { if( *asp >= 0x8000 ) *asp = 0x0001; else *asp <<= 1; }
#ifdef USE_LCD
              lcd.printEnvelope();
#endif
            }
            break;
          case 2: // Decay
            {
              EnvelopeParam *ep = &(PWM_SYNTH.getChannel(current_midi_channel)->env_param);
              byte *dtp = &(ep->decay_time);
              if( y_pos > 0 ) { if( *dtp >= 15 ) *dtp == 0; else (*dtp)++; }
              else { if( *dtp <= 0 ) *dtp = 15; else (*dtp)--; }
#ifdef USE_LCD
              lcd.printEnvelope();
#endif
            }
            break;
          case 3: // Sustain
            {
              EnvelopeParam *ep = &(PWM_SYNTH.getChannel(current_midi_channel)->env_param);
              unsigned int *slp = &(ep->sustain_level);
              if( y_pos > 0 ) *slp +=0x1000; else *slp -=0x1000;
#ifdef USE_LCD
              lcd.printEnvelope();
#endif
            }
            break;
          case 4: // Release
           {
              EnvelopeParam *ep = &(PWM_SYNTH.getChannel(current_midi_channel)->env_param);
              byte *rtp = &(ep->release_time);
              if( y_pos > 0 ) { if( *rtp >= 15 ) *rtp = 0; else (*rtp)++; }
              else { if( *rtp <= 0 ) *rtp = 15; else (*rtp)--; }
#ifdef USE_LCD
              lcd.printEnvelope();
#endif
            }
            break;
        }
        return;
      }
      if( active_note_buttons.isHoldMode(ActiveNoteButtons::MODE_POLY) ) {
        active_note_buttons.deactivateAll();
      }
      if( active_note_buttons.isHoldMode(ActiveNoteButtons::MODE_ARPEGGIO) ) {
        metronome.resetButton();
      }
      // Button activation
      ActiveNoteButton *bnp = active_note_buttons.getFreeEntry();
      if( bnp == NULL ) return;
      // Create chord
      boolean is_aug = (y_pos == 1 && offset5 == -1); // sus4(-5) -> +5
      MusicalChord chord = MusicalChord(
        key_signature, x_pos,
        is_aug?0:y_pos, is_aug?1:offset5, offset7, has9
      );
      size_t n_notes;
      char note_mode = active_note_buttons.getNoteMode();
      if( note_mode == ActiveNoteButtons::MODE_MONO ) {
        n_notes = bnp->getActiveNotes()->activate(
          current_midi_channel,
          chord.getOctaveShiftedNote(y_pos == 1 ? 12 : 0)
        );
        if( n_notes ) bnp->set(button_p);
        return;
      }
      if( note_mode & ActiveNoteButtons::MODE_POLY ) {
        n_notes = bnp->getActiveNotes()->activate(current_midi_channel, &chord);
        if(n_notes) bnp->set(button_p);
      }
      if( note_mode & ActiveNoteButtons::MODE_ARPEGGIO ) metronome.setButton(button_p);
      metronome.setChord(chord);
#ifdef USE_LCD
      lcd.printChord(chord);
#endif
    }
    void released(Button *button_p) {
      if( button_p->cathode8 >= 2 || button_p->anode6 >= 2 ) {
        if( ! active_note_buttons.isHoldMode(ActiveNoteButtons::MODE_POLY) )
          active_note_buttons.deactivate(button_p);
        if( ! active_note_buttons.isHoldMode(ActiveNoteButtons::MODE_ARPEGGIO) )
          metronome.resetButton(button_p);
        return;
      }
      switch(button_p->cathode8) {
        case -1:
          switch(button_p->anode6) {
            case -1: // Key/Tempo
              key = false;
#ifdef USE_LED
              led_view = &led_main;
#endif
#ifdef USE_LCD
              lcd.printKeySignature( key_signature, false );
              if(has9) lcd.printTempo( metronome.getBpm(), false );
#endif
              break;
            case 0: // MIDI Ch.
              ctrl = false;
#ifdef USE_LED
              led_view = &led_main;
#endif
#ifdef USE_LCD
              waveform.show(false);
              lcd.printKeySignature( key_signature, false );
#endif
              break;
          }
          break;
        case 0:
          switch(button_p->anode6) {
            case -1: // add9
              has9 = false;
#ifdef USE_LCD
              if(key) {
                lcd.printTempo( metronome.getBpm(), false );
                lcd.printKeySignature( key_signature, true );
              }
#endif
              break;
            case 0: // M7
              offset7 += 1; break;
          }
          break;
        case 1:
          switch(button_p->anode6) {
            case -1: // -5/+5 
              offset5 = 0; break;
            case 0:  // 7th
              offset7 += 2; break;
          }
          break;
      }
    }
  public:
    Buttons() {
      offset5 = offset7 = 0;
      has9 = ctrl = key = false;
      memset(anode6_bits_array, 0xFF, sizeof(anode6_bits_array));
      led_view = &led_main;
    }
    void scan() {
      Button button = Button();
      button.cathode8 = -1;
      byte portd_value = 0;
      byte *anode6_bits_p = anode6_bits_array;
      byte cathode8_mask = 1;
      byte portc_mask;
      while(cathode8_mask) {
#ifdef USE_LED
        // LED anode to OFF
        portc_mask = 0b11110011; // port 16,17 : bit0
        DDRC  &= portc_mask; // Set INPUT
        PORTC &= portc_mask; // Set pullup flag off (Hi-Z)
#endif // USE_LED
        PORTD &= 0b00011111; // Set port 5,6,7 LOW for 74HC138
        PORTD |= portd_value; // Set 3-bit value
#ifdef USE_LED
        // LED anode to ON
        portc_mask = 0;
        if( led_view->isOn(1,cathode8_mask) ) {
          portc_mask |= 0b00001000; // port 17 : bit1
        }
        if( led_view->isOn(0,cathode8_mask) ) {
          portc_mask |= 0b00000100; // port 16 : bit1
        }
        if( portc_mask ) {
          DDRC  |= portc_mask; // Set OUTPUT
          PORTC |= portc_mask; // Set HIGH
        }
#endif // USE_LED
        byte anode6_change = *anode6_bits_p;
        byte anode6_bits = PINB | 0b11000000;
        anode6_change ^= anode6_bits;
        if( anode6_change ) {
          *anode6_bits_p = anode6_bits;
          button.anode6 = -1;
          byte anode6_mask = 1;
          while( anode6_mask <= 0b00100000 ) {
            if( anode6_change & anode6_mask ) {
              if( anode6_bits & anode6_mask ) released(&button);
              else pressed(&button);
            }
            button.anode6++;
            anode6_mask <<= 1;
          }
        }
        PWM_SYNTH.updateEnvelopeStatus();
        button.cathode8++;
        anode6_bits_p++;
        portd_value += 0b00100000;
        cathode8_mask <<= 1;
      }
    }
};
Buttons buttons = Buttons();

// MIDI IN receive callbacks
#ifdef USE_LED
void HandleNoteOff(byte channel, byte pitch, byte velocity) {
  PWM_SYNTH.noteOff(channel,pitch,velocity);
  noteCounter.noteOff(pitch);
}
#endif
void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  if( velocity == 0 ) {
    HandleNoteOff(channel,pitch,velocity);
    return;
  }
  PWM_SYNTH.noteOn(channel,pitch,velocity);
#ifdef USE_LED
  noteCounter.noteOn(pitch);
#endif
}

void setup() {
#ifdef USE_LCD
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.printKeySignature(key_signature, false);
#endif
  PWM_SYNTH.setup();
  waveform.clear();
  for( int i=1; i<=16; i++ ) {
    EnvelopeParam *ep = &(PWM_SYNTH.getChannel(i)->env_param);
    if( i == 10 ) {
      ep->attack_speed = 0xFFFF;
      ep->decay_time = 5;
      ep->sustain_level = 0;
      ep->release_time = 5;
    }
    else {
      ep->attack_speed = 0x1000;
      ep->decay_time = 10;
      ep->sustain_level = 0;
      ep->release_time = 8;
    }
  }
  DDRB  &= 0b11000000; // port 8..13 INPUT from buttons
  PORTB |= 0b00111111; // and enable internal pullup resistor
  DDRD  |= 0b11100000; // port 5..7 OUTPUT to 74HC138
#ifdef USE_MIDI
#ifdef USE_MIDI_IN
#ifdef USE_LED
  MIDI.setHandleNoteOff(HandleNoteOff);
#else
  MIDI.setHandleNoteOff(PWM_SYNTH.noteOff);
#endif
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandlePitchBend(PWM_SYNTH.pitchBend);
  MIDI.setHandleControlChange(PWM_SYNTH.controlChange);
#endif
  MIDI.begin(MIDI_CHANNEL_OMNI); // receives all MIDI channels
  MIDI.turnThruOff(); // Disable MIDI IN -> MIDI OUT mirroring
  pinMode(MIDI_ENABLE_PIN,OUTPUT);
  digitalWrite(MIDI_ENABLE_PIN,HIGH); // enable MIDI port
#endif
}

void loop() {
  buttons.scan();
#ifdef USE_MIDI_IN
  MIDI.read();
#endif
  metronome.update();
}

