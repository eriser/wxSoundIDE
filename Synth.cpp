#include "Synth.h"
#include "oscillator.h"
//#include "sounds.h"
#include <math.h>
#include "portaudio.h"
#include "fastdiv.h"
#include <iostream>
#include <fstream>

/** Sound Variables **/

uint8_t data[NUMFRAMES]; //portaudio

uint8_t snddatanext=1; // which array sound data comes from next
uint8_t volume=255; //portaudio
PaStream *paStream;
PaError paErr;
uint8_t fakeOCR2B, fakeCount=0;
uint16_t skipstep = 0;
boolean PWMemulation = true;
uint8_t isLoop = true;


int pitch = 440, pitch2 = 100;
long count = 0;
long halfcycle = 0;
long sercheck=0;
boolean sertoggle = false;
int osc2inc = 10;
int pitchbend = 16;

OSC osc1,osc2;
Instrument patch;
OSC* oscptr;
boolean osc12Lock = false;
char tick=0;


typedef void (*waveFunction)(OSC*);

// Arduino compatibility

long random(long howbig)
{
  if (howbig == 0) {
    return 0;
  }
    long r;
    r = rand() % howbig;
    return  r;
}

long random(long howsmall, long howbig)
{
  if (howsmall >= howbig) {
    return howsmall;
  }
  long diff = howbig - howsmall;
  return random(diff) + howsmall;
}


void Synth::begin() {
    //testOsc();
    //initSound();
} // end of begin

static int paCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    /* Cast data passed through stream to our structure. */
    uint8_t *data = (uint8_t*)userData;
    uint8_t *out = (uint8_t*)outputBuffer;
    unsigned int i;
    unsigned long j =0;
    (void) inputBuffer; /* Prevent unused variable warning. */

    /** create sound buffer by using the ISR **/


    //osc1.count = osc2.count = 0;

    for (j=0;j<framesPerBuffer;j+=PWMLEVELS) {
            fakeISR(); /** create next sample **/
            if (!patch.playing) fakeOCR2B=0;

            /** Now create duty cycle **/
            skipstep = (fakeOCR2B+1) / PWMLEVELS;
            // if OCR2B is 255, skipstep is 16, and all output is 255
            // if OCR2B is 127, skipstep is 8, and 50% output is 255
            // if OCR2B is 64, skipstep is 4, and 75% output is 255

            if (PWMemulation) {
                for (i=0; i< PWMLEVELS; i++) {
                    if (i >= skipstep) {
                        *out++ = int(0+fakeOCR2B)/2;
                    }
                    else {
                        *out++ = int(255+fakeOCR2B)/2;
                    }
                }
            } else {
                for (i=0; i< PWMLEVELS; i++) { *out++ = fakeOCR2B; }

            }

    }

    return paContinue; /** THIS IS VERY IMPORTANT !!!! **/
}

void stopSound() {
    patch.playing = false;
    patch.count=0;
}


void initSound() {

    paErr = Pa_Initialize();
    if( paErr != paNoError ) goto error;

    paErr = Pa_OpenDefaultStream( &paStream,
                                0,          /* no input channels */
                                1,          /* mono output */
                                paUInt8,    /* 8 bit output */
                                SAMPLE_RATE,
                                NUMFRAMES,  /* frames per buffer was 256 */
                                paCallback,
                                &data );
    if( paErr != paNoError ) goto error;

    paErr = Pa_StartStream( paStream );
    if( paErr != paNoError ) goto error;
    return;

error:
    Pa_Terminate();
    return;

}

void killSound()
{
     paErr = Pa_StopStream( paStream );
     if( paErr != paNoError ) goto error;
     paErr = Pa_CloseStream( paStream );
     if( paErr != paNoError ) goto error;

error:
    Pa_Terminate();
    return;
}

void playSound(uint8_t loop, uint16_t length) {

    patch.loop = loop;
    patch.length = length;
    patch.playing = true;
    patch.count = 0;
    return;
}

/** SOUND FUNCTIONS **/


void waveoff(OSC* o){
  o->output = 0;
}

void sqwave(OSC* o){
  if (o->count > o->wcycle) {
    o->count=0;
    if (o->output) o->output = 0;
    else o->output = o->vol;
  }
}

void sawwave(OSC* o){
 o->output += o->wslope; // add 16 bit slope
 if (o->output > o->vol) o->output = 0;
}

void triwave(OSC* o){

if (o->increment) {
    if (o->output < o->vol - o->wslope) o->output += o->wslope;
    else o->increment = 0;
} else {
    if (o->output > o->wslope) o->output -= o->wslope;
    else o->increment = 1;
}

//if (o->count > o->wcycle) { o->wslope = ~o->wslope; o->count = 0;}

}

void noise(OSC* o){
  if (o->count > o->wcycle) {
    o->count=0;
    o->output = random(0,o->vol);
  }
}

void sample(OSC* o) {

    if (o->samplepos > o->samplelength ) o->samplepos = 0;

    if (o->count > o->wcycle) {
        o->count=0;
        if (o->output) o->output = 0;
        //else o->output = o->output=pgm_read_byte((uint32_t)(sfxBike) + o->inccount);
    }
}

