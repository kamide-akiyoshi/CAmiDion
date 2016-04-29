////////////////////////////////////////////////////////////////////
// CAmiDion button matrix (octal index)
//
//    10                          20  21  22
//  00  11    23  24  25  26  27  50  51  52  53  54  55  56  57
//    01  12  13  14  15  16  17  40  41  42  43  44  45  46  47
//      02    03  04  05  06  07  30  31  32  33  34  35  36  37
//
////////////////////////////////////////////////////////////////////
enum ButtonID : byte {
  KEY_BUTTON,
  ADD9_BUTTON,
  FLAT5_BUTTON,
  MIDI_CH_BUTTON = 8,
  M7_BUTTON,
  SEVENTH_BUTTON,
  ARPEGGIO_BUTTON = 16,
  DRUM_BUTTON,
  CHORD_BUTTON,
  ANY_BUTTON = UCHAR_MAX - 1,
  NULL_BUTTON
};
class ButtonHandler {
  public:
    virtual void pressed(ButtonID button_id);
    virtual void released(ButtonID button_id);
};
class ButtonInput {
  protected:
    static const byte PORTB_BUTTON_MASK = 0b00111111; // Arduino port D8..13
    byte waiting_after_off[48];
    byte input_status[8];
  public:
    boolean isOn(ButtonID bid) { return waiting_after_off[(byte)bid]; }
    ButtonInput() {
      memset(waiting_after_off, 0, sizeof(waiting_after_off));
      memset(input_status, 0xFF, sizeof(input_status));
    }
    void setup() {
      DDRB  &= ~PORTB_BUTTON_MASK; // Set 0 (INPUT)
      PORTB |= PORTB_BUTTON_MASK;  // Set 1 to enable internal pullup resistor
    }
    void scan(ButtonHandler *handler, HC138Decoder *decoder) {
      byte new_input = PINB | ~PORTB_BUTTON_MASK;
      byte index = decoder->getInput();
      byte *input = input_status + index;
      byte change = *input ^ new_input;
      if( change == 0 ) return;
      *input = new_input;
      for( byte mask = 1; mask < PORTB_BUTTON_MASK; mask <<= 1, index += 8 ) {
        if( (change & mask) == 0 ) continue;
        byte *waiting = waiting_after_off + index;
        if( (*input & mask) == 0 ) {
          handler->pressed((ButtonID)index);
          *waiting = BUTTON_RELEASE_WAIT_TIMES;
          continue;
        }
        if( *waiting == 0 ) {
          handler->released((ButtonID)index);
          continue;
        }
        // Cancel bit, and countdown
        *input ^= mask; (*waiting)--;
      }
    }
};

