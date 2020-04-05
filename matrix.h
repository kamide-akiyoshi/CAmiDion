//////////////////////////////////////
// CAmiDion matrix LED buffer in HEX
//
//       3       5 7   A C E
//     0 1 2    4 6 8 9 B D F
//////////////////////////////////////

class LedStatus {
  protected:
    union _LEDs {
      unsigned int value16;
      byte values8[2]; // LEFT_BYTE, RIGHT_BYTE
    } leds;
  public:
    LedStatus() : leds{0} {}
    enum LedByte {LEFT_BYTE, RIGHT_BYTE};
    byte getStatus(const LedByte b) const { return leds.values8[b]; }
};

class MidiChLedStatus : public LedStatus {
  public:
    void setMidiChannel(const byte ch0) { leds.value16 = 1 << ch0; }
};

class NoteLedStatus : public LedStatus {
  protected:
    static const unsigned int NOTE_MASK = 0xFFF0;
    unsigned int pitchMask(const byte pitch) const { return 0x0010 << pitch; }
  public:
    // 4 LEDs which controlled by buttons
    enum LedPosition {LEFT, CENTER, RIGHT, UPPER};
    void setOn(const LedPosition pos)  { sbi(leds.values8[LEFT_BYTE], pos); }
    void setOff(const LedPosition pos) { cbi(leds.values8[LEFT_BYTE], pos); }
    //
    // 12 LEDs to display note
    boolean isNoteOn() const { return leds.value16 & NOTE_MASK; }
    void noteOn(const byte pitch)  { leds.value16 |= pitchMask(pitch); }
    void noteOff(const byte pitch) { leds.value16 &= ~pitchMask(pitch); }
    void allNotesOff() { leds.value16 &= ~NOTE_MASK; }
    void setKeySignature(KeySignature * const ksp) {
      allNotesOff(); noteOn(ksp->getNote());
    }
};

class NoteCountableLedStatus : public NoteLedStatus {
  protected:
    char counts[12];
    void resetCount() { memset(counts,0,NumberOf(counts)); }
  public:
    NoteCountableLedStatus() : NoteLedStatus() { resetCount(); }
    void allNotesOff() {
      resetCount(); NoteLedStatus::allNotesOff();
    }
    void noteOff(byte pitch) {
      pitch = MusicalNote::musicalMod12(pitch);
      if (--counts[pitch] == 0) NoteLedStatus::noteOff(pitch);
    }
    void noteOn(byte pitch) {
      pitch = MusicalNote::musicalMod12(pitch);
      if (counts[pitch]++ == 0) NoteLedStatus::noteOn(pitch);
    }
};

