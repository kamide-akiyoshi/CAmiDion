////////////////////////////////////////////////////////////////////
// CAmiDion button matrix (octal index)
//
//   10                          20  21  22
// 00  11    23  24  25  26  27  50  51  52  53  54  55  56  57
//   01  12  13  14  15  16  17  40  41  42  43  44  45  46  47
//     02    03  04  05  06  07  30  31  32  33  34  35  36  37
//
////////////////////////////////////////////////////////////////////
class ButtonInput {
  protected:
    static const byte WAIT_TIMES = 16; // Anti-chattering release wait count
    static const byte PORTB_BUTTON_MASK = 0b00111111; // Arduino port D8..13
    byte waiting_after_off[48];
    byte input_status[8];
    void (*pressed)(byte);
    void (*released)(byte);
  public:
    static const byte INDEX_OF_KEY      = 0;
    static const byte INDEX_OF_ADD9     = 1;
    static const byte INDEX_OF_MIDI_CH  = 8;
    static const byte INDEX_OF_ARPEGGIO = 16;
    static const byte INDEX_OF_DRUM     = 17;
    static const byte INDEX_OF_CHORD    = 18;
    boolean isKeyOn()    { return ~input_status[0] & (1<<0); }
    boolean isAdd9On()   { return ~input_status[1] & (1<<0); }
    boolean isFlat5On()  { return ~input_status[2] & (1<<0); }
    boolean isMidiChOn() { return ~input_status[0] & (1<<1); }
    boolean isM7On()     { return ~input_status[1] & (1<<1); }
    boolean is7On()      { return ~input_status[2] & (1<<1); }
    ButtonInput(void (*pressed)(byte), void (*released)(byte)) {
      memset(waiting_after_off, 0, sizeof(waiting_after_off));
      memset(input_status, 0xFF, sizeof(input_status));
      this->pressed = pressed;
      this->released = released;
    }
    void setup() {
      DDRB  &= ~PORTB_BUTTON_MASK; // Set 0 (INPUT)
      PORTB |= PORTB_BUTTON_MASK;  // Set 1 to enable internal pullup resistor
    }
    void scan(HC138Decoder *decoder) {
      byte new_input = PINB | ~PORTB_BUTTON_MASK;
      byte index = decoder->getInput();
      byte *input = input_status + index;
      byte change = *input ^ new_input;
      if( change == 0 ) return;
      *input = new_input;
      for( byte mask = 1; mask < PORTB_BUTTON_MASK; mask <<= 1, index += 8 ) {
        if( (change & mask) == 0 ) continue;
        byte *waiting = waiting_after_off + index;
        if( (*input & mask) == 0 ) { pressed(index); *waiting = WAIT_TIMES; continue; }
        if( *waiting ) { *input ^= mask; (*waiting)--; continue; }
        released(index);
      }
    }
};

