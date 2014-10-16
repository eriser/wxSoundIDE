#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#define WOFF     0
#define WSQUARE  1
#define WSAW     2
#define WTRI     3
#define WNOISE   4
#define WSAMPLE  5

#define QUIET      'Q'
#define REPORT     'R'

#define OSC1SET    'A'
#define OSC2SET    'B'
#define OSC3SET    'C'

#define OPITCHUP   'P'
#define OPITCHDN   'p'
#define OPITCHUPS   'O'
#define OPITCHDNS   'o'
#define OVOLUMEUP  'V'
#define OVOLUMEDN  'v'

#define OTOGGLE    '0'
#define OSQUARE    '1'
#define OSAW       '2'
#define OTRI       '3'
#define ONOISE     '4'

// ADSR OPTIONS

#define ADSRTOGGLE 'a'
#define ATTACKLESS 'k'
#define ATTACKMORE 'K'
#define DELAYLESS  'd'
#define DELAYMORE  'D'
#define SUSTAINLESS   's'
#define SUSTAINMORE   'S'
#define RELEASELESS   'e'
#define RELEASEMORE   'E'

// VOLUME & PITCH RAMP OPTIONS
#define VRAMPUP  '('
#define VRAMPDOWN ')'
#define PRAMPUP  '['
#define PRAMPDOWN ']'

// MODULATION OPTIONS
#define COMBINE    'c'
#define AM         'm'
#define FM         'f'

#define OLOCK      'l'

//#include <Arduino.h>

extern void testOsc();
extern void fakeISR();

typedef uint8_t byte;
typedef uint8_t boolean;

struct ADSR
{
  boolean on;
  byte vol;
  byte stage; // 0 for off, 1 for attack, 2 for decay, 3 for sustain, 4 for release
  long count; // counter for envelop effects
  long cycle; // period when adsrcount is increased
  byte attackinc; // speed of attack
  byte attacklevel; // end level of attack
  byte decayinc; //speed of decay
  byte decaylevel; // end level of decay
  int sustaincount;
  int sustain; // number of ticks to sustain, -1 forever
  char releaseinc; //speed of decrease to 0
};

struct OSC {
  byte on;
  byte wave;
  int pitch;
  unsigned int count;
  unsigned int inccount;
  int8_t increment;
  unsigned int inccycle;
  unsigned int halfcycle;

  unsigned int wslope;
  unsigned int wcycle;
  /** ADSR related properties **/



  unsigned int vol;
  unsigned int output; // output is stored as  16 bit value and shifted before its put in OCR2B
};

#endif
