#ifndef SYNTH_H
#define SYNTH_H

#include <stdint.h>
#include <stdlib.h>
#include "oscillator.h"

extern void testOsc();
extern void setPitch(int);
extern void setWave(int);
extern void setVolume(int);
extern void initSound();
extern void setOSC(OSC*,byte, byte,int,byte);

extern OSC osc1,osc2;


class Synth {
public:

	void begin();


private:


};

#endif /* SYNTH_H */
