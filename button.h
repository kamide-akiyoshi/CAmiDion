
////////////////////////////////////////////////////////////////////
// CAmiDion button matrix anode6(+1) - cathode8(+1)
//
//   1-0                            2-0 2-1 2-2
// 0-0 1-1    2-3 2-4 2-5 2-6 2-7 | 5-0 5-1 5-2 5-3 5-4 5-5 5-6 5-7
//   0-1 1-2  1-3 1-4 1-5 1-6 1-7 | 4-0 4-1 4-2 4-3 4-4 4-5 4-6 4-7
//     0-2    0-3 0-4 0-5 0-6 0-7 | 3-0 3-1 3-2 3-3 3-4 3-5 3-6 3-7
//
////////////////////////////////////////////////////////////////////
class MatrixButton {
  public:
    static const byte NUMBER_OF_CATHODE = 8;  // Number of diode cathode line
    static const byte NUMBER_OF_ANODE = 6;    // Number of diode anode line
    static const byte NUMBER_OF_BUTTONS = NUMBER_OF_CATHODE * NUMBER_OF_ANODE;
    static const char LOWER_BOUND = -1;
    static const char NULL_BUTTON = -2;
    char cathode8; // -1,0,1 | 2,3,4,5,6
    char anode6;   // -1,0,1 | 2,3,4
    boolean isNull() { return cathode8 == NULL_BUTTON; }
    boolean equals(MatrixButton *p) { return cathode8 == p->cathode8 && anode6 == p->anode6; }
    void setNull() { cathode8 = NULL_BUTTON; }
    void setNull(MatrixButton *p) { if( equals(p) ) setNull(); }
    void set(MatrixButton *p) { cathode8 = p->cathode8; anode6 = p->anode6; }
};

class AbstractButtonHandler {
  public:
    virtual void pressed(MatrixButton *bp);
    virtual void released(MatrixButton *bp);
};

class ButtonScanner : public MatrixButton {
  protected:
    static const byte PORTB_BUTTON_MASK = 0b00111111; // Arduino port D8..13
    static const byte WAIT_TIMES = 8; // Anti-chattering release wait count
    byte last_inputs[NUMBER_OF_CATHODE];
    byte *last_input_p;
    byte waiting_after_off[NUMBER_OF_BUTTONS];
    byte getIndex() { // returns 0 .. NUMBER_OF_BUTTONS - 1
      return NUMBER_OF_CATHODE * (anode6 - LOWER_BOUND) + (cathode8 - LOWER_BOUND);
    }
  public:
    void setup() {
      DDRB  &= ~PORTB_BUTTON_MASK; // Set 0 (INPUT)
      PORTB |= PORTB_BUTTON_MASK;  // Set 1 to enable internal pullup resistor
      memset(last_inputs, 0b11111111, sizeof(last_inputs));
      memset(waiting_after_off, 0, sizeof(waiting_after_off));
    }
    void reset() { cathode8 = LOWER_BOUND; last_input_p = last_inputs; }
    void next() { cathode8++; last_input_p++; }
    void scan(AbstractButtonHandler *handler) {
      byte input = PINB | ~PORTB_BUTTON_MASK;
      byte change = input ^ *last_input_p;
      if( change == 0 ) return;
      anode6 = LOWER_BOUND;
      for( byte mask = 1; mask < PORTB_BUTTON_MASK; mask <<= 1, anode6++ ) {
        if( (change & mask) == 0 ) continue;
        byte *waiting = waiting_after_off + getIndex();
        if( (input & mask) == 0 ) { handler->pressed(this); *waiting = WAIT_TIMES; continue; }
        if( *waiting == 0 ) { handler->released(this); continue; }
        input ^= mask; (*waiting)--;
      }
      *last_input_p = input;
    }
};

