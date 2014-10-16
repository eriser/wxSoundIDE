#ifndef RBOY_H
#define RBOY_H

#include <stdint.h>
#include <stdlib.h>
#include "Rboy_math.h"

#ifdef WIN32
    #include "Rboy_SIM.h"
    #include <vector>
    #define PROGMEM
    #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
    #define pgm_read_word(addr) (*(const unsigned short *)(addr))
#else
    #include "Rboy_HW.h"
    //#include <pnew.cpp>
    #include <iterator>
    #include <vector>
#endif

/** useful defines */

#define WHITE   0
#define BLACK   1
#define HATCH   2
#define HATCH2  3
#define HATCH3  4
#define INVERT  5
#define RANDOM  6

#define HATCH1EVEN  0xAA
#define HATCH1ODD   0x55
#define HATCH2EVEN  0x88
#define HATCH2ODD   0x22
#define HATCH3EVEN  0xDD
#define HATCH3ODD   0x77

#define XMAX    83
#define YMAX    47

#define KEEP_WORDS_INTACT   0
#define REMOVE_START_WHITESPACE 1


#define swapWT(type, a, b)    \
{               \
    type _t_;   \
    _t_ = a;    \
    a = b;      \
    b = _t_;    \
}

#define P0CLIPPED   1
#define P1CLIPPED   2
#define PSWAPPED    4

#define clearDisplay()	fillDisplay(0)

typedef struct
{
    int16_t x, y; // screen xy coordinates, 8 bits is not enough
} pointXY;

typedef std::vector<pointXY> pointXYArray; // display clipped xy's

extern void testOsc();
extern void setPitch(int);


class Rboy : public Print {
public:
    uint8_t* buffer; // Pointer to screen buffer
    uint8_t edgeTable1[84]; // edge tables for polygon filling
    uint8_t edgeTable2[84]; // ToDo: make optional if not needed
    int16_t cursor_x, cursor_y; // position of text cursor
    int8_t font_color, font_bg;
    uint8_t chars_wanted, chars_printed, scroll_up, scroll_down;
    uint8_t clip_left,clip_right, clip_top, clip_bottom;
	void begin(uint8_t*);
	void fill(uint8_t);
	void set_pixel(uint8_t,uint8_t,char);
    void draw_circle(uint8_t, uint8_t, uint8_t, char, char);
	void draw_line(int16_t,int16_t, int16_t, int16_t, char);
    void draw_column(int8_t, int16_t, int16_t, int8_t);
    void draw_row(int8_t, int16_t, int16_t, int8_t);
    void draw_rect(int8_t,int8_t,int8_t,int8_t,char,char);
    void refresh();
    void draw_poly(uint8_t, pointXYArray&, char, char);
    virtual size_t write(uint8_t);
    void set_cursor(int16_t,int16_t);
    void draw_char(uint8_t, uint8_t, unsigned char, uint8_t);
    void outputWord();
    void setClipRect(uint8_t, uint8_t, uint8_t, uint8_t);
    void alphabitmap(int8_t, int8_t, const unsigned char*, const unsigned char*,boolean, rb14 scale=1);
    void overlaybitmap(int16_t, int16_t, const unsigned char * bmp, uint16_t, uint8_t, uint8_t);
    void bitmap(uint8_t x, uint8_t y, const unsigned char * bmp, uint16_t i = 0, uint8_t width = 0, uint8_t lines = 0);
    void mirrorbitmap(int16_t, int16_t, const unsigned char * bmp, uint16_t, uint8_t, uint8_t);
    void scalebitmap(int8_t, int8_t, const unsigned char*, const unsigned char*,boolean, rb14 scale=1);
    void rlebitmap(int16_t,int16_t,const unsigned char*);

    void print(int x,int y,int v);
    void print(int x,int y,const char str[]);

private:
    void sp(uint8_t,uint8_t,char);
    void walkEdge(uint8_t *, pointXY *, pointXY *);
    char clipLine(int16_t*, int16_t*, int16_t*, int16_t*);
    char clipLine(pointXY*, pointXY*);
};

#endif /* RBOY_H */

