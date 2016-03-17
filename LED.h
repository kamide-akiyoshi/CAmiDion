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
    LedStatus() { this->leds.value16 = 0; }
    enum LedByte {LEFT_BYTE, RIGHT_BYTE};
    boolean isBitOn(LedByte b, byte decoder_output) {
      return leds.values8[b] & decoder_output;
    }
};

class MidiChLedStatus : public LedStatus {
  public:
    void setMidiChannel(byte ch0) { leds.value16 = 1 << ch0; }
};

class NoteLedStatus : public LedStatus {
  protected:
    static const unsigned int NOTE_MASK = 0xFFF0;
    unsigned int pitchMask(byte pitch) { return 0x0010 << pitch; }
  public:
    // 4 LEDs which controlled by buttons
    enum LedPosition {LEFT, CENTER, RIGHT, UPPER};
    void setOn(LedPosition pos)  { sbi(leds.values8[LEFT_BYTE], pos); }
    void setOff(LedPosition pos) { cbi(leds.values8[LEFT_BYTE], pos); }
    //
    // 12 LEDs to display note
    boolean isNoteOn() { return leds.value16 & NOTE_MASK; }
    void noteOn(byte pitch)  { leds.value16 |= pitchMask(pitch); }
    void noteOff(byte pitch) { leds.value16 &= ~pitchMask(pitch); }
    void allNotesOff() { leds.value16 &= ~NOTE_MASK; }
    void setKeySignature(KeySignature *ksp) {
      allNotesOff(); noteOn(ksp->getNote());
    }
};

class NoteCountableLedStatus : public NoteLedStatus {
  protected:
    char counts[12];
    void resetCount() { memset(counts,0,NumberOf(counts)); }
  public:
    NoteCountableLedStatus() : NoteLedStatus() { resetCount(); }
    void noteOff(byte pitch) {
      pitch = PWMDACSynth::musicalMod12(pitch);
      if (--counts[pitch] == 0) NoteLedStatus::noteOff(pitch);
    }
    void noteOn(byte pitch) {
      pitch = PWMDACSynth::musicalMod12(pitch);
      if (counts[pitch]++ == 0) NoteLedStatus::noteOn(pitch);
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
      DDRC  &= ~PORTC_LED_MASK; // INPUT (Hi-Z)
      PORTC &= ~PORTC_LED_MASK; // LOW (pullup-R off)
    }
    void lightOn(byte decoder_output) {
      byte portc_mask = 0;
      if( source->isBitOn(LedStatus::LEFT_BYTE,  decoder_output) ) portc_mask |= PORTC_LED0_MASK;
      if( source->isBitOn(LedStatus::RIGHT_BYTE, decoder_output) ) portc_mask |= PORTC_LED1_MASK;
      if( ! portc_mask ) return; // Both bit 0, keep light off
      PORTC |= portc_mask; // HIGH (pullup-R on)
      DDRC  |= portc_mask; // OUTPUT (Lo-Z)
    }
};

