////////////////////////////////////////////////////////////////////
// CAmiDion button matrix (octal index)
//
//    10                          20  21  22
//  00  11    23  24  25  26  27  50  51  52  53  54  55  56  57
//    01  12  13  14  15  16  17  40  41  42  43  44  45  46  47
//      02    03  04  05  06  07  30  31  32  33  34  35  36  37
//
////////////////////////////////////////////////////////////////////
class ButtonHandler {
  public:
    virtual void pressed(byte button_index);
    virtual void released(byte button_index);
};
class ButtonInput {
  protected:
    static const byte PORTB_BUTTON_MASK = 0b00111111; // Arduino port D8..13
    byte waiting_after_off[48];
    byte input_status[8];
  public:
    enum ButtonID {
      KEY,      ADD9,  FLAT5,   Ebm,    Bbm,     Fm,      Cm,      Gm,
      MIDI_CH,  M7,    SEVENTH, Gb,     Db,      Ab,      Eb,      Bb,
      ARPEGGIO, DRUM,  CHORD,   Gbsus4, Dbsus4,  Absus4,  Ebsus4,  Bbsus4,
      Dm,       Am,    Em,      Bm,     Fsharpm, Csharpm, Gsharpm, Dsharpm,
      F,        C,     G,       D,      A,       E,       B,       Fsharp,
      Fsus4,    Csus4, Gsus4,   Dsus4,  Asus4,   Esus4,   Bsus4,   Fsharpsus4,
    };
    boolean isOn(ButtonID bid) { return waiting_after_off[bid]; }
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
          handler->pressed(index);
          *waiting = BUTTON_RELEASE_WAIT_TIMES;
          continue;
        }
        if( *waiting == 0 ) {
          handler->released(index); continue;
        }
        // Cancel bit, and countdown
        *input ^= mask; (*waiting)--;
      }
    }
};

