//
// 74HC138 decoder driver
//
class HC138Decoder {
  protected:
    // Arduino port D5..7 connected to decoder input
    static const byte PORTD_MASK = 0b11100000;
    static const byte PORTD_VALUE_STEP = 0b00100000;
    byte portd_value;
    byte output_buffer;
  public:
    void reset() { portd_value = 0; output_buffer = 1; }
    void setup() { DDRD |= PORTD_MASK; } // Set 1 (OUTPUT)
    void send() {
      PORTD &= ~PORTD_MASK;
      PORTD |= portd_value;
    }
    void next() {
      portd_value += PORTD_VALUE_STEP;
      output_buffer <<= 1;
    }
    byte getOutput() { return output_buffer; }
};

