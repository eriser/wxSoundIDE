#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "Rboy_SIM.h"
#include "oscillator.h"
#include "sounds.h"

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "portaudio.h"

#define NUM_SECONDS   (5)
#define SAMPLE_RATE   (384000)
#define NUMFRAMES 38400 //100 ms before refresh
#define PWMLEVELS 16

using namespace std;
using namespace sf;

/** Sound Variables **/

uint8_t data[NUMFRAMES]; //portaudio

uint8_t snddatanext=1; // which array sound data comes from next
uint8_t volume=255; //portaudio
PaStream *paStream;
PaError paErr;
uint8_t fakeOCR2B, fakeCount=0;
uint16_t skipstep = 0;


int pitch = 440, pitch2 = 100;
long count = 0;
long halfcycle = 0;
long sercheck=0;
boolean sertoggle = false;
int osc2inc = 10;
int pitchbend = 16;

OSC osc1,osc2;
ADSR adsr;
OSC* oscptr;
boolean osc12Lock = false;
char tick=0;
char oscmode = COMBINE;

typedef void (*waveFunction)(OSC*);


uint8_t buttons_state, buttons_held; // from LSB up,down,left,right,a,b,c

//RenderWindow emuWindow;

sf::Texture simTexture;
sf::Sprite simSprite;
sf::RectangleShape simPixel0, simPixel1;
sf::Color col0(250, 250, 250); // background
sf::Color col1(50, 50, 50); // foreground
RenderWindow simWindow{{SIMW, SIMH}, "Rboy Simulator"};
sf::View simScreen(sf::FloatRect(0, 0, 84, 48));
sf::Event simEvent;
sf::Clock simClock;


int getMemory() {
  return 2048;
}

int pollEsc() {
simWindow.pollEvent(simEvent);
    if(simEvent.type == sf::Event::Closed || Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
       simWindow.close();
       return -1;
    }
return 0;
}

void initDisplay() {
    simWindow.setFramerateLimit(60);
    simPixel0.setSize(sf::Vector2f(1,1));
    simPixel0.setFillColor(col0);
    simPixel1.setSize(sf::Vector2f(1,1));
    simPixel1.setFillColor(col1);
    simScreen.zoom(SIMZ);
    simScreen.setViewport(sf::FloatRect(0, 0, 1, 1));
    simWindow.clear(col0);
    simWindow.setView(simScreen);
}


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
    long j =0;
    (void) inputBuffer; /* Prevent unused variable warning. */


    uint8_t skipcounter = 0;

    /** create sound buffer by using the ISR **/

    for (j=0;j<framesPerBuffer;j+=PWMLEVELS) {
            fakeISR(); /** create next sample **/

            /** Now create duty cycle **/
            skipstep = (fakeOCR2B+1) / PWMLEVELS;
            // if OCR2B is 255, skipstep is 16, and all output is 255
            // if OCR2B is 127, skipstep is 8, and 50% output is 255
            // if OCR2B is 64, skipstep is 4, and 75% output is 255

            for (i=0; i< PWMLEVELS; i++) {
                if (i >= skipstep) {

                        *out++ = 0;
                }
                else {
                        *out++ = 255;

                }
            }
    }
    skipcounter = 0;
    return paContinue; /** THIS IS VERY IMPORTANT !!!! **/
}

void stopSound() {
    paErr = Pa_StopStream( paStream );
    paErr = Pa_CloseStream( paStream );
    Pa_Terminate();
}

void setPitch(int pitch){
  // first we need to test if pitch is already set
  // if not, create buffer and reload oscillators
  long temp;
  if (pitch == osc1.pitch) return; //no need to change sound

  // otherwise recalculate

  pitch++; // avoid division by zero
  osc1.pitch = pitch;
  //temp = (384000/2)/pitch; // or 56819
  temp = (24000/2)/pitch; // or 56819
  osc1.halfcycle = (uint16_t) temp;

  osc2.pitch = pitch/2;
  temp = temp/2; // or 56819

  osc2.halfcycle = (uint16_t) temp;
  if (osc2.vol != 0) osc2.inccycle = osc2.halfcycle / osc2.vol ; // number of counts per increment
  osc2.increment = 1;
}




void initSound() {
    uint32_t loop = 200;
    int8_t volstep=1;
    //setPitch(100);

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
    Pa_Sleep(200);
    if( paErr != paNoError ) goto error;
    return;

error:
    Pa_Terminate();
    return;

}


void refreshDisplay(uint8_t* buffer) {
    simWindow.setView(simScreen);
    simWindow.clear(col0);

    int x=0, y=0;
    for (uint16_t byteptr=0; byteptr < 504; byteptr++) {
        for (int8_t bitptr=0; bitptr < 8; bitptr ++) {
            if (buffer[byteptr] & (0x80 >> (bitptr&7)))
                {
                    simPixel1.setPosition(x,y+bitptr);
                    simWindow.draw(simPixel1);
                } else {
                    //simPixel0.setPosition(x,y+bitptr);
                    //simWindow.draw(simPixel0);
                }
            }
        x++;
        if (x==84) { y += 8; x=0;}
    }

    simWindow.display();
    pollEsc();
}

void fillDisplay(uint8_t color) {
    simWindow.setView(simScreen);
    simWindow.clear(col0);

    int x=0, y=0;
    for (uint16_t byteptr=0; byteptr < 504; byteptr++) {
        for (int8_t bitptr=0; bitptr < 8; bitptr ++) {
                    if (color) {
                    simPixel1.setPosition(x,y+bitptr);
                    simWindow.draw(simPixel1);
                    }
            }
        x++;
        if (x==84) { y += 8; x=0;}
    }
    simWindow.display();
    pollEsc();
}

