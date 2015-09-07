
////////////////////////////////////////////////////////////////////
// CAmiDion button matrix (octal index)
//
//   10                          20  21  22
// 00  11    23  24  25  26  27  50  51  52  53  54  55  56  57
//   01  12  13  14  15  16  17  40  41  42  43  44  45  46  47
//     02    03  04  05  06  07  30  31  32  33  34  35  36  37
//
////////////////////////////////////////////////////////////////////

class ButtonStatus {
  protected:
    byte last_inputs[8];
  public:
    static const byte INDEX_OF_KEY   = 0;
    static const byte INDEX_OF_ADD9  = 1;
    static const byte INDEX_OF_FLAT5 = 2;
    boolean isKeyOn()    { return ~last_inputs[0] & (1<<0); }
    boolean isAdd9On()   { return ~last_inputs[1] & (1<<0); }
    boolean isFlat5On()  { return ~last_inputs[2] & (1<<0); }
    static const byte INDEX_OF_MIDI_CH = 8;
    static const byte INDEX_OF_M7      = 9;
    static const byte INDEX_OF_7       = 10;
    boolean isMidiChOn() { return ~last_inputs[0] & (1<<1); }
    boolean isM7On()     { return ~last_inputs[1] & (1<<1); }
    boolean is7On()      { return ~last_inputs[2] & (1<<1); }
    static const byte INDEX_OF_ARPEGGIO = 16;
    static const byte INDEX_OF_DRUM     = 17;
    static const byte INDEX_OF_CHORD    = 18;
};

class AbstractButtonHandler {
  public:
    virtual void pressed(ButtonStatus *button_status, byte button_index);
    virtual void released(ButtonStatus *button_status, byte button_index);
};

class ButtonScanner : public ButtonStatus {
  protected:
    static const byte PORTB_BUTTON_MASK = 0b00111111; // Arduino port D8..13
    static const byte WAIT_TIMES = 8; // Anti-chattering release wait count
    byte waiting_after_off[48];
  public:
    void setup() {
      DDRB  &= ~PORTB_BUTTON_MASK; // Set 0 (INPUT)
      PORTB |= PORTB_BUTTON_MASK;  // Set 1 to enable internal pullup resistor
      memset(last_inputs, 0xFF, sizeof(last_inputs));
      memset(waiting_after_off, 0, sizeof(waiting_after_off));
    }
    void scan(AbstractButtonHandler *handler, HC138Decoder *decoder) {
      byte input = PINB | ~PORTB_BUTTON_MASK;
      byte decoder_input = decoder->getInput();
      byte change = input ^ last_inputs[decoder_input];
      if( change == 0 ) return;
      byte local_index = 0;
      for( byte mask = 1; mask < PORTB_BUTTON_MASK; mask <<= 1, local_index++ ) {
        if( (change & mask) == 0 ) continue;
        byte button_index = 8 * local_index + decoder_input;
        byte *waiting = waiting_after_off + button_index;
        if( (input & mask) == 0 ) {
          handler->pressed(this, button_index); *waiting = WAIT_TIMES; continue;
        }
        if( *waiting == 0 ) {
          handler->released(this, button_index); continue;
        }
        input ^= mask; (*waiting)--;
      }
      last_inputs[decoder->getInput()] = input;
    }
};

