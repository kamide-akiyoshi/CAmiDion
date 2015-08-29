//
// CAmiDion - Musical Chord Instrument
//  ver.20150829
//  by Akiyoshi Kamide (Twitter: @akiyoshi_kamide)
//  http://kamide.b.osdn.me/camidion/
//  http://osdn.jp/users/kamide/pf/CAmiDion/
//  http://www.yk.rim.or.jp/~kamide/music/chordhelper/hardware/
//

#include "CAmiDionConfig.h"

#if defined(USE_MIDI)
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

#include <PWMDAC_Synth.h> // PWM DAC Synthesizer lib
extern PROGMEM const byte shepardToneSawtoothWavetable[];
extern PROGMEM const byte guitarWavetable[];
extern PROGMEM const byte randomWavetable[];

#include "musicalnote.h"
KeySignature key_signature = KeySignature();

#include "decoder.h"
#include "button.h"
#ifdef USE_LED
#include "LED.h"
NoteCountableLedStatus led_main
  = NoteCountableLedStatus(0x0004); // One-push chord mode
LedStatus led_key  = LedStatus(0x0011); // C major key, hold arpeggio
LedStatus led_ctrl = LedStatus(0x0001); // Ch.1 (0..15 = MIDI Channel 1..16)
LedViewer led_viewer = LedViewer(&led_main);
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
CAmiDionLCD lcd = CAmiDionLCD();
#endif

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
class WaveformSelecter {
  protected:
    char selected_waveforms[16];
    byte current_midi_channel;    // 1==CH1, ...
  public:
    WaveformSelecter() {
      current_midi_channel = 1;
      memset(selected_waveforms, 0, sizeof(selected_waveforms));
    }
    void clear() {
      PWM_SYNTH.setWave( (byte *)pgm_read_word(wavetables) );
      change(10, NumberOf(wavetables)-1); // set MIDI Ch.10 to random wave noise
      for( int i=1; i<=16; i++ ) {
        EnvelopeParam *ep = &(PWM_SYNTH.getChannel(i)->env_param);
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
#ifdef USE_LCD
    void show(boolean channel_is_changing = false) {
      lcd.printWaveform(
        current_midi_channel,
        (PROGMEM const byte *)pgm_read_word(
          wavetables + selected_waveforms[current_midi_channel - 1]),
        channel_is_changing);
    }
#endif
    void change(byte midi_channel, char offset) {
      char *selected_waveform = selected_waveforms + (midi_channel - 1);
      *selected_waveform += offset;
      if (*selected_waveform < 0)
        *selected_waveform += NumberOf(wavetables);
      else if (*selected_waveform >= NumberOf(wavetables))
        *selected_waveform -= NumberOf(wavetables);
      PWM_SYNTH.getChannel(midi_channel)->wavetable
        = (PROGMEM const byte *)pgm_read_word(wavetables + *selected_waveform);
      if(midi_channel == current_midi_channel) show();
    }
    void change(char offset) { change(current_midi_channel,offset); }
    byte getCurrentMidiChannel() { return current_midi_channel; }
    void changeCurrentMidiChannel(char offset) {
      char ch0 = current_midi_channel + offset - 1;
      current_midi_channel = (ch0 &= 0xF) + 1;
#ifdef USE_LED
      led_ctrl.showMidiChannel(ch0);
#endif
#ifdef USE_LCD
      show(true);
      lcd.printEnvelope(current_midi_channel);
#endif
    }
};

WaveformSelecter waveform_selecter = WaveformSelecter();

class NoteButton : public Button, MidiSender {
  protected:
    static const byte VELOCITY = 100;
    size_t n_notes;
    char *notes;
    byte midi_channel;
    void noteOnChannel(byte midi_channel) {
      this->midi_channel = midi_channel;
      char *p = notes;
      for( size_t n = n_notes; n>0; n-- ) noteOn(midi_channel, *p++, VELOCITY);
    }
    char *mallocNotes(size_t n) { return (char *)malloc(sizeof(char) * n); }
  public:
    NoteButton() { n_notes = 0; }
    boolean isActivated() { return n_notes > 0 ; }
    size_t activate(Button *bp, byte midi_channel, char note) {
      if (isActivated()) return 0;
      if ((notes = mallocNotes(1)) == NULL) return 0;
      set(bp); n_notes = 1; *notes = note;
      noteOnChannel(midi_channel);
      return n_notes;
    }
    size_t activate(Button *bp, byte midi_channel, MusicalChord *chord) {
      if (isActivated()) return 0;
      if ((notes = mallocNotes(chord->MAX_NOTES)) == NULL) return 0;
      set(bp); n_notes = chord->MAX_NOTES; chord->toNotes(notes);
      noteOnChannel(midi_channel);
      return n_notes;
    }
    void deactivate() {
      if( ! isActivated() ) return;
      char *p = notes;
      for( size_t n = n_notes; n>0; n-- ) noteOff(midi_channel, *p++, VELOCITY);
      free(notes); n_notes = 0;
    }
};

class NoteButtons {
  protected:
    static const byte MAX_BUTTONS = 8;
    NoteButton buttons[MAX_BUTTONS];
    static const byte MODE_POLY = (1<<0);
    static const byte MODE_HOLD = (1<<1);
    byte mode;
  public:
    NoteButtons() {
      for(char i=0; i<NumberOf(buttons); i++) buttons[i] = NoteButton();
      mode = MODE_POLY;
    }
    NoteButton *getFreeEntry() {
      NoteButton *bp = buttons;
      for( char i=NumberOf(buttons); i>0; i--, bp++ ) if( !(bp->isActivated()) ) return bp;
      return NULL;
    }
    void deactivateAll() {
      NoteButton *bp = buttons;
      for( char i=NumberOf(buttons); i>0; i--, bp++ ) bp->deactivate();
    }
    void deactivate(Button *button_p) {
      NoteButton *bp = buttons;
      for( char i=NumberOf(buttons); i>0; i--, bp++ ) if( bp->equals(button_p) ) {
        bp->deactivate();
        return;
      }
    }
    boolean isPolyMode() { return mode & MODE_POLY; }
    void togglePolyMode() { mode ^= MODE_POLY; redisplayPolyMode(); }
    void redisplayPolyMode() {
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
        waveform_selecter.show(false);
#endif
      }
    }
    boolean isHoldMode() { return mode & MODE_HOLD; }
    void released(Button *bp) { if( ! isHoldMode() ) deactivate(bp); }
    void forceRelease() { if( isHoldMode() ) deactivateAll(); }
    void toggleHoldMode() { mode ^= MODE_HOLD; redisplayHoldMode(); }
    void redisplayHoldMode() {
      if ( isHoldMode() ) {
#ifdef USE_LED
        led_key.setOn(LedStatus::RIGHT);
#endif
      }
      else {
        deactivateAll();
#ifdef USE_LED
        led_key.setOff(LedStatus::RIGHT);
#endif
      }
    }
};
NoteButtons note_buttons = NoteButtons();

class ArpeggioChordButton : public Button, MidiSender {
  protected:
    static const char VELOCITY = 100;
    MusicalChord chord;
    char note;
    char midi_channel;
    static const byte MODE_ON = (1<<0);
    static const byte MODE_HOLD = (1<<1);
    char mode;
  public:
    ArpeggioChordButton() { setNull(); note = -1; mode = MODE_HOLD; }
    void set(Button *bp, char midi_channel, MusicalChord *chord) {
      Button::set(bp);
      this->midi_channel = midi_channel;
      this->chord = *chord;
    }
    void noteOff() {
      if( note < 0 ) return;
      MidiSender::noteOff(midi_channel, note, VELOCITY);
      note = -1;
    }
    void noteOn() {
      if( isNull() ) return;
      MidiSender::noteOn(midi_channel, note = chord.getRandomNote(), VELOCITY);
    }
#ifdef USE_LED
    void beatOff() { if (isOn()) led_main.setOff(LedStatus::LEFT); }
    void beatOn()  { if (isOn()) led_main.setOn(LedStatus::LEFT); }
#endif
    boolean isOn() { return mode & MODE_ON; }
    void toggleOnOff() { mode ^= MODE_ON; redisplayOnOff(); }
    void redisplayOnOff() {
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
        setNull();
#ifdef USE_LED
        led_main.setOff(LedStatus::LEFT);
#endif
#ifdef USE_LCD
        waveform_selecter.show(false);
#endif
      }
    }
    boolean isHoldMode() { return mode & MODE_HOLD; }
    void released(Button *bp) { if( ! isHoldMode() ) setNull(bp); }
    void forceRelease() { if( isHoldMode() ) setNull(); }
    void toggleHoldMode() { mode ^= MODE_HOLD; redisplayHoldMode(); }
    void redisplayHoldMode() {
      if ( isHoldMode() )
#ifdef USE_LED
        led_key.setOn(LedStatus::LEFT);
#endif
      else {
        setNull();
#ifdef USE_LED
        led_key.setOff(LedStatus::LEFT);
#endif
      }
    }
};
ArpeggioChordButton arpeggio_chord_button = ArpeggioChordButton();

