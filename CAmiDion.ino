//
// CAmiDion - Musical Chord Instrument
//  ver.20150915
//  by Akiyoshi Kamide (Twitter: @akiyoshi_kamide)
//  http://kamide.b.osdn.me/camidion/
//  http://osdn.jp/users/kamide/pf/CAmiDion/
//  http://www.yk.rim.or.jp/~kamide/music/chordhelper/hardware/
//

#include "CAmiDionConfig.h"

#include <limits.h>

#include <PWMDAC_Synth.h>
PWMDAC_CREATE_DEFAULT_INSTANCE();
extern PROGMEM const byte shepardToneSawtoothWavetable[];
extern PROGMEM const byte guitarWavetable[];
extern PROGMEM const byte randomWavetable[];

#if defined(USE_MIDI)
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

#include "musicalnote.h"
#include "decoder.h"
#include "button.h"

#ifdef USE_LED
#include "LED.h"
NoteCountableLedStatus led_main;
LedStatus led_key;
LedStatus led_ctrl;
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


PROGMEM const byte * const wavetables[] = {
#ifdef OCTAVE_ANALOG_PIN
  PWM_SYNTH.sawtoothWavetable,
  PWM_SYNTH.squareWavetable,
  guitarWavetable,
  PWM_SYNTH.sineWavetable,
  PWM_SYNTH.triangleWavetable,
#endif
  PWM_SYNTH.shepardToneSineWavetable,
  shepardToneSawtoothWavetable,
  randomWavetable,
};
class WaveSelecter {
  protected:
    char selected_waveforms[16];
    byte current_midi_channel; // 1==CH1, ...
    EnvelopeParam *getEnvParam(byte ch) {
      return &(PWM_SYNTH.getChannel(ch)->env_param);
    }
    EnvelopeParam *getEnvParam() {
      return getEnvParam(current_midi_channel);
    }
    void changeEnvTime(byte *p, char offset) {
      if( offset > 0 ) { if( *p >= 15 ) *p = 0; else (*p)++; }
      else { if( *p <= 0 ) *p = 15; else (*p)--; }
#ifdef USE_LCD
      showEnvelope();
#endif
    }
  public:
    WaveSelecter() {
      current_midi_channel = 1;
#ifdef USE_LED
      led_ctrl.setMidiChannel(current_midi_channel - 1);
#endif
      memset(selected_waveforms, 0, sizeof(selected_waveforms));
    }
    void setup() {
      PWM_SYNTH.setWave( (byte *)pgm_read_word(wavetables) );
      changeWaveform(10, NumberOf(wavetables)-1); // set MIDI Ch.10 to random wave noise
      for( byte i=1; i<=16; i++ ) {
        EnvelopeParam *ep = getEnvParam(i);
        if( i == 10 ) {
          ep->attack_speed = 0xFFFF;
          ep->decay_time = 5;
          ep->sustain_level = 0;
          ep->release_time = 5;
        }
        else {
          ep->attack_speed = 0x1000;
          ep->decay_time = 10;
          ep->sustain_level = 0;
          ep->release_time = 8;
        }
      }
    }
    byte getCurrentMidiChannel() { return current_midi_channel; }
    void changeCurrentMidiChannel(char offset) {
      char ch0 = current_midi_channel + offset - 1;
      current_midi_channel = (ch0 &= 0xF) + 1;
#ifdef USE_LED
      led_ctrl.setMidiChannel(ch0);
#endif
#ifdef USE_LCD
      showWaveform(true);
      showEnvelope();
#endif
    }
#ifdef USE_LCD
    void showWaveform(boolean channel_is_changing = false) {
      lcd.printWaveform(
        current_midi_channel,
        (PROGMEM const byte *)pgm_read_word(
          wavetables + selected_waveforms[current_midi_channel - 1]),
        channel_is_changing);
    }
    void showEnvelope() { lcd.printEnvelope(getEnvParam()); }
#endif
    void changeWaveform(byte midi_channel, char offset) {
      char *selected_waveform = selected_waveforms + (midi_channel - 1);
      *selected_waveform += offset;
      if (*selected_waveform < 0)
        *selected_waveform += NumberOf(wavetables);
      else if (*selected_waveform >= NumberOf(wavetables))
        *selected_waveform -= NumberOf(wavetables);
      PWM_SYNTH.getChannel(midi_channel)->wavetable
        = (PROGMEM const byte *)pgm_read_word(wavetables + *selected_waveform);
      if(midi_channel == current_midi_channel) showWaveform();
    }
    void changeWaveform(char offset) {
      changeWaveform(current_midi_channel, offset);
    }
    void changeAttack(char offset) {
      unsigned int *asp = &(getEnvParam()->attack_speed);
      if( offset > 0 ) { if( *asp <= 0x0001 ) *asp = 0x8000; else *asp >>= 1; }
      else { if( *asp >= 0x8000 ) *asp = 0x0001; else *asp <<= 1; }
#ifdef USE_LCD
      showEnvelope();
#endif
    }
    void changeDecay(char offset) {
      changeEnvTime( &(getEnvParam()->decay_time), offset );
    }
    void changeSustain(char offset) {
      unsigned int *slp = &(getEnvParam()->sustain_level);
      if( offset > 0 ) *slp +=0x1000; else *slp -=0x1000;
#ifdef USE_LCD
      showEnvelope();
#endif
    }
    void changeRelease(char offset) {
      changeEnvTime( &(getEnvParam()->release_time), offset );
    }
};

