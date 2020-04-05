
class MusicalNote {
  protected:
    char co5_value;  // Circle of Fifths value
    char note_value; // Chromatic note value
  public:
    static int getLowerBound() {
#ifdef OCTAVE_ANALOG_PIN
      return map(analogRead(OCTAVE_ANALOG_PIN), 0, 1023, 0, 127-11);
#else
      return 36;
#endif
    }
    static int shiftOctave(int note) { return shiftOctave(note, getLowerBound()); }
    static int shiftOctave(int note, int lower_bound) {
      return musicalConstrain12(note, lower_bound, lower_bound + 11);
    }
    static int shiftOctave(const int note, const int lower_bound, const char chromatic_offset) {
      return shiftOctave( note, (
        chromatic_offset == 0 ?
        lower_bound :
        musicalConstrain12(lower_bound + chromatic_offset, 0, 127-11)
      ) );
    }
    MusicalNote() { note_value = co5_value = 0; }
    MusicalNote(const char co5) { setCo5(co5); }
    void setCo5(const char co5) {
      co5_value = co5;
      note_value = musicalMod12( co5 + (co5 & 1) * 6 );
    }
    char getCo5() const { return  co5_value; }
    char getNote() const { return note_value; }
    char getOctaveShiftedNote() const { return shiftOctave(note_value); }
    char getOctaveShiftedNote(const char chromatic_offset) const {
      return shiftOctave( note_value, getLowerBound(), chromatic_offset );
    }
    char *print(char *bufp, const char offset = 0) {
      const char co5 = co5_value + offset + 1;
      *bufp++ = "FCGDAEB"[musicalMod7(co5)];
      if (co5 < 0) {
        *bufp++ = 'b'; // flat or double flat
        if (co5 < -7) *bufp++ = 'b'; // double flat
      }
      else if (co5 >= 14) *bufp++ = 'x'; // double sharp
      else if (co5 >=  7) *bufp++ = '#'; // sharp
      return bufp;
    }
    // y = musicalMod12(x) in Hex
    //   x : ... F3, F4, F5, ... FE, FF, 0, 1, 2, ... ,B, C, D, ...
    //   y : ...  B,  0,  1, ...  A,  B, 0, 1, 2, ... ,B, 0, 1, ...
    static byte musicalMod12(char x) {
      char qx = x >> 2;
      while( qx & 0xFC ) qx = (qx >> 2) + (qx & 3);
      x &= 3;
      return (qx==3||qx==0) ? x : (x + (qx << 2));
    }
    static byte musicalMod7(char x) {
      while( x & 0xF8 ) x = (x >> 3) + (x & 7);
      return x==7 ? 0 : x;
    }
    static int musicalConstrain12(const int note, const int min_note, const int max_note) {
      if( max_note < note ) {
        return max_note - musicalMod12(max_note - note);
      }
      else if( min_note > note ) {
        return min_note + musicalMod12(note - min_note);
      }
      return note;
    }
};

class KeySignature : public MusicalNote {
  public:
    void shift(char offset, const char v_min = -7, const char v_max = 7) {
      offset += co5_value;
      if      (offset > v_max) offset -= 12;
      else if (offset < v_min) offset += 12;
      MusicalNote::setCo5(offset);
    }
    char *print(char *bufp) const {
      if (co5_value == 0) return bufp;
      char b = co5_value < 0 ? 'b':'#';
      char n = abs(co5_value);
      if (n < 5) do { *bufp++ = b; } while(--n);
      else { *bufp++ = '0'+n; *bufp++ = b; }
      return bufp;
    }
    char *printSymbol(char *bufp) {
      bufp = MusicalNote::print(bufp);
      *bufp++ = '/';
      bufp = MusicalNote::print(bufp,3);
      *bufp++ = 'm';
      return bufp;
    }
};

class MusicalChord : public MusicalNote {
  protected:
    char offset3;   // offset of major 3rd
    char offset5;   // offset of perfect 5th
    char offset7;   // 0:none -1:M7 -2:7th -3:6th/dim7th
    boolean has9;   // add9 extended
  public:
    static const byte MAX_NOTES = 6;
    MusicalChord() : MusicalNote() {
      offset3 = offset5 = offset7 = 0; has9 = false;
    }
    MusicalChord(
      const KeySignature key_sig,
      const char offset1 = 0,
      const char offset3 = 0,
      const char offset5 = 0,
      const char offset7 = 0,
      const boolean has9 = false
    ) {
      char offset = offset1 + key_sig.getCo5();
      if( (this->offset3 = offset3) < 0 ) offset += 3;
      MusicalNote::setCo5(offset);
      this->offset5 = offset5;
      this->offset7 = offset7;
      this->has9 = has9;
    }
    boolean equals(const MusicalChord *chord) const {
      return ! memcmp( this, chord, sizeof(MusicalChord) );
    }
    boolean isSus4() const { return offset3 > 0; }
    char get3rdNote() const { return note_value +  4 + offset3; }
    char get5thNote() const { return note_value +  7 + offset5; }
    char get7thNote() const { return note_value + 12 + offset7; }
    char get9thNote() const { return note_value + (has9?14:12); }
    char getNote(const byte i, const int lower_bound) const {
      switch(i) {
      case 0: return shiftOctave( note_value,   lower_bound );
      case 1: return shiftOctave( get3rdNote(), lower_bound );
      case 2: return shiftOctave( get5thNote(), lower_bound );
      case 3: return offset7==0?
        shiftOctave( get5thNote(), lower_bound, -12 ) :
        shiftOctave( get7thNote(), lower_bound );
      case 4: return has9?
        shiftOctave( get9thNote(), lower_bound ):
        shiftOctave( get3rdNote(), lower_bound, -12 );
      case 5: return shiftOctave( note_value, lower_bound, -12 );
      }
      return 0;
    }
    char getNote(const byte i) const { return getNote( i, getLowerBound() ); }
    char getRandomNote() const { return getNote(random(MAX_NOTES)); }
    void toNotes(char * const notes, const size_t n_notes, const int lower_bound) const {
      for(byte i=0; i<n_notes; i++) notes[i] = getNote(i, lower_bound);
    }
    void toNotes(char * const notes, const size_t n_notes) const {
      toNotes(notes, n_notes, getLowerBound());
    }
    void toNotes(char * const notes) const { toNotes(notes, MAX_NOTES); }
    char *print(char *bufp) {
      bufp = MusicalNote::print(bufp);
      if( offset3 < 0 && offset5 < 0 && offset7 == -3 ) {
        memcpy(bufp,"dim",3); bufp += 3;
        *bufp++ = (has9 ? '9':'7');
      }
      else {
        if( offset3 < 0 ) *bufp++ = 'm';
        if( offset5 > 0 ) { memcpy(bufp,"aug",3); bufp += 3; }
        switch(offset7) {
          case  0: if(has9) { memcpy(bufp,"add9",4); bufp += 4; } break;
          case -1: *bufp++ = 'M'; /* FALLTHROUGH */
          case -2: *bufp++ = (has9 ? '9':'7'); break;
          case -3: *bufp++ = '6'; if(has9) *bufp++ = '9'; break;
        }
        if( offset3 > 0 ) { memcpy(bufp,"sus4",4); bufp += 4; }
        if( offset5 < 0 ) { memcpy(bufp,"(-5)",4); bufp += 4; }
      }
      return bufp;
    }
};