class Drum {
  protected:
    static const char VELOCITY = 100;
    static const byte MIDI_CH = 10;
    char note;
    boolean is_on;
  public:
    Drum() { is_on = false; note = -1; }
    void noteOff() {
      if( note < 0 ) return;
      PWM_SYNTH.noteOff(MIDI_CH, note, VELOCITY);
      note = -1;
#ifdef USE_LED
      led_main.setOff(LedStatus::CENTER);
      led_key.setOff(LedStatus::CENTER);
#endif
    }
    void noteOn() {
      if( ! isOn() ) return;
      PWM_SYNTH.noteOn(MIDI_CH, note = 0, VELOCITY);
#ifdef USE_LED
      led_main.setOn(LedStatus::CENTER);
      led_key.setOn(LedStatus::CENTER);
#endif
    }
    boolean isOn() { return is_on; }
    void toggleActive() { is_on = ! is_on; redisplayOnOff(); }
    void redisplayOnOff() {
#ifdef USE_LED
      if( isOn() ) {
        led_main.setOn(LedStatus::CENTER);
        led_key.setOn(LedStatus::CENTER);
      } else {
        led_main.setOff(LedStatus::CENTER);
        led_key.setOff(LedStatus::CENTER);
      }
#endif
    }
};
Drum drum = Drum();

