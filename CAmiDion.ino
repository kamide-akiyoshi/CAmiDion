//
// CAmiDion - Musical Chord Instrument
//  ver.20151002
//  by Akiyoshi Kamide (Twitter: @akiyoshi_kamide)
//  http://kamide.b.osdn.me/camidion/
//  http://osdn.jp/users/kamide/pf/CAmiDion/
//  http://www.yk.rim.or.jp/~kamide/music/chordhelper/hardware/
//

#include "CAmiDionConfig.h"

#include <PWMDAC_Synth.h>
const EnvelopeParam DEFAULT_ENV_PARAM = {4, 10, 0, 8};
const EnvelopeParam DRUM_ENV_PARAM = {0, 5, 0, 5};
#if defined(OCTAVE_ANALOG_PIN)
PWMDAC_CREATE_INSTANCE(sawtoothWavetable, PWMDAC_SAWTOOTH_WAVE, DEFAULT_ENV_PARAM);
PWMDAC_CREATE_WAVETABLE(squareWavetable, PWMDAC_SQUARE_WAVE);
PWMDAC_CREATE_WAVETABLE(triangleWavetable, PWMDAC_TRIANGLE_WAVE);
PWMDAC_CREATE_WAVETABLE(sineWavetable, PWMDAC_SINE_WAVE);
PWMDAC_CREATE_WAVETABLE(shepardToneSineWavetable, PWMDAC_SHEPARD_TONE);
extern PROGMEM const byte guitarWavetable[];
#else
PWMDAC_CREATE_INSTANCE(shepardToneSineWavetable, PWMDAC_SHEPARD_TONE);
#endif
extern PROGMEM const byte randomWavetable[];
extern PROGMEM const byte shepardToneSawtoothWavetable[];
PROGMEM const byte * const wavetables[] = {
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

#if defined(USE_MIDI)
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

#include "musicalnote.h"
#include "decoder.h"
#include "button.h"

#if defined(USE_LED)
#include "LED.h"
NoteCountableLedStatus led_main;
NoteLedStatus led_key;
MidiChLedStatus led_ctrl;
LedViewport led_viewport = LedViewport(&led_main);
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

class WaveSelecter {
  public:
    WaveSelecter() { clearWaveIndices(); setDefaultDrum(); setDefaultChannel(); }
#ifdef USE_LCD
    void showWaveform(char delimiter = ':') {
      lcd.printWaveform(current_midi_channel, getChannel()->wavetable, delimiter);
    }
    void showEnvelope() { lcd.printEnvelope(&(getChannel()->env_param)); }
#endif
    byte getCurrentMidiChannel() { return current_midi_channel; }
    void changeCurrentMidiChannel(char offset) {
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
    void changeWaveform(char offset) {
      char *swi = selected_wave_indices + (current_midi_channel - 1);
      if( offset ) {
        if ( (*swi += offset) < 0) {
          *swi += N_WAVETABLES;
        }
        else if (*swi >= N_WAVETABLES) {
          *swi -= N_WAVETABLES;
        }
      }
      getChannel()->wavetable = (PROGMEM const byte *)pgm_read_word(wavetables + *swi);
#ifdef USE_LCD
      showWaveform('>');
#endif
    }
    void changeAttack(char offset) {
      changeEnvTime( &(getChannel()->env_param.attack_time), offset );
    }
    void changeDecay(char offset) {
      changeEnvTime( &(getChannel()->env_param.decay_time), offset );
    }
    void changeSustain(char offset) {
      getChannel()->env_param.sustain_level += offset * 0x10;
#ifdef USE_LCD
      showEnvelope();
#endif
    }
    void changeRelease(char offset) {
      changeEnvTime( &(getChannel()->env_param.release_time), offset );
    }
  protected:
    static const byte N_WAVETABLES = NumberOf(wavetables);
    char selected_wave_indices[16];
    byte current_midi_channel; // 1==CH1, ...
    MidiChannel *getChannel() {
      return PWMDACSynth::getChannel(current_midi_channel);
    }
    void clearWaveIndices() {
      memset(selected_wave_indices, 0, sizeof(selected_wave_indices));
    }
    void setDefaultDrum() {
      char *swi = selected_wave_indices + ((current_midi_channel = 10) - 1);
      *swi += N_WAVETABLES - 1;
      MidiChannel *mcp = getChannel();
      mcp->wavetable = (PROGMEM const byte *)pgm_read_word(wavetables + *swi);
      mcp->env_param = DRUM_ENV_PARAM;
    }
    void setDefaultChannel() {
      current_midi_channel = 1;
#ifdef USE_LED
      led_ctrl.setMidiChannel(0);
#endif
    }
    void changeEnvTime(byte *p, char offset) {
      *p += offset; *p &= 0xF;
#ifdef USE_LCD
      showEnvelope();
#endif
    }
};

// MIDI IN receive callbacks
void HandleNoteOff(byte channel, byte pitch, byte velocity) {
  PWMDACSynth::noteOff(channel,pitch,velocity);
#ifdef USE_LED
  led_main.noteOff(pitch);
#endif
}
void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  if( velocity == 0 ) {
    HandleNoteOff(channel,pitch,velocity);
    return;
  }
  PWMDACSynth::noteOn(channel,pitch,velocity);
#ifdef USE_LED
  led_main.noteOn(pitch);
#endif
}

class ButtonDrivenNoteSender {
  public:
    boolean isForButton(ButtonID button_id) {
      return this->button_id == button_id;
    }
  protected:
    ButtonID button_id;
    byte midi_channel;
    void setFree() { button_id = NULL_BUTTON; }
    boolean isFree() { return isForButton(NULL_BUTTON); }
    void noteOff(char note, char velocity) {
      HandleNoteOff(midi_channel, note, velocity);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOff(note, velocity, midi_channel);
#endif
    }
    void noteOn(char note, char velocity) {
      HandleNoteOn(midi_channel, note, velocity);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOn(note, velocity, midi_channel);
#endif
    }
};

class PolyNoteSender : public ButtonDrivenNoteSender {
#define EACH_NOTES(p, method)  \
  char *p = notes; \
  for( size_t n = n_notes; n>0; n-- ) \
  ButtonDrivenNoteSender::method(*p++, NOTE_VELOCITY)

  protected:
    size_t n_notes; char *notes;
    boolean mallocNotes(size_t n) {
      return isFree() && (notes = (char *)malloc(sizeof(char) * n)) != NULL;
    }
    void freeNotes() { free(notes); n_notes = 0; }
    size_t noteOn(ButtonID button_id, byte midi_channel) {
      this->button_id = button_id;
      this->midi_channel = midi_channel;
      EACH_NOTES(p, noteOn);
      return n_notes;
    }
  public:
    PolyNoteSender() { n_notes = 0; }
    boolean isFree() { return n_notes == 0; }
    size_t noteOn(ButtonID button_id, byte midi_channel, char note) {
      if ( ! mallocNotes(1) ) return 0;
      n_notes = 1; *notes = note;
      return noteOn(button_id, midi_channel);
    }
    size_t noteOn(ButtonID button_id, byte midi_channel, MusicalChord *chord) {
      if ( ! mallocNotes(chord->MAX_NOTES) ) return 0;
      n_notes = chord->MAX_NOTES; chord->toNotes(notes);
      return noteOn(button_id, midi_channel);
    }
    void noteOff() {
      if( isFree() ) return;
      EACH_NOTES(p, noteOff); freeNotes();
    }
#undef EACH_NOTES
};

WaveSelecter wave_selecter;

class Arpeggiator : public ButtonDrivenNoteSender {
  protected:
    static const byte MODE_NOTE_ON = (1<<0);
    static const byte MODE_ENABLE = (1<<1);
    static const byte MODE_HOLD = (1<<2);
    char mode;
    MusicalChord chord;
    char note;
  public:
    Arpeggiator() {
      setFree();
      mode = MODE_HOLD;
#ifdef USE_LED
      led_key.setOn(NoteLedStatus::LEFT);
#endif
    }
    boolean isNoteOn() { return mode & MODE_NOTE_ON; }
    void noteOff() {
      if( ! isNoteOn() ) return;
      ButtonDrivenNoteSender::noteOff(note, ARPEGGIO_VELOCITY);
      mode &= ~MODE_NOTE_ON;
    }
    void noteOn() {
      if( isFree() ) return;
      ButtonDrivenNoteSender::noteOn(note = chord.getRandomNote(), ARPEGGIO_VELOCITY);
      mode |= MODE_NOTE_ON;
    }
#ifdef USE_LED
    void beatOff() { if (isEnabled()) led_main.setOff(NoteLedStatus::LEFT); }
    void beatOn()  { if (isEnabled()) led_main.setOn(NoteLedStatus::LEFT); }
#endif
    void pressed(ButtonID button_id, char midi_channel, MusicalChord *chord) {
      if( ! isEnabled() ) return;
      this->button_id = button_id;
      this->midi_channel = midi_channel;
      this->chord = *chord;
    }
    void released(ButtonID button_id) {
      if( ! isHoldMode() && isForButton(button_id) ) setFree();
    }
    void forceRelease() { if( isHoldMode() ) setFree(); }
    boolean isEnabled() { return mode & MODE_ENABLE; }
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
        setFree();
#ifdef USE_LED
        led_main.setOff(NoteLedStatus::LEFT);
#endif
#ifdef USE_LCD
        wave_selecter.showWaveform();
#endif
      }
    }
    boolean isHoldMode() { return mode & MODE_HOLD; }
    void toggleHoldMode() {
      mode ^= MODE_HOLD;
      if ( isHoldMode() )
#ifdef USE_LED
        led_key.setOn(NoteLedStatus::LEFT);
#endif
      else {
        setFree();
#ifdef USE_LED
        led_key.setOff(NoteLedStatus::LEFT);
#endif
      }
    }
};

