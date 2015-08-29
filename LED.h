//////////////////////////////
// LED buffer (in HEX)
//
//       3       5 7   A C E
//     0 1 2    4 6 8 9 B D F
//////////////////////////////

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

class NoteCountableLedStatus : public LedStatus {
  protected:
    byte counts[12];
  public:
    NoteCountableLedStatus(unsigned int value16) : LedStatus(value16) {
      memset(counts,0,NumberOf(counts));
    }
    void noteOff(byte pitch) {
      pitch = PWMDACSynth::musicalMod12(pitch);
      if (--counts[pitch] == 0) LedStatus::noteOff(pitch);
    }
    void noteOn(byte pitch) {
      pitch = PWMDACSynth::musicalMod12(pitch);
      if (counts[pitch]++ == 0) LedStatus::noteOn(pitch);
    }
};

class LedViewer {
  protected:
    // LED anode masks
    static const byte PORTC_LED0_MASK = 0b00000100; // Arduino port D16
    static const byte PORTC_LED1_MASK = 0b00001000; // Arduino port D17
    static const byte PORTC_LED_MASK = (PORTC_LED0_MASK | PORTC_LED1_MASK);
    LedStatus *source;
  public:
    LedViewer(LedStatus *source) { setSource(source); }
    void setSource(LedStatus *source) { this->source = source; }
    void ledOff() {
      DDRC  &= ~PORTC_LED_MASK;  // Set INPUT
      PORTC &= ~PORTC_LED_MASK;  // Set pullup flag off (Hi-Z)
    }
    void ledOn(HC138Decoder *decoder) {
      byte portc_mask = 0;
      if( source->isOn(1,decoder->getOutput()) ) portc_mask |= PORTC_LED1_MASK;
      if( source->isOn(0,decoder->getOutput()) ) portc_mask |= PORTC_LED0_MASK;
      if( ! portc_mask ) return;
      DDRC  |= portc_mask; // Set OUTPUT
      PORTC |= portc_mask; // Set HIGH
    }
};

