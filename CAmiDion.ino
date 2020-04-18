//
// CAmiDion - Musical Chord Instrument
//  ver.20200418
//  by Akiyoshi Kamide (Twitter: @akiyoshi_kamide)
//  http://camidion.wordpress.com/camidion/
//  http://osdn.net/users/kamide/pf/CAmiDion/
//  http://www.yk.rim.or.jp/~kamide/music/chordhelper/hardware/
//

#include "CAmiDionConfig.h"

#include <PWMDAC_Synth.h>
extern const byte randomWavetable[] PROGMEM;
extern const byte shepardToneSawtoothWavetable[] PROGMEM;
PWMDAC_CREATE_WAVETABLE(shepardToneSineWavetable, PWMDAC_SHEPARD_TONE);
#if defined(OCTAVE_ANALOG_PIN)
extern const byte guitarWavetable[] PROGMEM;
PWMDAC_CREATE_WAVETABLE(sawtoothWavetable, PWMDAC_SAWTOOTH_WAVE);
PWMDAC_CREATE_WAVETABLE(squareWavetable, PWMDAC_SQUARE_WAVE);
PWMDAC_CREATE_WAVETABLE(triangleWavetable, PWMDAC_TRIANGLE_WAVE);
PWMDAC_CREATE_WAVETABLE(sineWavetable, PWMDAC_SINE_WAVE);
#endif
extern const Instrument INSTRUMENTS[] PROGMEM;
PWMDAC_CREATE_INSTANCE(INSTRUMENTS);

#if defined(USE_MIDI)
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

#include "musicalnote.h"
#include "matrix.h"

#if defined(USE_LED)
NoteCountableLedStatus led_main;
NoteLedStatus led_key;
MidiChLedStatus led_ctrl;
#endif

#if defined(USE_LCD)
#if defined(USE_LCD_VIA_I2C)
#include <Wire.h>
#include <I2CLiquidCrystal.h>
#elif defined(USE_LCD_VIA_74164)
#include <LcdCore.h>
#include <Lcd74HC164.h>
#elif defined(USE_LCD_VIA_PARALLEL)
#include <LiquidCrystal.h>
#endif
#include "CAmiDionLCD.h"
CAmiDionLCD lcd;
#endif

const byte * const WAVETABLE_LIST[] PROGMEM = {
#if defined(OCTAVE_ANALOG_PIN)
  sawtoothWavetable,
  squareWavetable,
  guitarWavetable,
  sineWavetable,
  triangleWavetable,
#endif
  shepardToneSineWavetable,
  shepardToneSawtoothWavetable,
  randomWavetable,
};
const Instrument DRUM_INSTRUMENT PROGMEM = {randomWavetable, {5, 0, 5, 0}};

