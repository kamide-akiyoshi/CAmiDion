
#if defined(OCTAVE_ANALOG_PIN)
// General MIDI instruments
PROGMEM const Instrument instruments[] = {
  // Piano
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {guitarWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 10, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {8, 0, 10, 4}},

  // Chromatic Percussion
  {sawtoothWavetable, {9, 0, 10, 4}},
  {sineWavetable, {9, 0, 11, 4}},
  {sineWavetable, {9, 0, 10, 3}},
  {sineWavetable, {9, 0, 11, 4}},
  {sineWavetable, {8, 0, 8, 4}},
  {sineWavetable, {7, 0, 7, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 9, 4}},

  // Organ
  {guitarWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 255, 11, 4}},
  {guitarWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 8}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},

  // Guitar
  {guitarWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {guitarWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {guitarWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 10, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {randomWavetable, {9, 8, 10, 4}},

  // Bass
  {guitarWavetable, {9, 0, 11, 4}},
  {guitarWavetable, {9, 0, 11, 4}},
  {guitarWavetable, {9, 192, 9, 3}},
  {guitarWavetable, {9, 192, 9, 6}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 128, 10, 4}},
  {sawtoothWavetable, {9, 192, 11, 4}},

  // Strings
  {sawtoothWavetable, {9, 255, 10, 6}},
  {sawtoothWavetable, {9, 255, 10, 6}},
  {sawtoothWavetable, {9, 255, 10, 6}},
  {sawtoothWavetable, {9, 255, 10, 6}},
  {guitarWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {8, 0, 8, 4}},
  {sineWavetable, {9, 0, 11, 4}},
  {randomWavetable, {9, 0, 10, 3}},

  // Ensemble
  {triangleWavetable, {9, 255, 11, 6}},
  {triangleWavetable, {9, 255, 11, 9}},
  {triangleWavetable, {9, 255, 11, 6}},
  {triangleWavetable, {9, 255, 11, 7}},
  {guitarWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 128, 9, 4}},
  {sineWavetable, {9, 255, 11, 4}},
  {squareWavetable, {8, 0, 9, 3}},

  // Brass
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {guitarWavetable, {9, 255, 11, 7}},
  {sawtoothWavetable, {9, 128, 9, 5}},
  {sawtoothWavetable, {9, 192, 9, 5}},
  {guitarWavetable, {9, 192, 10, 5}},

  // Reed
  {guitarWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 255, 11, 4}},
  {triangleWavetable, {9, 255, 11, 5}},

  // Pipe
  {guitarWavetable, {9, 255, 11, 6}},
  {guitarWavetable, {9, 255, 11, 7}},
  {sineWavetable, {9, 255, 11, 6}},
  {triangleWavetable, {9, 255, 11, 7}},
  {randomWavetable, {9, 128, 10, 6}},
  {triangleWavetable, {9, 255, 11, 7}},
  {sineWavetable, {9, 255, 11, 7}},
  {sineWavetable, {9, 255, 11, 7}},

  // Synth Lead
  {squareWavetable, {9, 255, 11, 3}},
  {sawtoothWavetable, {9, 255, 11, 3}},
  {triangleWavetable, {9, 255, 11, 5}},
  {triangleWavetable, {9, 128, 8, 6}},
  {sawtoothWavetable, {9, 192, 8, 5}},
  {sawtoothWavetable, {9, 192, 8, 5}},
  {guitarWavetable, {9, 192, 11, 5}},
  {sawtoothWavetable, {9, 255, 11, 3}},

  // Synth Pad
  {guitarWavetable, {9, 255, 11, 5}},
  {guitarWavetable, {9, 255, 11, 9}},
  {guitarWavetable, {4, 192, 9, 4}},
  {guitarWavetable, {9, 255, 11, 6}},
  {guitarWavetable, {9, 192, 10, 8}},
  {sawtoothWavetable, {9, 255, 11, 10}},
  {guitarWavetable, {9, 255, 11, 8}},
  {sawtoothWavetable, {9, 255, 11, 10}},

  // Synth Effects
  {guitarWavetable, {9, 128, 9, 4}},
  {sawtoothWavetable, {9, 128, 10, 10}},
  {triangleWavetable, {9, 192, 8, 4}},
  {guitarWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {10, 0, 11, 3}},
  {triangleWavetable, {9, 0, 11, 10}},
  {guitarWavetable, {9, 255, 11, 4}},
  {sawtoothWavetable, {9, 255, 11, 6}},

  // Ethnic
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {9, 0, 9, 4}},
  {sawtoothWavetable, {9, 0, 11, 4}},
  {sawtoothWavetable, {8, 0, 8, 4}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},
  {sawtoothWavetable, {9, 255, 11, 6}},

  // Percussive
  {squareWavetable, {9, 64, 9, 4}},
  {sawtoothWavetable, {8, 0, 8, 4}},
  {guitarWavetable, {9, 0, 11, 5}},
  {sineWavetable, {8, 0, 8, 4}},
  {randomWavetable, {8, 0, 8, 4}},
  {triangleWavetable, {8, 0, 8, 4}},
  {randomWavetable, {8, 0, 8, 4}},
  {randomWavetable, {8, 255, 7, 12}},

  // Sound Effects
  {randomWavetable, {8, 0, 8, 4}},
  {randomWavetable, {8, 0, 8, 4}},
  {randomWavetable, {10, 64, 12, 12}},
  {sineWavetable, {9, 255, 11, 4}},
  {randomWavetable, {9, 255, 11, 4}},
  {randomWavetable, {9, 255, 11, 12}},
  {randomWavetable, {10, 255, 10, 11}},
  {randomWavetable, {9, 0, 10, 3}},
};
#endif