class Drum {
  protected:
    static const byte MIDI_CH = 10;
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
    void noteOff() {
      if( note < 0 ) return;
      PWMDACSynth::noteOff(MIDI_CH, note, DRUM_VELOCITY);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOff(DRUM_NOTE_NUMBER, DRUM_VELOCITY, MIDI_CH);
#endif
      note = -1;
#ifdef USE_LED
      ledOff();
#endif
    }
    void noteOn() {
      if( ! is_enabled ) return;
      PWMDACSynth::noteOn(MIDI_CH, note = 0, DRUM_VELOCITY);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOn(DRUM_NOTE_NUMBER, DRUM_VELOCITY, MIDI_CH);
#endif
#ifdef USE_LED
      ledOn();
#endif
    }
    void toggleEnabled() {
      is_enabled = ! is_enabled;
#ifdef USE_LED
      if( is_enabled ) ledOn(); else ledOff();
#endif
    }
};


Arpeggiator arpeggiator;
Drum drum;

#define US_PER_MINUTE  (60000000ul) // microseconds per minute
class Metronome {
  protected:
    static const unsigned int DEFAULT_BPM = 120;
    unsigned long us_timeout;
    unsigned long us_interval;
    unsigned long us_last_tapped;
    unsigned int bpm;
    byte resolution;
    byte count;
    void noteOff() {
      drum.noteOff();
#ifdef USE_LED
      led_main.setOff(NoteLedStatus::UPPER);
      led_key.setOff(NoteLedStatus::UPPER);
      arpeggiator.beatOff();
#endif
    }
    void noteOn() {
      drum.noteOn();
#ifdef USE_LED
      led_main.setOn(NoteLedStatus::UPPER);
      led_key.setOn(NoteLedStatus::UPPER);
      arpeggiator.beatOn();
#endif
    }
    void adjustInterval() {
      us_interval = (unsigned long)(US_PER_MINUTE / resolution / bpm);
    }
    void adjustBpm() {
      bpm = (unsigned int)(US_PER_MINUTE / resolution / us_interval);
    }
    void sync( unsigned long us = micros() ) { us_timeout = us; count = 0; }
  public:
    Metronome() {
      us_last_tapped = ULONG_MAX;
      resolution = 4;
      setBpm(DEFAULT_BPM);
      sync();
    }
    unsigned long getIntervalMicros() { return us_interval; }
    void setIntervalMicros(unsigned long us_interval) {
      this->us_interval = us_interval;
      adjustBpm();
    }
    byte getResolution() { return resolution; }
    void setResolution(byte resolution) {
      this->resolution = resolution;
      adjustInterval();
    }
    unsigned int getBpm() { return bpm; }
    void setBpm(unsigned int bpm) { this->bpm = bpm; adjustInterval(); }
    void update() { if( micros() >= us_timeout ) timeout(); }
    void timeout() {
      arpeggiator.noteOff();
      arpeggiator.noteOn();
      switch(count) {
        case 0: noteOn();  break;
        case 1: noteOff(); break;
      }
      if(count < resolution - 1) count++; else count = 0;
      us_timeout += us_interval;
    }
    void shiftTempo(char offset) {
      if(offset) setBpm(bpm + offset);
      sync();
#ifdef USE_LCD
      lcd.printTempo(bpm, '>');
#endif
    }
    void tap( unsigned long us = micros() ) {
      if( us_last_tapped < ULONG_MAX ) {
        unsigned long us_diff = us - us_last_tapped;
        if( us_diff < US_PER_MINUTE / 40 ) setIntervalMicros(us_diff / resolution);
      }
      sync(us_last_tapped = us);
#ifdef USE_LCD
      lcd.printTempo(bpm);
#endif
    }
};