// MIDI IN receive callbacks
void HandleNoteOff(byte channel, byte pitch, byte velocity) {
  PWM_SYNTH.noteOff(channel,pitch,velocity);
#ifdef USE_LED
  led_main.noteOff(pitch);
#endif
}
void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  if( velocity == 0 ) {
    HandleNoteOff(channel,pitch,velocity);
    return;
  }
  PWM_SYNTH.noteOn(channel,pitch,velocity);
#ifdef USE_LED
  led_main.noteOn(pitch);
#endif
}

// MIDI OUT 
class MidiSender {
  public:
    void noteOff(char channel, char note, char velocity) {
      HandleNoteOff(channel, note, velocity);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOff(note, velocity, channel);
#endif
    }
    void noteOn(char channel, char note, char velocity) {
      HandleNoteOn(channel, note, velocity);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOn(note, velocity, channel);
#endif
    }
};

class NoteButtonEntry : public MidiSender {
  protected:
    byte button_index;
    size_t n_notes;
    char *notes;
    byte midi_channel;
    void noteOnChannel(byte midi_channel) {
      this->midi_channel = midi_channel;
      char *p = notes;
      for( size_t n = n_notes; n>0; n-- ) {
        MidiSender::noteOn(midi_channel, *p++, NOTE_VELOCITY);
      }
    }
    char *mallocNotes(size_t n) { return (char *)malloc(sizeof(char) * n); }
  public:
    NoteButtonEntry() { n_notes = 0; }
    boolean isFree() { return n_notes == 0; }
    byte getButtonIndex() { return button_index; }
    size_t noteOn(byte button_index, byte midi_channel, char note) {
      if ( ! isFree() || (notes = mallocNotes(1)) == NULL ) return 0;
      this->button_index = button_index;
      n_notes = 1; *notes = note;
      noteOnChannel(midi_channel);
      return n_notes;
    }
    size_t noteOn(byte button_index, byte midi_channel, MusicalChord *chord) {
      if ( ! isFree() || (notes = mallocNotes(chord->MAX_NOTES)) == NULL ) return 0;
      this->button_index = button_index;
      n_notes = chord->MAX_NOTES; chord->toNotes(notes);
      noteOnChannel(midi_channel);
      return n_notes;
    }
    void noteOff() {
      if( isFree() ) return;
      char *p = notes;
      for( size_t n = n_notes; n>0; n-- ) {
        MidiSender::noteOff(midi_channel, *p++, NOTE_VELOCITY);
      }
      free(notes); n_notes = 0;
    }
};


WaveSelecter wave_selecter;

