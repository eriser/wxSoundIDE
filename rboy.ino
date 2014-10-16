#include <Arduino.h>
#include "Rboy.h"
#include "Game.h"
#include "picture.h"
#include "jpak.h"

Rboy Rb;
Game Game;
uint8_t scrbuffer[504]; // the screen buffer
int16_t cy = 48;

void drawJpak (int8_t x, int8_t y, const unsigned char * bmp, byte fader=1) {
    int8_t width,col, sx=x, sy=y, fstep = 0;
    uint16_t size;
    Jpak js(bmp);
    width = js.getWidth();
    size = js.getSize();

    while (fader) {
        x = sx; y = sy;
        col =0;
        js.rewindStream();

        for (uint16_t pixel = 0; pixel < size; pixel++ ) {
            if (x == sx+width) {
                x =sx;
                y++;
            }
            uint8_t col = js.readNextPixel();
            fstep++;
            if (fstep>=fader) {
                    Game.set_pixel(x,y,col);
                    fstep=0;
            }
            x++;

        }
        fader--;
        Game.refresh();
        }
}

void setup()
{
	Serial.begin(9600);
	Game.begin(&scrbuffer[0]);
	//pinMode(13, OUTPUT);
	/*Rb.begin(&scrbuffer[0]);
    Serial.println("Hello world!");
    Rb.set_cursor(0,cy);
    */

    //delay(10);
    drawJpak(TVXCENTER-17,TVYCENTER-13,rboylogo,45);
    while(!aBtn());
    drawJpak(0,5,maniax,25);
    while(!aBtn());
    drawJpak(0,0,bikepic,25);
    while(!aBtn());
    Game.gameSetup();
}

void loop()
{


    Game.update();
    #ifdef ARDUSIM
        delay(20);
    #endif
	//digitalWrite(13, HIGH);
	//Rb.fill(HATCH);
	//Rb.draw_line(0,0,XMAX,YMAX,0);
	//Rb.draw_rect(10,10,XMAX-10,YMAX-10,-1,HATCH2);
	/*Rb.refresh();

	//digitalWrite(13, LOW);
	Rb.fill(WHITE);
	//Rb.set_cursor(Rb.cursor_x, Rb.cursor_y--);
	//if (Rb.cursor_y  < -10) Rb.cursor_y = -10;
	//Rb.println("0123456789");
	//Rb.println("A B CDEFGH IJKLMNOPQRSTUVWXYZ");
	//Rb.println("\"!#$%&/()=+-<>@?");

	Rb.set_cursor(0,0);
	Rb.setClipRect(10,10,XMAX-10,YMAX-10);
	if (cy < -60) cy = -60;
	Rb.println("The door slowly opens, and a youngish looking man with curly brown hair stands before you. You examine each other silently for a few moments.\"Yes?\" he finally asks. \"How may I be of service?\"");
	if (Rb.cursor_y > Rb.clip_bottom) Serial.print ("Scrolling down enabled");
	/*Rb.print("This library is ");
	Rb.font_bg = true;
	Rb.font_color = WHITE;
	Rb.print("free");
	Rb.font_bg = false;
	Rb.font_color = BLACK;
	Rb.print(" software; you can redistribute it and modify it under the terms of the GNU");*/
	//Rb.refresh();
    //delay(10000);
}
