#ifndef SYNTH_H
#define SYNTH_H

#include <stdint.h>
#include <stdlib.h>

extern void testOsc();
extern void setPitch(int);
extern void setWave(int);
extern void setVolume(int);
extern void initSound();


class Synth {
public:

	void begin();


private:


};

#endif /* SYNTH_H */