class Arpeggiator : public MidiSender {
  protected:
    static const byte NULL_BUTTON = 0xFF;
    byte button_index;
    static const byte MODE_NOTE_ON = (1<<0);
    static const byte MODE_ON = (1<<1);
    static const byte MODE_HOLD = (1<<2);
    char mode;
    MusicalChord chord;
    char midi_channel;
    char note;
  public:
    Arpeggiator() {
      button_index = NULL_BUTTON;
      mode = MODE_HOLD;
#ifdef USE_LED
      led_key.setOn(LedStatus::LEFT);
#endif
    }
    boolean isNoteOn() { return mode & MODE_NOTE_ON; }
    void noteOff() {
      if( ! isNoteOn() ) return;
      MidiSender::noteOff(midi_channel, note, ARPEGGIO_VELOCITY);
      mode &= ~MODE_NOTE_ON;
    }
    void noteOn() {
      if( isFree() ) return;
      MidiSender::noteOn(
        midi_channel, note = chord.getRandomNote(), ARPEGGIO_VELOCITY
      );
      mode |= MODE_NOTE_ON;
    }
#ifdef USE_LED
    void beatOff() { if (isOn()) led_main.setOff(LedStatus::LEFT); }
    void beatOn()  { if (isOn()) led_main.setOn(LedStatus::LEFT); }
#endif
    boolean isFree() { return button_index == NULL_BUTTON; }
    void pressed(byte button_index, char midi_channel, MusicalChord *chord) {
      if( ! isOn() ) return;
      this->button_index = button_index;
      this->midi_channel = midi_channel;
      this->chord = *chord;
    }
    void released(byte button_index) {
      if( this->button_index != button_index ) return;
      if( ! isHoldMode() ) this->button_index = NULL_BUTTON;
    }
    void forceRelease() { if( isHoldMode() ) button_index = NULL_BUTTON; }
    boolean isOn() { return mode & MODE_ON; }
    void toggleOnOff() {
      mode ^= MODE_ON;
#ifdef USE_LCD
      lcd.setCursor(0,0);
#endif
      if ( isOn() ) {
#ifdef USE_LED
        led_main.setOn(LedStatus::LEFT);
#endif
#ifdef USE_LCD
        lcd.printChord();
#endif
      } else {
        button_index = NULL_BUTTON;
#ifdef USE_LED
        led_main.setOff(LedStatus::LEFT);
#endif
#ifdef USE_LCD
        wave_selecter.showWaveform(false);
#endif
      }
    }
    boolean isHoldMode() { return mode & MODE_HOLD; }
    void toggleHoldMode() {
      mode ^= MODE_HOLD;
      if ( isHoldMode() )
#ifdef USE_LED
        led_key.setOn(LedStatus::LEFT);
#endif
      else {
        button_index = NULL_BUTTON;
#ifdef USE_LED
        led_key.setOff(LedStatus::LEFT);
#endif
      }
    }
};

class Drum {
  protected:
    static const char VELOCITY = DRUM_VELOCITY;
    static const byte MIDI_NOTE = DRUM_NOTE_NUMBER;
    static const byte MIDI_CH = 10;
    char note;
    boolean is_on;
#ifdef USE_LED
    void ledOn() {
      led_main.setOn(LedStatus::CENTER);
      led_key.setOn(LedStatus::CENTER);
    }
    void ledOff() {
      led_main.setOff(LedStatus::CENTER);
      led_key.setOff(LedStatus::CENTER);
    }
#endif
  public:
    Drum() { is_on = false; note = -1; }
    void noteOff() {
      if( note < 0 ) return;
      PWM_SYNTH.noteOff(MIDI_CH, note, VELOCITY);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOff(MIDI_NOTE, VELOCITY, MIDI_CH);
#endif
      note = -1;
#ifdef USE_LED
      ledOff();
#endif
    }
    void noteOn() {
      if( ! is_on ) return;
      PWM_SYNTH.noteOn(MIDI_CH, note = 0, VELOCITY);
#ifdef USE_MIDI_OUT
      MIDI.sendNoteOn(MIDI_NOTE, VELOCITY, MIDI_CH);
#endif
#ifdef USE_LED
      ledOn();
#endif
    }
    void toggleActive() {
      is_on = ! is_on;
#ifdef USE_LED
      if( is_on ) ledOn(); else ledOff();
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
      led_main.setOff(LedStatus::UPPER);
      led_key.setOff(LedStatus::UPPER);
      arpeggiator.beatOff();
#endif
    }
    void noteOn() {
      drum.noteOn();
#ifdef USE_LED
      led_main.setOn(LedStatus::UPPER);
      led_key.setOn(LedStatus::UPPER);
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
      lcd.printTempo(bpm, true);
#endif
    }
    void tap( unsigned long us = micros() ) {
      if( us_last_tapped < ULONG_MAX ) {
        unsigned long us_diff = us - us_last_tapped;
        if( us_diff < US_PER_MINUTE / 40 ) setIntervalMicros(us_diff / resolution);
      }
      sync(us_last_tapped = us);
#ifdef USE_LCD
      lcd.printTempo(bpm, false);
#endif
    }
};

