//
// 74HC138 decoder controller
//
class HC138Decoder {
  protected:
    // Arduino port D5..7 (ATMEGA328 port-D higher 3 bits) connected to 74HC138 decoder input
    static const byte PORTD_MASK = 0b11100000;
    static const byte PORTD_STEP = 0b00100000;
    // Bit status after sendOut()
    byte output; // Decoded output (/Y7 .. /Y0, LOW active == bit 1)
    byte input;  // Decoder binary input 0b000..0b111 (0..7)
#if defined(USE_LED)
    LedViewport *led_viewport;
#endif
    void reset() { input = 0; output = 1; }
  public:
#if defined(USE_LED)
    HC138Decoder(LedViewport *led_viewport) {
      this->led_viewport = led_viewport;
      reset();
    }
#else
    HC138Decoder() { reset(); }
#endif
    void setup() {
      DDRD |= PORTD_MASK; // Set AVR pins OUTPUT for 74HC138 input
    }
    void next() {
#if defined(USE_LED)
      led_viewport->lightOff();
#endif
      if ( (output <<= 1) == 0 ) reset(); else input++;
      PORTD = PORTD & ~PORTD_MASK | PORTD_STEP * input;
#if defined(USE_LED)
      led_viewport->lightOn(output);
#endif
    }
    byte getOutput() { return output; }
    byte getInput() { return input; }
};