////////////////////////////////////////////////////////////////////
// CAmiDion matrix button ID in octal
//
//    10                          20  21  22
//  00  11    23  24  25  26  27  50  51  52  53  54  55  56  57
//    01  12  13  14  15  16  17  40  41  42  43  44  45  46  47
//      02    03  04  05  06  07  30  31  32  33  34  35  36  37
//
////////////////////////////////////////////////////////////////////
class MatrixScanner {
  public:
    static const byte OUTPUT_PINS = 8;
    static const byte INPUT_PINS = 6;
    static const byte NUMBER_OF_BUTTONS = OUTPUT_PINS * INPUT_PINS;
    static const byte BUTTON_ID_HALF_MASK = 0b00000111;
  protected:
    boolean isButtonOn(const byte button_id) const { return button_instabilities[button_id]; }
    virtual void button_pressed(const byte button_id);
    virtual void button_released(const byte button_id);
  private:
    // Arduino output port D5..7 connected to 74HC138 decoder input
    static const byte PORTD_OFF_MASK     = 0b00011111;
    static const byte PORTD_DECODER_MASK = 0b11100000;
    static const byte PORTD_DECODER_STEP = 0b00100000;
    //
    // Arduino input port D8..13 connected to button
    static const byte PORTB_BUTTON_MASK  = 0b00111111;
    static const byte PORTB_SCAN_MASK    = 0b00000001;
    //
    byte decoder_input;
    byte port_b_inputs[OUTPUT_PINS];
    byte button_instabilities[NUMBER_OF_BUTTONS];
#if defined(USE_LED)
    const LedStatus * led_status_source;
    static const byte PORTC_OFF_MASK  = 0b11110011;
    static const byte PORTC_LED0_MASK = 0b00000100; // Arduino output port D16 connected to left 8 LED anodes
    static const byte PORTC_LED1_MASK = 0b00001000; // Arduino output port D17 connected to right 8 LED anodes
#endif
  public:
    MatrixScanner() {
      decoder_input = 0;
      memset(port_b_inputs, 0xFF, sizeof(port_b_inputs));
      memset(button_instabilities, 0, sizeof(button_instabilities));
    }
    void setup() {
      DDRD  |= PORTD_DECODER_MASK; // Set Pord-D = OUTPUT
      DDRB  &= ~PORTB_BUTTON_MASK; // Set Port-B = INPUT
      PORTB |= PORTB_BUTTON_MASK;  //  and enable internal pullup resistor
    }
#if defined(USE_LED)
    void setLedStatusSource(const LedStatus * const led_status_source) {
      this->led_status_source = led_status_source;
    }
#endif
    void scan() {
      // Update internal status
      ++decoder_input;
      const byte port_d_on_mask = PORTD_DECODER_STEP * (decoder_input &= OUTPUT_PINS - 1);
#if defined(USE_LED)
      // For avoiding flicker, Turn LED anodes OFF first
      DDRC  &= PORTC_OFF_MASK; // INPUT (Hi-Z)
      PORTC &= PORTC_OFF_MASK; // LOW (Pullup resistor OFF)
#endif
      // Update 74HC138 decoder input
      PORTD = (PORTD & PORTD_OFF_MASK) | port_d_on_mask;
#if defined(USE_LED)
      // Wait enough time to 74HC138 decoder output be stable.
      // While waiting it, Make LED masks.
      //
      // Guess the value of 74HC138 decoder output connected to LED cathodes
      // (74HC138 output pin: /Y7 .. /Y0, LOW active == bit 1)
      const byte decoder_output = 1 << decoder_input;
      //
      // Make LED anode mask
      byte port_c_on_mask = 0;
      if( led_status_source->getStatus(LedStatus::LEFT_BYTE) & decoder_output ) port_c_on_mask |= PORTC_LED0_MASK;
      if( led_status_source->getStatus(LedStatus::RIGHT_BYTE) & decoder_output ) port_c_on_mask |= PORTC_LED1_MASK;
      //
      // Turn LED anodes ON
      PORTC |= port_c_on_mask; // HIGH (Pullup resistor ON)
      DDRC  |= port_c_on_mask; // OUTPUT (Lo-Z)
#endif
      // Check status of buttons connected to current activated output line of 74HC138 decoder
      //
      // Check Port-B input change
      byte * const port_b_input = port_b_inputs + decoder_input;
      const byte new_port_b_input = PINB | ~PORTB_BUTTON_MASK;
      const byte change = *port_b_input ^ new_port_b_input;
      if( change == 0 ) return;
      //
      // Check each bit change
      *port_b_input = new_port_b_input;
      byte button_id = decoder_input;
      for( byte mask = PORTB_SCAN_MASK; mask < PORTB_BUTTON_MASK; mask <<= 1, button_id += OUTPUT_PINS ) {
        if( (change & mask) == 0 ) continue;
        //
        // Check whether changed bit is 0(ON) or 1(OFF)
        byte * const instability = button_instabilities + button_id;
        if( (*port_b_input & mask) == 0 ) {
          // Changed OFF -> ON
          button_pressed(button_id);
          *instability = BUTTON_RELEASE_WAIT_TIMES;
          continue;
        }
        // Changed ON -> OFF
        if( *instability > 0 ) {
          // Not stably OFF
          // Back the Port-B input bit to 0(ON), and detect as ON -> OFF change later
          *port_b_input ^= mask;
          // Count down until stably OFF
          (*instability)--;
          continue;
        }
        // Stably OFF
        button_released(button_id);
      }
    }
};