class NoteButtons {
  protected:
    static const byte MAX_BUTTONS = 8;
    NoteButtonEntry buttons[MAX_BUTTONS];
    static const byte MODE_POLY = (1<<0);
    static const byte MODE_HOLD = (1<<1);
    byte mode;
    void noteOff() {
      NoteButtonEntry *bp = buttons;
      for( byte i=NumberOf(buttons); i>0; i--, bp++ ) bp->noteOff();
    }
    NoteButtonEntry *getFreeEntry() {
      NoteButtonEntry *bp = buttons;
      for( byte i=NumberOf(buttons); i>0; i--, bp++ ) if( bp->isFree() ) return bp;
      return NULL;
    }
  public:
    NoteButtons() {
      mode = MODE_POLY;
#ifdef USE_LED
      led_main.setOn(LedStatus::RIGHT);
#endif
    }
    void pressed(byte button_index, byte midi_channel, MusicalChord *chord) {
      if( isHoldMode() ) noteOff();
      arpeggiator.forceRelease();
      NoteButtonEntry *bp = getFreeEntry();
      if( isPolyMode() ) {
        if( bp != NULL ) bp->noteOn(button_index, midi_channel, chord);
      } else if( ! arpeggiator.isOn() ) {
        if( bp != NULL ) bp->noteOn(
          button_index, midi_channel,
          chord->getOctaveShiftedNote(chord->isSus4() ? 12 : 0)
        );
        return;
      }
      arpeggiator.pressed(button_index, midi_channel, chord);
#ifdef USE_LCD
      lcd.printChord(chord);
#endif
    }
    void released(byte button_index) {
      if( ! isHoldMode() ) {
        NoteButtonEntry *bp = buttons;
        for( byte i=NumberOf(buttons); i>0; i--, bp++ ) {
          if( button_index == bp->getButtonIndex() ) {
            bp->noteOff(); break;
          }
        }
      }
      arpeggiator.released(button_index);
    }
    boolean isPolyMode() { return mode & MODE_POLY; }
    void togglePolyMode() {
      mode ^= MODE_POLY;
#ifdef USE_LCD
      lcd.setCursor(0,0);
#endif
      if ( isPolyMode() ) {
#ifdef USE_LED
        led_main.setOn(LedStatus::RIGHT);
#endif
#ifdef USE_LCD
        lcd.printChord();
#endif
      } else {
#ifdef USE_LED
        led_main.setOff(LedStatus::RIGHT);
#endif
#ifdef USE_LCD
        wave_selecter.showWaveform(false);
#endif
      }
    }
    boolean isHoldMode() { return mode & MODE_HOLD; }
    void toggleHoldMode() {
      mode ^= MODE_HOLD;
      if ( isHoldMode() ) {
#ifdef USE_LED
        led_key.setOn(LedStatus::RIGHT);
#endif
      }
      else {
        noteOff();
#ifdef USE_LED
        led_key.setOff(LedStatus::RIGHT);
#endif
      }
    }
};


KeySignature key_signature;
ButtonInput button_input;
Metronome metronome;
NoteButtons note_buttons;

