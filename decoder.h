//
// 74HC138 decoder driver
//
class HC138Decoder {
  protected:
    // Arduino port D5..7 (ATMEGA328 port-D higher 3 bits) connected to 74HC138 decoder input
    static const byte PORTD_MASK = 0b11100000;
    static const byte PORTD_STEP = 0b00100000;
    // Bit status after send()
    byte portd_value; // ATMEGA328 port-D output
    byte output;      // Decoder output (Bit /Y7 .. /Y0, 1 is LOW active)
  public:
    void setup() { DDRD |= PORTD_MASK; } // Set pins OUTPUT for 74HC138 input
    void reset() { portd_value = 0; output = 1; }
    void send()  { PORTD &= ~PORTD_MASK; PORTD |= portd_value; }
    void next() { portd_value += PORTD_STEP; output <<= 1; }
    boolean isSendable() { return output; }
    byte getDecodedOutput() { return output; }
};

