
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
    byte last_input_bits[8];
    void (*pressed)(byte);
    void (*released)(byte);
  public:
    static const byte INDEX_OF_KEY      = 0;
    static const byte INDEX_OF_ADD9     = 1;
    static const byte INDEX_OF_FLAT5    = 2;
    static const byte INDEX_OF_MIDI_CH  = 8;
    static const byte INDEX_OF_M7       = 9;
    static const byte INDEX_OF_7        = 10;
    static const byte INDEX_OF_ARPEGGIO = 16;
    static const byte INDEX_OF_DRUM     = 17;
    static const byte INDEX_OF_CHORD    = 18;
    boolean isKeyOn()    { return ~last_input_bits[0] & (1<<0); }
    boolean isAdd9On()   { return ~last_input_bits[1] & (1<<0); }
    boolean isFlat5On()  { return ~last_input_bits[2] & (1<<0); }
    boolean isMidiChOn() { return ~last_input_bits[0] & (1<<1); }
    boolean isM7On()     { return ~last_input_bits[1] & (1<<1); }
    boolean is7On()      { return ~last_input_bits[2] & (1<<1); }
    ButtonInput() {
      memset(waiting_after_off, 0, sizeof(waiting_after_off));
      memset(last_input_bits, 0xFF, sizeof(last_input_bits));
      pressed = released = NULL;
    }
    void setup() {
      DDRB  &= ~PORTB_BUTTON_MASK; // Set 0 (INPUT)
      PORTB |= PORTB_BUTTON_MASK;  // Set 1 to enable internal pullup resistor
    }
    void setHandlePressed(void (*handler)(byte)) { pressed = handler; }
    void setHandleReleased(void (*handler)(byte)) { released = handler; }
    void scan(HC138Decoder *decoder) {
      byte new_input = PINB | ~PORTB_BUTTON_MASK;
      byte index = decoder->getInput();
      byte *last = last_input_bits + index;
      byte change = new_input ^ *last;
      if( change == 0 ) return;
      for( byte mask = 1; mask < PORTB_BUTTON_MASK; mask <<= 1, index += 8 ) {
        if( (change & mask) == 0 ) continue;
        byte *waiting = waiting_after_off + index;
        if( (new_input & mask) == 0 ) {
          if (pressed != NULL) pressed(index);
          *waiting = WAIT_TIMES; continue;
        }
        if( *waiting == 0 ) {
          if (pressed != NULL) released(index);
          continue;
        }
        new_input ^= mask; (*waiting)--;
      }
      *last = new_input;
    }
};

