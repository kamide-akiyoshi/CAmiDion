//
// LCD class for CAmiDion

#if !defined(LCD_ROWS)
#define LCD_ROWS 2
#endif

#if defined(I2CLiquidCrystal_h)
#define LCD_PARENT_CLASS I2CLiquidCrystal
#if !defined(LCD_COLS)
#define LCD_COLS 8
#endif
// LCD_CONSTRUCTOR_ARGS -> contrast(0-63), true:5V / false:3.3V
#if defined(ALKALINE_BATTERY_3)
#define LCD_CONSTRUCTOR_ARGS  (uint8_t)13,(bool)false
#else
#define LCD_CONSTRUCTOR_ARGS  (uint8_t)63,(bool)true
#endif
#elif defined(Lcd74HC164_h)
#define LCD_PARENT_CLASS Lcd74HC164
#define LCD_CONSTRUCTOR_ARGS  18,19,4 // data,clock,enable
#elif defined(LiquidCrystal_h)
#define LCD_PARENT_CLASS LiquidCrystal
#define LCD_CONSTRUCTOR_ARGS  14,15,16,17,18,19 // rs,enable,d4,d5,d6,d7
#endif

#if !defined(LCD_COLS)
#define LCD_COLS 16
#endif

class CAmiDionLCD : public LCD_PARENT_CLASS {
  protected:
    static const char NATURAL = 1;
#if LCD_COLS >= 24
    char line_buf[LCD_COLS + 1];
#else
    char line_buf[24];
#endif
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
    void setup() { begin(LCD_COLS, LCD_ROWS); }
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
    void printKeySignature(KeySignature *ksp, char delimiter = ':') {
      setString("Key",3);
      *bufp++ = delimiter;
      bufp = ksp->print(bufp);
      if( ksp->getCo5() == 0 ) *bufp++ = NATURAL;
#if LCD_COLS >= 15
      *bufp++ = '(';
      bufp = ksp->printSymbol(bufp);
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
    void printChord(MusicalChord *chord) {
      if( current_chord.equals(chord) ) return;
      current_chord = *chord;
#if LCD_COLS >= 15
      setString("Chord:",6);
#endif
      bufp = chord->print(bufp);
      setCursor(0,0);
      printLineBuffer();
    }
    void printTempo(unsigned int bpm, char delimiter = ':') {
#if LCD_COLS >= 15
      setString("Tempo",5);
#else
      *bufp++ = 'T';
#endif
      bufp += sprintf(bufp,"%c%ubpm    ", delimiter, bpm);
      setCursor(0,0);
      printLineBuffer();
      clearChord();
    }
    void printEnvelope(EnvelopeParam *ep) {
      *bufp++ = 'a'; setHex(0xF - PWMDACSynth::log2(ep->attack_speed));
      *bufp++ = 'd'; setHex(ep->decay_time);
      *bufp++ = 's'; setHex(ep->sustain_level >> 12);
      *bufp++ = 'r'; setHex(ep->release_time);
      setCursor(0,1);
      printLineBuffer();
    }
    void printWaveform(byte midi_channel, PROGMEM const byte wavetable[], char delimiter = ':') {
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
      static const char wavename2[]  PROGMEM = "\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02";
      static const char wavename_shepard[] PROGMEM = "Shepard Tone";
      static const char wavename_triangle_shepard[] PROGMEM = "\x02\x03Shepard";
#if defined(OCTAVE_ANALOG_PIN)
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
      static const char wavename23[] PROGMEM = "\x02\x03\x02\x03\x02\x03\x02\x03\x02\x03";
      static const char wavename_triangle[] PROGMEM = "/\x02/\x02/\x02/\x02/\x02";
      static const char wavename_guitar[] PROGMEM = "Guitar";
#endif
      setString("Ch",2);
      if( midi_channel >= 10 ) {
        *bufp++ = '1'; midi_channel -= 10;
      }
      *bufp++ = midi_channel + '0';
      *bufp++ = delimiter;
      const char PROGMEM *wavename;
      if( wavetable == shepardToneSineWavetable ) {
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
#if defined(OCTAVE_ANALOG_PIN)
      else if( wavetable == sawtoothWavetable ) {
        createChar_P( 2, sawtooth_left );
        createChar_P( 3, sawtooth_right );
        wavename = wavename23;
      }
      else if( wavetable == squareWavetable ) {
        createChar_P( 2, square_up );
        createChar_P( 3, square_down );
        wavename = wavename23;
      }
      else if( wavetable == guitarWavetable ) {
        wavename = wavename_guitar;
      }
      else if( wavetable == sineWavetable ) {
        createChar_P( 2, sine_up );
        createChar_P( 3, sine_down );
        wavename = wavename23;
      }
      else if( wavetable == triangleWavetable ) {
        createChar_P( 2, backslash_pattern );
        wavename = wavename_triangle;
      }
#endif
      size_t len = strlen_P(wavename);
      if( len > LCD_COLS ) len = LCD_COLS;
      memcpy_P( bufp, wavename, len );
      bufp += len;
      setCursor(0,0);
      printLineBuffer();
      clearChord();
    }
};


