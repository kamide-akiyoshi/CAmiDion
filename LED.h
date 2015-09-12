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
    unsigned int pitchMask(byte pitch) { return 0x0010 << pitch; }
    void allNotesOff() { leds.value16 &= 0x000F; }
  public:
    enum LedPosition {LEFT, CENTER, RIGHT, UPPER};
    enum LedByte {LOW_BYTE, HIGH_BYTE};
    LedStatus() { this->leds.value16 = 0; }
    boolean isOn(LedByte b, HC138Decoder *d) {
      return leds.values8[b] & d->getOutput();
    }
    void setOn(LedPosition pos)  { sbi(leds.values8[0], pos); }
    void setOff(LedPosition pos) { cbi(leds.values8[0], pos); }
    void noteOn(byte pitch)  { leds.value16 |= pitchMask(pitch); }
    void noteOff(byte pitch) { leds.value16 &= ~pitchMask(pitch); }
    void setMidiChannel(byte ch0) { leds.value16 = 1 << ch0; }
    void setKeySignature(KeySignature *ksp) {
      allNotesOff(); noteOn(ksp->getNote());
    }
};

class NoteCountableLedStatus : public LedStatus {
  protected:
    byte counts[12];
  public:
    NoteCountableLedStatus() : LedStatus() {
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

class LedViewport {
  protected:
    // LED anode masks
    static const byte PORTC_LED0_MASK = 0b00000100; // Arduino port D16
    static const byte PORTC_LED1_MASK = 0b00001000; // Arduino port D17
    static const byte PORTC_LED_MASK = (PORTC_LED0_MASK | PORTC_LED1_MASK);
    LedStatus *source;
  public:
    LedViewport(LedStatus *source) { setSource(source); }
    void setSource(LedStatus *source) { this->source = source; }
    void lightOff() {
      DDRC  &= ~PORTC_LED_MASK;  // Set INPUT
      PORTC &= ~PORTC_LED_MASK;  //  and pullup flag OFF (Hi-Z), LOW when output
    }
    void lightOn(HC138Decoder *decoder) {
      byte portc_mask = 0;
      if( source->isOn(LedStatus::HIGH_BYTE, decoder) ) portc_mask |= PORTC_LED1_MASK;
      if( source->isOn(LedStatus::LOW_BYTE,  decoder) ) portc_mask |= PORTC_LED0_MASK;
      if( ! portc_mask ) return;
      DDRC  |= portc_mask; // Set OUTPUT
      PORTC |= portc_mask; //  and HIGH
    }
};