void initButtons() {
  buttons_state = buttons_held = 0;
}

void pollButtons() {
    uint8_t buttons_state_old = buttons_state;
    buttons_state = 0; // clear all
    if (upBtn()) buttons_state |= (1<<UPBIT);
    if (downBtn()) buttons_state |= (1<<DOWNBIT);
    if (leftBtn()) buttons_state |= (1<<LEFTBIT);
    if (rightBtn()) buttons_state |= (1<<RIGHTBIT);
    if (aBtn()) buttons_state |= (1<<ABIT);
    if (bBtn()) buttons_state |= (1<<BBIT);
    if (cBtn()) buttons_state |= (1<<CBIT);
    buttons_held = buttons_state & buttons_state_old; // only if both 1, end result is 1
}

uint8_t upHeld() { if (buttons_held & (1<<UPBIT)) return 1; return 0;}
uint8_t downHeld() { if (buttons_held & (1<<DOWNBIT)) return 1; return 0;}
uint8_t leftHeld() { if (buttons_held & (1<<LEFTBIT)) return 1; return 0;}
uint8_t rightHeld() { if (buttons_held & (1<<RIGHTBIT)) return 1; return 0;}
uint8_t aHeld() { if (buttons_held & (1<<ABIT)) return 1; return 0;}
uint8_t bHeld() { if (buttons_held & (1<<BBIT)) return 1; return 0;}
uint8_t cHeld() { if (buttons_held & (1<<CBIT)) return 1; return 0;}

uint8_t leftBtn(void) {
  simWindow.pollEvent(simEvent);
  if (Keyboard::isKeyPressed(Keyboard::Key::Left)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t rightBtn(void) {
  simWindow.pollEvent(simEvent);
  if (Keyboard::isKeyPressed(Keyboard::Key::Right)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t upBtn(void) {
  simWindow.pollEvent(simEvent);
  if (Keyboard::isKeyPressed(Keyboard::Key::Up)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t downBtn(void) {
  simWindow.pollEvent(simEvent);
  if (Keyboard::isKeyPressed(Keyboard::Key::Down)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t aBtn(void) {
  simWindow.pollEvent(simEvent);
  if (Keyboard::isKeyPressed(Keyboard::Key::Z)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t bBtn(void) {
  simWindow.pollEvent(simEvent);
  if (Keyboard::isKeyPressed(Keyboard::Key::X)) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t cBtn(void) {
  simWindow.pollEvent(simEvent);
  if (Keyboard::isKeyPressed(Keyboard::Key::C)) {
    return 1;
  } else {
    return 0;
  }
}

/** SOUND FUNCTIONS **/


void waveoff(OSC* o){
  o->output = 0;
}

void sqwave(OSC* o){
  if (o->count > o->halfcycle) {
    o->count=0;
    if (o->output) o->output = 0;
    else o->output = o->vol;
  }
}

void sawwave(OSC* o){
 if (o->count > o->halfcycle) {
   o->count=0;
   o->output=0;
 }
 if (o->inccount > o->inccycle) {
   o->inccount=0;
   o->output++;
 }
}

void triwave(OSC* o){
 if (o->inccount > o->inccycle) {
     o->inccount=0;
     o->output += o->increment;
   }

 if (o->count > o->halfcycle) {
   o->increment = ~(o->increment);
   o->count=0;
   o->output=0;
 }
}

void noise(OSC* o){
  if (o->count > o->halfcycle) {
    o->count=0;
    o->output = random(0,o->vol);
  }
}

void sample(OSC* o) {

    if (o->inccount > 640) o->inccount = 0;

    if (o->count > o->halfcycle) {
        o->count=0;
        if (o->output) o->output = 0;
        else o->output = o->output=pgm_read_byte((uint32_t)(sfxBike) + o->inccount);
    }
}

#define NUMWAVES 5
waveFunction Farr []  = {waveoff, sqwave, sawwave, triwave, noise, sample};

void fakeISR(){

  osc1.count++;
  osc1.inccount++;

  osc2.count++;
  osc2.inccount++;

  //if (tick==7) {
    Farr[osc1.wave](&osc1);
    Farr[osc2.wave](&osc2);

    //fakeOCR2B = (osc1.output>>2)*(osc2.output>>2);
    fakeOCR2B = osc2.output;
    tick = 0;
  //} else tick++;
}

void setOSC(OSC* o,byte on, byte wave,int pitch,byte volume){
  long temp;
  o->on = on;
  o->wave = wave;
  o->pitch = pitch;
  o->count = 0;
  //temp = (384000/2)/pitch; // or 56819
  temp = (24000/2)/pitch; // or 56819
  o->halfcycle = (uint16_t) temp;
  //o->halfcycle = (57000/2)/pitch; // or 56819
  switch (wave) {
    case WSAW:
    o->inccycle = o->halfcycle/(volume/2); // number of counts per increment
    break;
    case WTRI:
    o->inccycle = o->halfcycle / volume ; // number of counts per increment
    o->increment = 1;
    break;
  }
  o->vol = volume;//volume;
  o->output = 0;
}

void testOsc(){

  setOSC(&osc1,true,WSAMPLE,100,127);
  setOSC(&osc2,true,WSAW,100,255);

}