#define NUMWAVES 5
waveFunction Farr []  = {waveoff, sqwave, sawwave, triwave, noise, sample};

void fakeISR(){

  osc1.count++;
  osc2.count++;
  patch.count++;

  if (patch.count >= patch.length) {
        if (patch.loop) patch.count = 0;
        else stopSound();
  }

  if (osc1.adsr.on) {
    if (patch.count > osc1.adsr.Rpos) osc1.adsrinc = 0;
    else if (patch.count > osc1.adsr.Spos) osc1.adsrinc = osc1.adsr.Rinc;
    else if (patch.count > osc1.adsr.Dpos) osc1.adsrinc = osc1.adsr.Sinc;
    else if (patch.count > osc1.adsr.Apos) osc1.adsrinc = osc1.adsr.Dinc;
    else osc1.adsrinc = osc1.adsr.Ainc;
    if (osc1.vol > osc1.adsrinc && osc1.vol < 65536 - osc1.adsrinc) osc1.vol += osc1.adsrinc;
  }

  //if (tick==7) {
    Farr[osc1.wave](&osc1);
    Farr[osc2.wave](&osc2);

    //fakeOCR2B = (osc1.output>>2)*(osc2.output>>2);
    fakeOCR2B = osc1.output>>8; // To output, shift back to 8-bit
    tick = 0;
  //} else tick++;
}

void setADSR(OSC* o, ADSR adsr){
     o->adsr.on = adsr.on;
     o->adsr.Apos = adsr.Apos;
     o->adsr.Dpos = adsr.Dpos;
     o->adsr.Spos = adsr.Spos;
     o->adsr.Rpos = adsr.Rpos;
     o->adsr.Aval = adsr.Aval;
     o->adsr.Dval = adsr.Dval;
     o->adsr.Sval = adsr.Sval;
     o->adsr.Rval = adsr.Rval;
     /*precalculate slopes*/
     o->adsr.Ainc = fastdiv((int16_t)adsr.Aval<<8, adsr.Apos+1);
     // Decay
     if (adsr.Aval > adsr.Dval) {
        o->adsr.Dinc = -fastdiv((int16_t)(adsr.Aval-adsr.Dval)<<8, (adsr.Dpos-adsr.Apos)+1);
     } else {
        o->adsr.Dinc = fastdiv((int16_t)(adsr.Dval-adsr.Aval)<<8, (adsr.Dpos-adsr.Apos)+1);
     }
     // Sustain
     if (adsr.Dval > adsr.Sval) {
        o->adsr.Sinc = -fastdiv((int16_t)(adsr.Dval-adsr.Sval)<<8, (adsr.Spos-adsr.Dpos)+1);
     } else {
        o->adsr.Sinc = fastdiv((int16_t)(adsr.Sval-adsr.Dval)<<8, (adsr.Spos-adsr.Dpos)+1);
     }
     // Release
     if (adsr.Sval > adsr.Rval) {
        o->adsr.Rinc = -fastdiv((int16_t)(adsr.Sval-adsr.Rval)<<8, (adsr.Rpos-adsr.Spos)+1);
     } else {
        o->adsr.Rinc = fastdiv((int16_t)(adsr.Rval-adsr.Sval)<<8, (adsr.Rpos-adsr.Spos)+1);
     }
}

void setOSC(OSC* o,byte on, byte wave,int pitch,byte volume){

  o->on = on;
  o->wave = wave;
  o->pitch = pitch;

  o->wcycle = fastdiv(SAMPLE_RATE/PWMLEVELS/2,pitch); // how many calls to ISR to complete a wave cycle
  o->vol = volume << 8;//volume;
  //o->halfcycle = (57000/2)/pitch; // or 56819
  switch (wave) {
    case WSAW:
    o->wslope = fastdiv(o->vol, o->wcycle); // rate of increase for wave as 16 bit value
    o->increment = 1;
    //o->wslope -= 5; //fast way to prevent overflow
    break;
    case WTRI:
    o->wslope = fastdiv(o->vol, (o->wcycle)); // rate of increase for wave as 16 bit value
    break;
  }

  o->output = 0;
}

void testOsc(){

  std::ofstream myfile;
  setOSC(&osc1,true,WOFF,100,127);
uint16_t i, j;
    myfile.open ("output.txt");

    /** create sound buffer by using the ISR **/

    //osc1.count = osc2.count = 0;

    for (j=0;j<NUMFRAMES;j+=PWMLEVELS) {
            fakeISR(); /** create next sample **/

            /** Now create duty cycle **/
            skipstep = (fakeOCR2B+1) / PWMLEVELS;
            // if OCR2B is 255, skipstep is 16, and all output is 255
            // if OCR2B is 127, skipstep is 8, and 50% output is 255
            // if OCR2B is 64, skipstep is 4, and 75% output is 255

            if (PWMemulation) {
                for (i=0; i< PWMLEVELS; i++) {
                    if (i >= skipstep) {
                        //*out++ = 0;
                    }
                    else {
                        //*out++ = 255;
                    }
                }
            } else {
                for (i=0; i< PWMLEVELS; i++) { myfile << int(fakeOCR2B) << ","; }

            }
            myfile << std::endl;
    }
    myfile.close();
  setOSC(&osc2,true,WSAW,100,240);

}