class WaveSelecter {
  protected:
    byte current_midi_channel; // 1==CH1
    byte programs[0x10]; // 0[0]==Prog1[CH1]
  public:
    WaveSelecter() {
      PWMDACSynth::getChannel(DRUM_MIDI_CHANNEL)->programChange(&DRUM_INSTRUMENT);
      current_midi_channel = 1;
#ifdef USE_LED
      led_ctrl.setMidiChannel(0);
#endif
      memset(programs, 0, sizeof(programs));
    }
#ifdef USE_LCD
    void showWaveform(const char delimiter) {
      lcd.printWaveform(
        current_midi_channel,
        PWMDACSynth::getChannel(current_midi_channel)->wavetable,
        delimiter
      );
    }
    void showEnvelope() {
      lcd.printEnvelope(PWMDACSynth::getChannel(current_midi_channel)->envelope);
    }
#endif
    byte getCurrentMidiChannel() const { return current_midi_channel; }
    void changeCurrentMidiChannel(const char offset) {
      char ch0 = current_midi_channel + offset - 1;
      current_midi_channel = (ch0 &= 0xF) + 1;
#ifdef USE_LED
      led_ctrl.setMidiChannel(ch0);
#endif
#ifdef USE_LCD
      showWaveform('>');
      showEnvelope();
#endif
    }
    void changeWaveform(const char offset) {
      if( ! offset ) return;
      MidiChannel *cp = PWMDACSynth::getChannel(current_midi_channel);
      for( char i = 0; (byte)i < NumberOf(WAVETABLE_LIST); ++i ) {
        if( (const byte *)pgm_read_word(WAVETABLE_LIST + i) != cp->wavetable ) continue;
        if ( (i += offset) < 0) i += NumberOf(WAVETABLE_LIST);
        else if ((byte)i >= NumberOf(WAVETABLE_LIST)) i -= NumberOf(WAVETABLE_LIST);
        cp->wavetable = (const byte *)pgm_read_word(WAVETABLE_LIST + i);
#ifdef USE_LCD
        showWaveform('>');
#endif
        return;
      }
    }
    void changeEnvelope(const AdsrParam adsr_param, const char offset) {
      byte *p = PWMDACSynth::getChannel(current_midi_channel)->envelope + adsr_param;
      if( adsr_param == ADSR_SUSTAIN_VALUE ) *p += offset * 0x10; // 0..F -> 0..FF
      else { *p += offset; *p &= 0xF; }
#ifdef USE_LCD
      showEnvelope();
#endif
    }
    byte getProgram(const byte channel) const { return programs[channel - 1]; }
    byte getProgram() const { return getProgram(current_midi_channel); }
    void programChange(const byte channel, const byte program) {
      if( channel == DRUM_MIDI_CHANNEL ) return;
      byte * const pp = programs + channel - 1;
      *pp = program & 0x7F;
#ifdef USE_LCD
      if( channel == current_midi_channel ) lcd.printProgram(*pp);
#endif
#if defined(OCTAVE_ANALOG_PIN)
      PWMDACSynth::getChannel(channel)->programChange(INSTRUMENTS + *pp);
#endif
#ifdef USE_MIDI_OUT
      MIDI.sendProgramChange(*pp, channel);
#endif      
    }
    void programChange(const byte program) {
      programChange(current_midi_channel, program);
    }
};

WaveSelecter wave_selecter;

// MIDI IN receive callbacks
//
void HandleNoteOff(const byte channel, const byte pitch, const byte velocity) {
  PWMDACSynth::noteOff(channel,pitch,velocity);
#ifdef USE_LED
  led_main.noteOff(pitch);
#endif
}
void HandleNoteOn(const byte channel, const byte pitch, const byte velocity) {
  if( velocity == 0 ) {
    HandleNoteOff(channel,pitch,velocity);
    return;
  }
  PWMDACSynth::noteOn(channel,pitch,velocity);
#ifdef USE_LED
  led_main.noteOn(pitch);
#endif
}

#if defined(OCTAVE_ANALOG_PIN)
void HandleProgramChange(const byte channel, const byte number) {
  wave_selecter.programChange(channel, number);
}
#endif

void HandleSystemReset() {
  PWMDACSynth::systemReset();
  // In PWMDAC Synth lib, All MIDI channel will be reset to same waveform/envelope even ch#10.
  // So ensure to set drum instrument to ch#10.
  PWMDACSynth::getChannel(DRUM_MIDI_CHANNEL)->programChange(&DRUM_INSTRUMENT);
#ifdef USE_LED
  led_main.allNotesOff();
#endif
}

// System Exclusive to reset, without first byte(0xF0)/last byte(0xF7)
const byte GM_SYSTEM_ON[] PROGMEM = {
  0x7E, // Universal
  0x7F, 0x09, 0x01
};
const byte GS_SYSTEM_ON[] PROGMEM = {
  0x41, // Roland
  0x00, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41
};
const byte XG_SYSTEM_ON[] PROGMEM = {
  0x43, // YAMAHA
  0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00
}; 
void HandleSystemExclusive(byte *array, const unsigned size) {
  array++;
  const unsigned sz = size - 2;
  if( memcmp_P(array, GM_SYSTEM_ON, sz) == 0 ) {
    HandleSystemReset(); return;
  }
  array[1] &= 0xF0; // Clear lower 4-bits of device ID (0x1n -> 0x10)
  if( memcmp_P(array, XG_SYSTEM_ON, sz) == 0 ) {
    HandleSystemReset(); return;
  }
  array[1] = 0; // Clear device ID (0xnn -> 0x00)
  if( memcmp_P(array, GS_SYSTEM_ON, sz) == 0 ) {
    HandleSystemReset(); return;
  }
}