#define US_PER_MINUTE  (60000000ul) // microseconds per minute
class Metronome {
  protected:
    // metronome
    unsigned long us_timeout;
    unsigned long us_interval;
    unsigned long us_tapped;
    unsigned int bpm;
    byte resolution;
    byte count;
    void noteOff() {
      drum.noteOff();
#ifdef USE_LED
      led_main.setOff(LedStatus::UPPER);
      led_key.setOff(LedStatus::UPPER);
      arpeggio_chord_button.beatOff();
#endif
    }
    void noteOn() {
      drum.noteOn();
#ifdef USE_LED
      led_main.setOn(LedStatus::UPPER);
      led_key.setOn(LedStatus::UPPER);
      arpeggio_chord_button.beatOn();
#endif
    }
    void adjustInterval() {
      us_interval = (unsigned long)(US_PER_MINUTE / resolution / bpm);
    }
    void adjustBpm() {
      bpm = (unsigned int)(US_PER_MINUTE / resolution / us_interval);
    }
  public:
    Metronome() {
      us_tapped = 0xFFFFFFFF;
      resolution = 4;
      setBpm(120);
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
    void sync( unsigned long us = micros() ) { us_timeout = us; count = 0; }
    void tap( unsigned long us = micros() ) {
      if( us_tapped < 0xFFFFFFFF ) {
        unsigned long us_diff = us - us_tapped;
        if( us_diff < 1500000 ) setIntervalMicros(us_diff / resolution);
      }
      sync(us_tapped = us);
    }
    void timeout() {
      arpeggio_chord_button.noteOff();
      arpeggio_chord_button.noteOn();
      switch(count) {
        case 0: noteOn();  break;
        case 1: noteOff(); break;
      }
      if(count < resolution - 1) count++; else count = 0;
      us_timeout += us_interval;
    }
};
Metronome metronome = Metronome();

class ButtonEventHandler {
  protected:
    char offset5; // -1:dim(-5) 0:P5 +1:aug(+5)
    char offset7; // 0:octave -1:M7 -2:7th -3:6th(dim7)
    static const byte FLAG_KEY = (1<<0); // Key/Tempo
    static const byte FLAG_MCH = (1<<1); // MIDI Ch./Wave/ADSR
    static const byte FLAG_ADD9 = (1<<2);
    byte flag;
  public:
    ButtonEventHandler() { offset5 = offset7 = flag = 0; }
    void controlButtonPressed(Button *button_p) {
      switch(button_p->cathode8) {
        case -1:
          switch(button_p->anode6) {
            case -1: flag |= FLAG_KEY;
#ifdef USE_LED
              led_viewer.setSource(&led_key);
              led_key.showNote(key_signature.getNote());
#endif
#ifdef USE_LCD
              lcd.printTempo( metronome.getBpm(), flag & FLAG_ADD9 );
              lcd.printKeySignature( key_signature, !(flag & FLAG_ADD9) );
#endif
              break;
            case 0: flag |= FLAG_MCH;
#ifdef USE_LED
              led_viewer.setSource(&led_ctrl);
#endif
#ifdef USE_LCD
              waveform_selecter.changeCurrentMidiChannel(0);
#endif
              break;
            case 1: // Arpeggiator
              if( ! (flag & FLAG_KEY) ) {
                arpeggio_chord_button.toggleOnOff();
              } else if( flag & FLAG_ADD9 ) {
                metronome.setResolution(metronome.getResolution()==4?3:4);
              }
              else arpeggio_chord_button.toggleHoldMode();
              break;
          }
          break;
        case 0:
          switch(button_p->anode6) {
            case -1: flag |= FLAG_ADD9;
#ifdef USE_LCD
              if( flag & FLAG_KEY ) {
                lcd.printTempo( metronome.getBpm(), true );
                lcd.printKeySignature( key_signature, false );
              }
#endif
              break;
            case  0: offset7 -= 1; break; // M7
            case  1: // Rhythm
              if( flag & FLAG_KEY ) metronome.tap(); else drum.toggleActive();
#ifdef USE_LCD
              lcd.printTempo( metronome.getBpm(), false );
#endif
              break;
          }
          break;
        case 1:
          switch(button_p->anode6) {
            case -1: offset5 = -1; break; // -5/+5
            case  0: offset7 -= 2; break; // 7th
            case  1: // Poly/Mono
              if( flag & FLAG_KEY ) note_buttons.toggleHoldMode();
              else note_buttons.togglePolyMode();
              break;
          }
          break;
      }
    }
    void controlButtonReleased(Button *button_p) {
      switch(button_p->cathode8) {
        case -1:
          switch(button_p->anode6) {
            case -1: flag &= ~FLAG_KEY;
#ifdef USE_LED
              led_viewer.setSource(&led_main);
#endif
#ifdef USE_LCD
              lcd.printKeySignature( key_signature, false );
              if( flag & FLAG_ADD9 ) lcd.printTempo( metronome.getBpm(), false );
#endif
              break;
            case 0: flag &= ~FLAG_MCH;
#ifdef USE_LED
              led_viewer.setSource(&led_main);
#endif
#ifdef USE_LCD
              waveform_selecter.show(false);
              lcd.printKeySignature( key_signature, false );
#endif
              break;
          }
          break;
        case 0:
          switch(button_p->anode6) {
            case -1:  flag &= ~FLAG_ADD9;
#ifdef USE_LCD
              if( flag & FLAG_KEY ) {
                lcd.printTempo( metronome.getBpm(), false );
                lcd.printKeySignature( key_signature, true );
              }
#endif
              break;
            case 0: offset7 += 1; break; // M7
          }
          break;
        case 1:
          switch(button_p->anode6) {
            case -1: offset5  = 0; break; // -5/+5
            case 0:  offset7 += 2; break; // 7th
          }
          break;
      }
    }
    void pressed(Button *button_p) {
      ChordButtonPosition pos = ChordButtonPosition();
      if( button_p->anode6 >= 2 ) {
        pos.x = button_p->cathode8;
        pos.y = button_p->anode6 - 3;
      }
      else if( button_p->cathode8 >= 2 ) {
        pos.x = button_p->cathode8 - 8;
        pos.y = button_p->anode6;
      }
      else {
        controlButtonPressed(button_p);
        return;
      }
      if( flag & FLAG_KEY ) {
        if( flag & FLAG_ADD9 ) {
          if(pos.x) return;
          if(pos.y) metronome.setBpm(metronome.getBpm() + pos.y);
          metronome.sync();
#ifdef USE_LCD
          lcd.printTempo(metronome.getBpm(),true);
#endif
          return;
        }
        if(pos.x) key_signature.shift(pos.x);
        else key_signature.shift( 7 * pos.y, -5, 6 );
#ifdef USE_LED
        led_key.show(key_signature);
#endif
#ifdef USE_LCD
        lcd.printKeySignature( key_signature, true );
#endif
        return;
      }
      if( flag & FLAG_MCH ) {
        if( pos.y == 0 ) {
          if( pos.x >= -1 && pos.x <= 1 ) waveform_selecter.changeCurrentMidiChannel(pos.x);
        }
        else switch(pos.x) {
          case 0: waveform_selecter.change(pos.y); break;
          case 1: // Attack
            {
              EnvelopeParam *ep = &(PWM_SYNTH.getChannel(waveform_selecter.getCurrentMidiChannel())->env_param);
              unsigned int *asp = &(ep->attack_speed);
              if( pos.y > 0 ) { if( *asp <= 0x0001 ) *asp = 0x8000; else *asp >>= 1; }
              else { if( *asp >= 0x8000 ) *asp = 0x0001; else *asp <<= 1; }
#ifdef USE_LCD
              lcd.printEnvelope(waveform_selecter.getCurrentMidiChannel());
#endif
            }
            break;
          case 2: // Decay
            {
              EnvelopeParam *ep = &(PWM_SYNTH.getChannel(waveform_selecter.getCurrentMidiChannel())->env_param);
              byte *dtp = &(ep->decay_time);
              if( pos.y > 0 ) { if( *dtp >= 15 ) *dtp == 0; else (*dtp)++; }
              else { if( *dtp <= 0 ) *dtp = 15; else (*dtp)--; }
#ifdef USE_LCD
              lcd.printEnvelope(waveform_selecter.getCurrentMidiChannel());
#endif
            }
            break;
          case 3: // Sustain
            {
              EnvelopeParam *ep = &(PWM_SYNTH.getChannel(waveform_selecter.getCurrentMidiChannel())->env_param);
              unsigned int *slp = &(ep->sustain_level);
              if( pos.y > 0 ) *slp +=0x1000; else *slp -=0x1000;
#ifdef USE_LCD
              lcd.printEnvelope(waveform_selecter.getCurrentMidiChannel());
#endif
            }
            break;
          case 4: // Release
           {
              EnvelopeParam *ep = &(PWM_SYNTH.getChannel(waveform_selecter.getCurrentMidiChannel())->env_param);
              byte *rtp = &(ep->release_time);
              if( pos.y > 0 ) { if( *rtp >= 15 ) *rtp = 0; else (*rtp)++; }
              else { if( *rtp <= 0 ) *rtp = 15; else (*rtp)--; }
#ifdef USE_LCD
              lcd.printEnvelope(waveform_selecter.getCurrentMidiChannel());
#endif
            }
            break;
        }
        return;
      }
      note_buttons.forceRelease();
      arpeggio_chord_button.forceRelease();
      NoteButton *bp = note_buttons.getFreeEntry();
      if( bp == NULL ) return;
      boolean is_aug = (pos.y == 1 && offset5 == -1); // sus4(-5) -> +5
      MusicalChord chord = MusicalChord(
        key_signature, pos.x, is_aug?0:pos.y, is_aug?1:offset5, offset7, flag & FLAG_ADD9
      );
      if( ! note_buttons.isPolyMode() && ! arpeggio_chord_button.isOn() ) {
        bp->activate(button_p, waveform_selecter.getCurrentMidiChannel(),
          chord.getOctaveShiftedNote(pos.y == 1 ? 12 : 0)
        );
        return;
      }
      if( note_buttons.isPolyMode() ) {
        bp->activate(button_p, waveform_selecter.getCurrentMidiChannel(), &chord);
      }
      if( arpeggio_chord_button.isOn() ) {
        arpeggio_chord_button.set(button_p, waveform_selecter.getCurrentMidiChannel(), &chord);
      }
#ifdef USE_LCD
      lcd.printChord(&chord);
#endif
    }
    void released(Button *button_p) {
      if( button_p->isChordButton() ) {
        note_buttons.released(button_p);
        arpeggio_chord_button.released(button_p);
      }
      else controlButtonReleased(button_p);
    }
};
ButtonEventHandler handler = ButtonEventHandler();

class ScannableButton : public Button {
  protected:
    static const byte PORTB_BUTTON_MASK = 0b00111111; // Arduino port D8..13
    static const byte RELEASE_WAIT_TIMES = 8; // Anti-chattering release wait count
    byte last_inputs[8];
    byte *last_input_p;
    byte release_wait_counts[NUMBER_OF_BUTTONS];
  public:
    void setup() {
      DDRB  &= ~PORTB_BUTTON_MASK; // Set 0 (INPUT)
      PORTB |= PORTB_BUTTON_MASK;  // Set 1 to enable internal pullup resistor
      memset(last_inputs, 0b11111111, sizeof(last_inputs));
      memset(release_wait_counts, 0, sizeof(release_wait_counts));
    }
    void reset() {
      cathode8 = LOWER_BOUND;
      last_input_p = last_inputs;
    }
    void scan() {
      byte input_bits = PINB | ~PORTB_BUTTON_MASK;
      byte change = *last_input_p ^ input_bits;
      if(change) {
        anode6 = LOWER_BOUND;
        for( byte mask = 1; mask < PORTB_BUTTON_MASK; mask <<= 1, anode6++ ) {
          if( ! (change & mask) ) continue; // Bit not changed
          byte *wcp = release_wait_counts + getSerialNumber();
          if( ! (input_bits & mask) ) {
            handler.pressed(this);
            *wcp = RELEASE_WAIT_TIMES;
            continue;
          }
          // Really released ? check the wait counter to avoid chattering
          if(*wcp) { // Waiting
            input_bits ^= mask; // Cancel the change
            (*wcp)--;           // Countdown the wait-time
            continue;
          }
          handler.released(this);
        }
        *last_input_p = input_bits;
      }
      cathode8++; last_input_p++;
    }
};

class Matrix {
  protected:
    ScannableButton button;
    HC138Decoder decoder;
  public:
    Matrix() { button = ScannableButton(); decoder = HC138Decoder(); }
    void setup() { button.setup(); decoder.setup(); }
    void scan() {
      for(button.reset(),decoder.reset(); decoder.getOutput(); decoder.next()) {
#ifdef USE_LED
        led_viewer.ledOff();
#endif
        decoder.send();
#ifdef USE_LED
        led_viewer.ledOn(&decoder);
#endif
        button.scan();
        PWM_SYNTH.updateEnvelopeStatus();
      }
    }
};
Matrix matrix = Matrix();

void setup() {
#ifdef USE_LCD
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.printKeySignature(key_signature, false);
#endif
  PWM_SYNTH.setup();
  waveform_selecter.clear();
  matrix.setup();
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
  matrix.scan();
#ifdef USE_MIDI_IN
  MIDI.read();
#endif
  metronome.update();
}