class MyButtonHandler : public ButtonHandler {
  protected:
    void shiftButtonPressed(byte button_index) {
      switch(button_index) {
        case ButtonInput::KEY:
#ifdef USE_LED
          led_key.setKeySignature(&key_signature);
          led_viewport.setSource(&led_key);
#endif
#ifdef USE_LCD
          lcd.printTempo( metronome.getBpm(), button_input.isOn(ButtonInput::ADD9) );
          lcd.printKeySignature( &key_signature, ! button_input.isOn(ButtonInput::ADD9) );
#endif
          break;
        case ButtonInput::ADD9:
#ifdef USE_LCD
          if( ! button_input.isOn(ButtonInput::KEY) ) break;
          lcd.printTempo( metronome.getBpm(), true );
          lcd.printKeySignature( &key_signature, false );
#endif
          break;
        case ButtonInput::MIDI_CH:
#ifdef USE_LED
          led_viewport.setSource(&led_ctrl);
#endif
#ifdef USE_LCD
          wave_selecter.changeCurrentMidiChannel(0);
#endif
          break;
        case ButtonInput::ARPEGGIO:
          if( ! button_input.isOn(ButtonInput::KEY) ) {
            arpeggiator.toggleOnOff();
          } else if( button_input.isOn(ButtonInput::ADD9) ) {
            metronome.setResolution(metronome.getResolution()==4?3:4);
          } else {
            arpeggiator.toggleHoldMode();
          }
          break;  
        case ButtonInput::DRUM:
          if( button_input.isOn(ButtonInput::KEY) ) {
            metronome.tap(); break;
          }
          drum.toggleActive(); break;
        case ButtonInput::CHORD:
          if( button_input.isOn(ButtonInput::KEY) ) {
            note_buttons.toggleHoldMode(); break;
          }
          note_buttons.togglePolyMode(); break;
      }
    }
    void shiftButtonReleased(byte button_index) {
      switch(button_index) {
        case ButtonInput::KEY:
#ifdef USE_LED
          led_viewport.setSource(&led_main);
#endif
#ifdef USE_LCD
          lcd.printKeySignature( &key_signature, false );
          if( button_input.isOn(ButtonInput::ADD9) ) {
            lcd.printTempo( metronome.getBpm(), false );
          }
#endif
          break;
        case ButtonInput::ADD9:
#ifdef USE_LCD
          if( ! button_input.isOn(ButtonInput::KEY) ) break;
          lcd.printTempo( metronome.getBpm(), false );
          lcd.printKeySignature( &key_signature, true );
#endif
          break;
        case ButtonInput::MIDI_CH:
#ifdef USE_LED
          led_viewport.setSource(&led_main);
#endif
#ifdef USE_LCD
          wave_selecter.showWaveform(false);
          lcd.printKeySignature( &key_signature, false );
#endif
          break;
      }
    }
  public:
    void pressed(byte button_index) {
      char x = button_index & 7;
      char y = (button_index >> 3) & 7;
      if( y >= 3 ) { x--; y -= 4; }
      else if( x >= 3 ) { x -= 9; y--; }
      else { shiftButtonPressed(button_index); return; }
      if( button_input.isOn(ButtonInput::KEY) ) {
        if( button_input.isOn(ButtonInput::ADD9) ) {
          if(x == 0) metronome.shiftTempo(y);
          return;
        }
        if(x) key_signature.shift(x); else key_signature.shift( 7 * y, -5, 6 );
#ifdef USE_LED
        led_key.setKeySignature(&key_signature);
#endif
#ifdef USE_LCD
        lcd.printKeySignature(&key_signature, true);
#endif
        return;
      }
      if( button_input.isOn(ButtonInput::MIDI_CH) ) {
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
      boolean is_flat5 = button_input.isOn(ButtonInput::FLAT5);
      boolean is_aug = (y > 0 && is_flat5);
      MusicalChord chord = MusicalChord(
        key_signature, x, is_aug?0:y, is_aug?1:is_flat5?-1:0,
        (button_input.isOn(ButtonInput::M7) ? -1 : 0) +
        (button_input.isOn(ButtonInput::SEVENTH) ? -2 : 0),
        button_input.isOn(ButtonInput::ADD9)
      );
      note_buttons.pressed(button_index, wave_selecter.getCurrentMidiChannel(), &chord);
    }
    void released(byte button_index) {
      if( button_index >= 24 || (button_index & 7) >= 3 ) {
        note_buttons.released(button_index);
        return;
      }
      shiftButtonReleased(button_index);
    }
};

MyButtonHandler handler = MyButtonHandler();
HC138Decoder decoder = HC138Decoder();

void setup() {
  PWM_SYNTH.setup();
#ifdef USE_LED
  led_key.setKeySignature(&key_signature);
#endif
#ifdef USE_LCD
  lcd.setup();
  lcd.printKeySignature(&key_signature, false);
#endif
  wave_selecter.setup();
  decoder.setup();
  button_input.setup();
#ifdef USE_MIDI
#ifdef USE_MIDI_IN
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandlePitchBend(PWM_SYNTH.pitchBend);
  MIDI.setHandleControlChange(PWM_SYNTH.controlChange);
#endif
  MIDI.begin(MIDI_CHANNEL_OMNI); // receives all MIDI channels
  MIDI.turnThruOff(); // Disable MIDI IN -> MIDI OUT mirroring
  pinMode(MIDI_ENABLE_PIN,OUTPUT);
  digitalWrite(MIDI_ENABLE_PIN,HIGH); // enable MIDI port
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
    PWM_SYNTH.update();
  }
#ifdef USE_MIDI_IN
  MIDI.read();
#endif
  metronome.update();
}