enum ButtonID : byte {
  KEY_BUTTON,
  ADD9_BUTTON,
  FLAT5_BUTTON,

  MIDI_CH_BUTTON = MatrixScanner::OUTPUT_PINS,
  M7_BUTTON,
  SEVENTH_BUTTON,

  ARPEGGIO_BUTTON = 2 * MatrixScanner::OUTPUT_PINS,
  DRUM_BUTTON,
  CHORD_BUTTON,

  ANY_BUTTON = UCHAR_MAX - 1,
  NULL_BUTTON
};

class NoteSender {
  protected:
    ButtonID button_id;
    byte midi_channel;
    void sendNoteOff(const char note, const char velocity) {
      HandleNoteOff(midi_channel, note, velocity);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOff(note, velocity, midi_channel);
#endif
    }
    void sendNoteOn(const char note, const char velocity) {
      HandleNoteOn(midi_channel, note, velocity);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOn(note, velocity, midi_channel);
#endif
    }
};

class PolyNoteSender : public NoteSender {
  protected:
    size_t getSizeOf(const char note) const { (void)note; return 1; }
    size_t getSizeOf(const MusicalChord * const chord) const { return chord->MAX_NOTES; }
    size_t n_notes;
    char *notes;
    char *setNoteValueOf(const char note) { *notes = note; return notes; }
    char *setNoteValueOf(MusicalChord * const chord) { chord->toNotes(notes); return notes; }
  public:
    PolyNoteSender() { n_notes = 0; }
    template <typename Note> size_t noteOn(const ButtonID button_id, const byte midi_channel, const Note note) {
      if( this->n_notes ) return 0;
      const size_t n_notes = getSizeOf(note);
      if( (notes = (char *)malloc(sizeof(char) * n_notes)) == NULL ) return 0;
      this->n_notes = n_notes;
      this->button_id = button_id;
      this->midi_channel = midi_channel;
      char *p = setNoteValueOf(note);
      for( size_t n = n_notes; n>0; n-- ) sendNoteOn(*p++, NOTE_VELOCITY);
      return n_notes;
    }
    boolean noteOff(const ButtonID button_id = ANY_BUTTON) {
      if( ! n_notes || (button_id != ANY_BUTTON && this->button_id != button_id) )
        return false;
      char *p = notes;
      for( size_t n = n_notes; n>0; n-- ) sendNoteOff(*p++, NOTE_VELOCITY);
      free(notes); n_notes = 0; return true;
    }
};