class PolyNoteSenders {
#define EACH_SENDER(sender) PolyNoteSender *sender = senders; \
  for( byte i=NumberOf(senders); i>0; i--, sender++ )

  protected:
    static const byte MAX_BUTTONS = 8;
    PolyNoteSender senders[MAX_BUTTONS];
    static const byte MODE_POLY = (1<<0);
    static const byte MODE_HOLD = (1<<1);
    byte mode;
    void noteOff() { EACH_SENDER(s) s->noteOff(); }
    PolyNoteSender *getFreeNoteSender() {
      EACH_SENDER(s) if( s->isFree() ) return s;
      return NULL;
    }
  public:
    PolyNoteSenders() {
      mode = MODE_POLY;
#ifdef USE_LED
      led_main.setOn(NoteLedStatus::RIGHT);
#endif
    }
    void pressed(ButtonID button_id, byte midi_channel, MusicalChord *chord) {
      if( isHoldMode() ) noteOff();
      arpeggiator.forceRelease();
      PolyNoteSender *bp = getFreeNoteSender();
      if( isPolyMode() ) {
        if( bp != NULL ) bp->noteOn(button_id, midi_channel, chord);
      } else if( ! arpeggiator.isEnabled() ) {
        if( bp != NULL ) bp->noteOn(
          button_id, midi_channel,
          chord->getOctaveShiftedNote(chord->isSus4() ? 12 : 0)
        );
        return;
      }
      arpeggiator.pressed(button_id, midi_channel, chord);
#ifdef USE_LCD
      lcd.printChord(chord);
#endif
    }
    void released(ButtonID button_id) {
      if( ! isHoldMode() ) {
        EACH_SENDER(s) if( s->isForButton(button_id) ) {
          s->noteOff(); break;
        }
      }
      arpeggiator.released(button_id);
    }
    boolean isPolyMode() { return mode & MODE_POLY; }
    void togglePolyMode() {
      mode ^= MODE_POLY;
#ifdef USE_LCD
      lcd.setCursor(0,0);
#endif
      if ( isPolyMode() ) {
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
        wave_selecter.showWaveform();
#endif
      }
    }
    boolean isHoldMode() { return mode & MODE_HOLD; }
    void toggleHoldMode() {
      mode ^= MODE_HOLD;
      if ( isHoldMode() ) {
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
#undef EACH_SENDER
};


KeySignature key_signature;
Metronome metronome;
PolyNoteSenders note_senders;
ButtonInput button_input;

class MyButtonHandler : public ButtonHandler {
  protected:
    void shiftButtonPressed(ButtonID button_id) {
      switch(button_id) {
        case KEY_BUTTON:
#ifdef USE_LED
          led_key.setKeySignature(&key_signature);
          led_viewport.setSource(&led_key);
#endif
#ifdef USE_LCD
          lcd.printTempo(
            metronome.getBpm(),
            button_input.isOn(ADD9_BUTTON)?'>':':' );
          lcd.printKeySignature(
            &key_signature,
            button_input.isOn(ADD9_BUTTON)?':':'>' );
#endif
          break;
        case ADD9_BUTTON:
#ifdef USE_LCD
          if( ! button_input.isOn(KEY_BUTTON) ) break;
          lcd.printTempo( metronome.getBpm(), '>' );
          lcd.printKeySignature(&key_signature);
#endif
          break;
        case MIDI_CH_BUTTON:
#ifdef USE_LED
          led_viewport.setSource(&led_ctrl);
#endif
#ifdef USE_LCD
          wave_selecter.changeCurrentMidiChannel(0);
#endif
          break;
        case ARPEGGIO_BUTTON:
          if( ! button_input.isOn(KEY_BUTTON) ) {
            arpeggiator.toggleEnabled();
          } else if( button_input.isOn(ADD9_BUTTON) ) {
            metronome.setResolution(metronome.getResolution()==4?3:4);
          } else {
            arpeggiator.toggleHoldMode();
          }
          break;
        case DRUM_BUTTON:
          if( button_input.isOn(KEY_BUTTON) ) {
            metronome.tap(); break;
          }
          drum.toggleEnabled(); break;
        case CHORD_BUTTON:
          if( button_input.isOn(KEY_BUTTON) ) {
            note_senders.toggleHoldMode(); break;
          }
          note_senders.togglePolyMode(); break;
      }
    }
    void shiftButtonReleased(ButtonID button_id) {
      switch(button_id) {
        case KEY_BUTTON:
#ifdef USE_LED
          led_viewport.setSource(&led_main);
#endif
#ifdef USE_LCD
          lcd.printKeySignature(&key_signature);
          if( button_input.isOn(ADD9_BUTTON) ) {
            lcd.printTempo(metronome.getBpm());
          }
#endif
          break;
        case ADD9_BUTTON:
#ifdef USE_LCD
          if( ! button_input.isOn(KEY_BUTTON) ) break;
          lcd.printTempo(metronome.getBpm());
          lcd.printKeySignature( &key_signature, '>' );
#endif
          break;
        case MIDI_CH_BUTTON:
#ifdef USE_LED
          led_viewport.setSource(&led_main);
#endif
#ifdef USE_LCD
          wave_selecter.showWaveform();
          lcd.printKeySignature(&key_signature);
#endif
          break;
      }
    }
  public:
    void pressed(ButtonID button_id) {
      char x = (byte)button_id & 7;
      char y = ((byte)button_id >> 3) & 7;
      if( y >= 3 ) { x--; y -= 4; }
      else if( x >= 3 ) { x -= 9; y--; }
      else { shiftButtonPressed(button_id); return; }
      if( button_input.isOn(KEY_BUTTON) ) {
        if( button_input.isOn(ADD9_BUTTON) ) {
          if(x == 0) metronome.shiftTempo(y);
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
      if( button_input.isOn(MIDI_CH_BUTTON) ) {
        if(y == 0) {
          if( x >= -1 && x <= 1 ) {
            wave_selecter.changeCurrentMidiChannel(x);
          }
          return;
        }
        switch(x) {
          case 0: wave_selecter.changeWaveform(y); break;
          case 1: wave_selecter.changeAttack(y);   break;
          case 2: wave_selecter.changeDecay(y);    break;
          case 3: wave_selecter.changeSustain(y);  break;
          case 4: wave_selecter.changeRelease(y);  break;
        }
        return;
      }
      boolean is_flat5 = button_input.isOn(FLAT5_BUTTON);
      boolean is_aug = (y > 0 && is_flat5);
      MusicalChord chord = MusicalChord(
        key_signature, x, is_aug?0:y, is_aug?1:is_flat5?-1:0,
        (button_input.isOn(M7_BUTTON) ? -1 : 0) +
        (button_input.isOn(SEVENTH_BUTTON) ? -2 : 0),
        button_input.isOn(ADD9_BUTTON)
      );
      note_senders.pressed(button_id, wave_selecter.getCurrentMidiChannel(), &chord);
    }
    void released(ButtonID button_id) {
      if( (byte)button_id >= 24 || ((byte)button_id & 7) >= 3 ) {
        note_senders.released(button_id);
        return;
      }
      shiftButtonReleased(button_id);
    }
};

MyButtonHandler handler;
HC138Decoder decoder;

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
  decoder.setup();
  button_input.setup();
#ifdef USE_MIDI
#ifdef USE_MIDI_IN
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandlePitchBend(PWMDACSynth::pitchBend);
  MIDI.setHandleControlChange(PWMDACSynth::controlChange);
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
  for( decoder.reset(); decoder.getOutput(); decoder.next() ) {
#ifdef USE_LED
    led_viewport.lightOff();
#endif
    decoder.sendOut();
#ifdef USE_LED
    led_viewport.lightOn(&decoder);
#endif
    button_input.scan(&handler, &decoder);
    PWMDACSynth::update();
  }
#ifdef USE_MIDI_IN
  MIDI.read();
#endif
  metronome.update();
}

