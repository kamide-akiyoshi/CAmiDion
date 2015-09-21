//
// 74HC138 decoder controller
//
class HC138Decoder {
  protected:
    // Arduino port D5..7 (ATMEGA328 port-D higher 3 bits) connected to 74HC138 decoder input
    static const byte PORTD_MASK = 0b11100000;
    static const byte PORTD_STEP = 0b00100000;
    // Bit status after sendOut()
    byte portd_value; // ATMEGA328 port-D output
    byte output;      // Decoder output (/Y7 .. /Y0, LOW active == bit 1)
    byte input;       // Decoder binary input 0b000..0b111 (0..7)
  public:
    void setup() { DDRD |= PORTD_MASK; } // Set pins OUTPUT for 74HC138 input
    void reset() { portd_value = input = 0; output = 1; }
    void sendOut()  { PORTD &= ~PORTD_MASK; PORTD |= portd_value; }
    void next() { portd_value += PORTD_STEP; output <<= 1; input++; }
    byte getOutput() { return output; }
    byte getInput() { return input; }
};