class Arpeggiator : public NoteSender {
  protected:
    static const byte MODE_ENABLE = (1<<0);
    static const byte MODE_HOLD = (1<<1);
    byte mode;
    static const char NULL_NOTE = -1;
    char current_note;
    MusicalChord chord;
  public:
    Arpeggiator() {
      button_id = NULL_BUTTON;
      mode = MODE_HOLD;
      current_note = NULL_NOTE;
#ifdef USE_LED
      led_key.setOn(NoteLedStatus::LEFT);
#endif
    }
    boolean isEnabled() const { return mode & MODE_ENABLE; }
    void noteOff() {
      if( current_note == NULL_NOTE ) return;
      sendNoteOff(current_note, ARPEGGIO_VELOCITY);
      current_note = NULL_NOTE;
    }
    void noteOn() {
      if( this->button_id == NULL_BUTTON ) return;
      sendNoteOn(current_note = chord.getRandomNote(), ARPEGGIO_VELOCITY);
    }
#ifdef USE_LED
    void beatLedOff() { if (isEnabled()) led_main.setOff(NoteLedStatus::LEFT); }
    void beatLedOn()  { if (isEnabled()) led_main.setOn(NoteLedStatus::LEFT); }
#endif
    void pressed(const ButtonID button_id, const char midi_channel, const MusicalChord * const chord) {
      if( ! isEnabled() ) return;
      this->button_id = button_id;
      this->midi_channel = midi_channel;
      this->chord = *chord;
    }
    void released(const ButtonID button_id) {
      if( mode & MODE_HOLD || this->button_id != button_id) return;
      this->button_id = NULL_BUTTON;
    }
    void forceRelease() {
      if( mode & MODE_HOLD ) button_id = NULL_BUTTON;
    }
    void toggleEnabled() {
      mode ^= MODE_ENABLE;
#ifdef USE_LCD
      lcd.setCursor(0,0);
#endif
      if ( isEnabled() ) {
#ifdef USE_LED
        led_main.setOn(NoteLedStatus::LEFT);
#endif
#ifdef USE_LCD
        lcd.printChord();
#endif
      } else {
        button_id = NULL_BUTTON;
#ifdef USE_LED
        led_main.setOff(NoteLedStatus::LEFT);
#endif
#ifdef USE_LCD
        wave_selecter.showWaveform(':');
#endif
      }
    }
    void toggleHoldMode() {
      mode ^= MODE_HOLD;
      if ( mode & MODE_HOLD ) {
#ifdef USE_LED
        led_key.setOn(NoteLedStatus::LEFT);
#endif
      } else {
        button_id = NULL_BUTTON;
#ifdef USE_LED
        led_key.setOff(NoteLedStatus::LEFT);
#endif
      }
    }
};

class Drum {
  protected:
    char note;
    boolean is_enabled;
#ifdef USE_LED
    void ledOn() {
      led_main.setOn(NoteLedStatus::CENTER);
      led_key.setOn(NoteLedStatus::CENTER);
    }
    void ledOff() {
      led_main.setOff(NoteLedStatus::CENTER);
      led_key.setOff(NoteLedStatus::CENTER);
    }
#endif
  public:
    Drum() { is_enabled = false; note = -1; }
    void toggleEnabled() { is_enabled = ! is_enabled;
#ifdef USE_LED
      if( is_enabled ) ledOn(); else ledOff();
#endif
    }
    void noteOff() {
      if( note < 0 ) return;
      PWMDACSynth::noteOff(DRUM_MIDI_CHANNEL, note, DRUM_VELOCITY);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOff(DRUM_NOTE_NUMBER, DRUM_VELOCITY, DRUM_MIDI_CHANNEL);
#endif
      note = -1;
#ifdef USE_LED
      ledOff();
#endif
    }
    void noteOn() {
      if( ! is_enabled ) return;
      PWMDACSynth::noteOn(DRUM_MIDI_CHANNEL, note = 0, DRUM_VELOCITY);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOn(DRUM_NOTE_NUMBER, DRUM_VELOCITY, DRUM_MIDI_CHANNEL);
#endif
#ifdef USE_LED
      ledOn();
#endif
    }
};

Arpeggiator arpeggiator;

