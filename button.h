
////////////////////////////////////////////////////////////////////
// CAmiDion button matrix anode6(+1) - cathode8(+1)
//
//   1-0                            2-0 2-1 2-2
// 0-0 1-1    2-3 2-4 2-5 2-6 2-7 | 5-0 5-1 5-2 5-3 5-4 5-5 5-6 5-7
//   0-1 1-2  1-3 1-4 1-5 1-6 1-7 | 4-0 4-1 4-2 4-3 4-4 4-5 4-6 4-7
//     0-2    0-3 0-4 0-5 0-6 0-7 | 3-0 3-1 3-2 3-3 3-4 3-5 3-6 3-7
//
////////////////////////////////////////////////////////////////////
class Button {
  protected:
    static const char NULL_BUTTON = -2;
  public:
    static const byte NUMBER_OF_BUTTONS = 8 * 6;
    static const char LOWER_BOUND = -1;
    char cathode8; // -1,0,1 | 2,3,4,5,6
    char anode6;   // -1,0,1 | 2,3,4
    boolean isNull() { return cathode8 == NULL_BUTTON; }
    boolean isChordButton() { return cathode8 >= 2 || anode6 >= 2; }
    boolean equals(Button *p) {
      return cathode8 == p->cathode8 && anode6 == p->anode6;
    }
    void setNull() { cathode8 = NULL_BUTTON; }
    void setNull(Button *p) { if( equals(p) ) setNull(); }
    void set(Button *p) { cathode8 = p->cathode8; anode6 = p->anode6; }
    byte getSerialNumber() { // returns 0 .. NUMBER_OF_BUTTONS - 1
      return 8 * (cathode8 - LOWER_BOUND) + (anode6 - LOWER_BOUND);
    }
};

class ChordButtonPosition {
  public:
    char x;
    char y;
};