class NoteSenders {
  protected:
    static const byte MAX_BUTTONS = 8;
    PolyNoteSender senders[MAX_BUTTONS];
    void noteOff() {
      for( byte i=0; i < NumberOf(senders); ++i ) senders[i].noteOff();
    }
    void noteOff(const ButtonID button_id) {
      for( byte i=0; i < NumberOf(senders); ++i ) if(senders[i].noteOff(button_id)) break;
    }
    template <typename T> void noteOn(const ButtonID button_id, const byte midi_channel, const T note) {
      for( byte i=0; i < NumberOf(senders); ++i ) if(senders[i].noteOn(button_id, midi_channel, note)) break;
    }
    static const byte MODE_POLY = (1<<0);
    static const byte MODE_HOLD = (1<<1);
    byte mode;
  public:
    NoteSenders() {
      mode = MODE_POLY;
#ifdef USE_LED
      led_main.setOn(NoteLedStatus::RIGHT);
#endif
    }
    void pressed(const ButtonID button_id, const byte midi_channel, MusicalChord * const chord) {
      if( mode & MODE_HOLD ) noteOff();
      arpeggiator.forceRelease();
      if( mode & MODE_POLY ) {
        noteOn(button_id, midi_channel, chord);
      } else if( ! arpeggiator.isEnabled() ) {
        noteOn(button_id, midi_channel, chord->getOctaveShiftedNote(chord->isSus4() ? 12 : 0));
        return;
      }
      arpeggiator.pressed(button_id, midi_channel, chord);
#ifdef USE_LCD
      lcd.printChord(chord);
#endif
    }
    void released(ButtonID button_id) {
      if( !(mode & MODE_HOLD) ) noteOff(button_id);
      arpeggiator.released(button_id);
    }
    void togglePolyMode() {
      mode ^= MODE_POLY;
#ifdef USE_LCD
      lcd.setCursor(0,0);
#endif
      if ( mode & MODE_POLY ) {
#ifdef USE_LED
        led_main.setOn(NoteLedStatus::RIGHT);
#endif
#ifdef USE_LCD
        lcd.printChord();
#endif
      } else {
#ifdef USE_LED
        led_main.setOff(NoteLedStatus::RIGHT);
#endif
#ifdef USE_LCD
        wave_selecter.showWaveform(':');
#endif
      }
    }
    void toggleHoldMode() {
      mode ^= MODE_HOLD;
      if ( mode & MODE_HOLD ) {
#ifdef USE_LED
        led_key.setOn(NoteLedStatus::RIGHT);
#endif
      }
      else {
        noteOff();
#ifdef USE_LED
        led_key.setOff(NoteLedStatus::RIGHT);
#endif
      }
    }
};

Drum drum;

#define US_PER_MINUTE  (60000000ul) // microseconds per minute
class Metronome {
  protected:
    static const byte COUNT_PER_QUARTER = 12;
    static const unsigned long US_TAP_INTERVAL_LIMIT = US_PER_MINUTE / 40; // 1500 ms
    unsigned long us_timeout;
    unsigned long us_interval;
    unsigned long us_last_tapped;
    unsigned long us_per_quarter;
    byte arpeggiator_period;
    byte clock_count;
    byte midi_clock_count;
  public:
    Metronome() {
      us_interval = (us_per_quarter = US_PER_MINUTE / 120) / COUNT_PER_QUARTER;
      us_last_tapped = ULONG_MAX;
      arpeggiator_period = 3;
      clock_count = 0;
      us_timeout = micros();
    }
    void toggleResolution() { arpeggiator_period ^= 7; } // 100(4) <-> 011(3) by XOR 111(7)
    unsigned int getBpm() const {
      return (unsigned int)((double)US_PER_MINUTE / (double)us_per_quarter + 0.5);
    }
    void shiftBpm(const char offset) {
      unsigned int bpm = getBpm() + offset;
      us_interval = (us_per_quarter = US_PER_MINUTE / bpm) / COUNT_PER_QUARTER;
#ifdef USE_LCD
      lcd.printTempo(bpm,'>');
#endif
    }
    void sync() { us_timeout = micros(); clock_count = 0; }
    void tap(const byte clocks_per_quarter = 1) {
      unsigned long us_diff = micros() - us_last_tapped;
      if( us_diff < US_TAP_INTERVAL_LIMIT ) {
        if( ++midi_clock_count < clocks_per_quarter ) return;
        us_interval = (us_per_quarter = us_diff) / COUNT_PER_QUARTER;
      }
      us_timeout = (us_last_tapped += us_diff);
      midi_clock_count = clock_count = 0;
#ifdef USE_LCD
      lcd.printTempo(getBpm(),'>');
#endif
    }
    void update() {
      if( micros() < us_timeout ) return;
      if( clock_count % arpeggiator_period == 0 ) {
        arpeggiator.noteOff();
        arpeggiator.noteOn();
        if(clock_count == 0) {
          drum.noteOn();
#ifdef USE_LED
          led_main.setOn(NoteLedStatus::UPPER);
          led_key.setOn(NoteLedStatus::UPPER);
          arpeggiator.beatLedOn();
#endif
        }
        if(clock_count == arpeggiator_period) {
          drum.noteOff();
#ifdef USE_LED
          led_main.setOff(NoteLedStatus::UPPER);
          led_key.setOff(NoteLedStatus::UPPER);
          arpeggiator.beatLedOff();
#endif
        }
      }
      if(++clock_count >= COUNT_PER_QUARTER) clock_count = 0;
      us_timeout += us_interval;
    }
};

Metronome metronome;
void HandleClock() { metronome.tap(24); }

KeySignature key_signature;
NoteSenders note_senders;

class MatrixButtons : public MatrixScanner {
  public:
    void button_pressed(const byte button_id) {
      char x = button_id & BUTTON_ID_HALF_MASK;
      char y = (button_id >> 3) & BUTTON_ID_HALF_MASK;
      if( y >= 3 ) { x--; y -= 4; }
      else if( x >= 3 ) { x -= 9; y--; }
      else {
        switch(button_id) {
          case KEY_BUTTON:
#ifdef USE_LED
            led_key.setKeySignature(&key_signature);
            setLedStatusSource(&led_key);
#endif
#ifdef USE_LCD
            lcd.printTempo(metronome.getBpm(), isButtonOn(ADD9_BUTTON)?'>':':');
            lcd.printKeySignature(&key_signature, isButtonOn(ADD9_BUTTON)?':':'>');
#endif
            break;
          case ADD9_BUTTON:
#ifdef USE_LCD
            if( isButtonOn(KEY_BUTTON) ) {
              lcd.printTempo(metronome.getBpm(),'>');
              lcd.printKeySignature(&key_signature);
            }
#endif
            break;
          case MIDI_CH_BUTTON:
#ifdef USE_LED
            setLedStatusSource(&led_ctrl);
#endif
#ifdef USE_LCD
            wave_selecter.changeCurrentMidiChannel(0);
#endif
            break;
          case ARPEGGIO_BUTTON:
            if( ! isButtonOn(KEY_BUTTON) ) {
              arpeggiator.toggleEnabled();
            } else if( isButtonOn(ADD9_BUTTON) ) {
              metronome.toggleResolution();
            } else {
              arpeggiator.toggleHoldMode();
            }
            break;
          case DRUM_BUTTON:
            isButtonOn(KEY_BUTTON) ? metronome.tap() : drum.toggleEnabled();
            break;
          case CHORD_BUTTON:
            isButtonOn(KEY_BUTTON) ? note_senders.toggleHoldMode() : note_senders.togglePolyMode();
            break;
        }
        return;
      }
      if( isButtonOn(KEY_BUTTON) ) {
        if( isButtonOn(ADD9_BUTTON) ) {
          if( x == 0 ) {
            if( y == 0 ) metronome.sync(); else metronome.shiftBpm(y);
          }
          return;
        }
        if(x) key_signature.shift(x); else key_signature.shift( 7 * y, -5, 6 );
#ifdef USE_LED
        led_key.setKeySignature(&key_signature);
#endif
#ifdef USE_LCD
        lcd.printKeySignature(&key_signature, '>');
#endif
        return;
      }
      if( isButtonOn(MIDI_CH_BUTTON) ) {
        if(y == 0) {
          if( x >= -1 && x <= 1 ) wave_selecter.changeCurrentMidiChannel(x);
#ifdef USE_LCD
          else if( x == -2 ) lcd.printProgram(wave_selecter.getProgram());
#endif
          return;
        }
        switch(x) {
          case 0: wave_selecter.changeWaveform(y); break;
          case 1: wave_selecter.changeEnvelope(ADSR_ATTACK_VALUE, y); break;
          case 2: wave_selecter.changeEnvelope(ADSR_DECAY_VALUE, y); break;
          case 3: wave_selecter.changeEnvelope(ADSR_SUSTAIN_VALUE, y); break;
          case 4: wave_selecter.changeEnvelope(ADSR_RELEASE_VALUE, y); break;
          case -1: wave_selecter.programChange(wave_selecter.getProgram() + y*8); break;
          case -2: wave_selecter.programChange(wave_selecter.getProgram() + y); break;
        }
        return;
      }
      const boolean is_flat5 = isButtonOn(FLAT5_BUTTON);
      const boolean is_aug = (y > 0 && is_flat5);
      MusicalChord chord = MusicalChord(
        key_signature, x, is_aug?0:y, is_aug?1:is_flat5?-1:0,
        (isButtonOn(M7_BUTTON) ? -1 : 0) +
        (isButtonOn(SEVENTH_BUTTON) ? -2 : 0),
        isButtonOn(ADD9_BUTTON)
      );
      note_senders.pressed((ButtonID)button_id, wave_selecter.getCurrentMidiChannel(), &chord);
    }
    void button_released(const byte button_id) {
      if( button_id > CHORD_BUTTON || (button_id & BUTTON_ID_HALF_MASK) >= 3 ) {
        note_senders.released((ButtonID)button_id);
        return;
      }
      switch(button_id) {
        case KEY_BUTTON:
#ifdef USE_LED
          setLedStatusSource(&led_main);
#endif
#ifdef USE_LCD
          lcd.printKeySignature(&key_signature);
          if( isButtonOn(ADD9_BUTTON) ) lcd.printTempo(metronome.getBpm(),':');
#endif
          break;
        case ADD9_BUTTON:
#ifdef USE_LCD
          if( isButtonOn(KEY_BUTTON) ) {
            lcd.printTempo(metronome.getBpm(),':');
            lcd.printKeySignature( &key_signature, '>' );
          }
#endif
          break;
        case MIDI_CH_BUTTON:
#ifdef USE_LED
          setLedStatusSource(&led_main);
#endif
#ifdef USE_LCD
          wave_selecter.showWaveform(':');
          lcd.printKeySignature(&key_signature);
#endif
          break;
      }
    }
};

MatrixButtons matrix;

void setup() {
#ifdef OCTAVE_ANALOG_PIN
  randomSeed(analogRead(OCTAVE_ANALOG_PIN));
#endif
  PWMDACSynth::setup();
#ifdef USE_LED
  led_key.setKeySignature(&key_signature);
#endif
#ifdef USE_LCD
  lcd.setup();
  lcd.printKeySignature(&key_signature);
#endif
#if defined(USE_LED)
  matrix.setLedStatusSource(&led_main);
#endif
  matrix.setup();
#ifdef USE_MIDI
#ifdef USE_MIDI_IN
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandlePitchBend(PWMDACSynth::pitchBend);
  MIDI.setHandleControlChange(PWMDACSynth::controlChange);
  MIDI.setHandleClock(HandleClock);
  MIDI.setHandleSystemReset(HandleSystemReset);
  MIDI.setHandleSystemExclusive(HandleSystemExclusive);
#if defined(OCTAVE_ANALOG_PIN)
  MIDI.setHandleProgramChange(HandleProgramChange);
#endif
#endif
  MIDI.begin(MIDI_CHANNEL_OMNI); // receives all MIDI channels
  MIDI.turnThruOff(); // Disable MIDI IN -> MIDI OUT mirroring
#ifdef MIDI_ENABLE_PIN
  pinMode(MIDI_ENABLE_PIN,OUTPUT);
  digitalWrite(MIDI_ENABLE_PIN,HIGH); // enable MIDI port
#endif
#endif
}

void loop() {
#ifdef USE_MIDI_IN
  MIDI.read();
#endif
  matrix.scan();
  metronome.update();
  PWMDACSynth::update();
}
