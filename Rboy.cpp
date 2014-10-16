#include <Arduino.h>
#include "Rboy.h"
#include "Rboy_math.h"
#include "Print.h"

// PUBLIC
void Rboy::begin(uint8_t* buf) {
    initDisplay();
    //#ifdef SOUND
        testOsc();
        initSound();

    //#endif // SOUND
    clearDisplay();
    initButtons();
    buffer = buf; // set pointer to screen buffer
    cursor_x = cursor_y = 0;
    font_color = BLACK;
    font_bg = -1;
    chars_wanted = chars_printed = 0;
    clip_left=0,clip_right=XMAX, clip_top =0, clip_bottom = YMAX;
} // end of begin

void Rboy::fill(uint8_t color) {
	switch(color) {
		case BLACK:
			for (int i = 0; i < 504; i++)
				buffer[i] = 0xFF;
			break;
		case WHITE:
			for (int i = 0; i < 504; i++)
				buffer[i] = 0x00;
			break;
        case HATCH:
			for (int i = 0; i < 504; i++)
				buffer[i] = 0xAA;
			break;
		case INVERT:
			for (int i = 0; i < 504; i++)
				buffer[i] = ~buffer[i];
			break;
	}
} // end of fill

void Rboy::setClipRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    clip_left = x0; clip_right = x1; clip_top = y0; clip_bottom = y1;
}

void Rboy::refresh() {
	refreshDisplay(buffer);
}

void Rboy::set_pixel(uint8_t x, uint8_t y, char c) {
	if (x >= 84 || y >= 48) return;
	if (c==BLACK) {buffer[(y >> 3) * 84 + x] |= (0x80 >> (y & 7)); return;}
	buffer[(y >> 3) * 84 + x] &= ~(0x80 >> (y & 7));
}// end of set_pixel

void Rboy::draw_circle(uint8_t x0, uint8_t y0, uint8_t radius, char c, char fc) {

	int f = 1 - radius;
	int ddF_x = -2 * radius;
	int	ddF_y = 1;
	int x = radius;
	int y = 0;
	uint8_t pyy = y,pyx = x;

	if (fc != -1)
		draw_column(x0,y0-radius,y0+radius,fc);

	sp(x0, y0 + radius,c);
	sp(x0, y0 - radius,c);
	sp(x0 + radius, y0,c);
	sp(x0 - radius, y0,c);

	while(y < x) {
		if(f >= 0) {
			x--;
			ddF_x += 2;
			f += ddF_x;
		}
		y++;
		ddF_y += 2;
		f += ddF_y;


		if (fc != -1) {

			if (pyx != x) {
				draw_column(x0+x,y0-y,y0+y,fc);
				draw_column(x0-x,y0-y,y0+y,fc);
			}
			if (pyy != y && y != x) {
				draw_column(x0+y,y0-x,y0+x,fc);
				draw_column(x0-y,y0-x,y0+x,fc);
			}
			pyy = y;
			pyx = x;
		}
		sp(x0 + x, y0 + y,c);
		sp(x0 - x, y0 + y,c);
		sp(x0 + x, y0 - y,c);
		sp(x0 - x, y0 - y,c);
		sp(x0 + y, y0 + x,c);
		sp(x0 - y, y0 + x,c);
		sp(x0 + y, y0 - x,c);
		sp(x0 - y, y0 - x,c);
	}
}

void Rboy::draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, char c) {

if ((uint8_t)x0 > XMAX || (uint8_t)y0 > YMAX || (uint8_t)x1 > XMAX || (uint8_t)y1 > YMAX ) {
    if (clipLine (&x0,&y0,&x1,&y1)==0) return; // line out of bounds
}

	if (x0 == x1)
		draw_column(x0,y0,y1,c);
	else if (y0 == y1)
		draw_row(y0,x0,x1,c);
	else {
		int e;
		signed int dx,dy,j, temp;
		signed char s1,s2, xchange;
		signed int x,y;

		x = x0;
		y = y0;

		//take absolute value
		if (x1 < x0) {
			dx = x0 - x1;
			s1 = -1;
		}
		else if (x1 == x0) {
			dx = 0;
			s1 = 0;
		}
		else {
			dx = x1 - x0;
			s1 = 1;
		}

		if (y1 < y0) {
			dy = y0 - y1;
			s2 = -1;
		}
		else if (y1 == y0) {
			dy = 0;
			s2 = 0;
		}
		else {
			dy = y1 - y0;
			s2 = 1;
		}

		xchange = 0;

		if (dy>dx) {
			temp = dx;
			dx = dy;
			dy = temp;
			xchange = 1;
		}

		e = ((int)dy<<1) - dx;

		for (j=0; j<=dx; j++) {
			sp(x,y,c);

			if (e>=0) {
				if (xchange==1) x = x + s1;
				else y = y + s2;
				e = e - ((int)dx<<1);
			}
			if (xchange==1)
				y = y + s2;
			else
				x = x + s1;
			e = e + ((int)dy<<1);
		}
	}
}

void Rboy::draw_row(int8_t y, int16_t x0, int16_t x1, int8_t c) {
	uint8_t hbit;

    if (x0 > x1) {
			hbit = x0;
			x0 = x1;
			x1 = hbit;
		} // swap the xs correct way round

	if (x0 == x1)
		sp(x0,y,c);
	else {

	hbit = 0x80 >> (y&7); //find the bit to be set in that byte
                          // remainder = number & ( divisor - 1 )

    x0 = (y>>3)*84 + x0 ;         // x0 now points to first buffer byte
    x1 = (y>>3)*84 + x1 ;         // x1 now points to last buffer byte

    // Drawing loops

    if (c == BLACK) {
			while (x0 <= x1)
				buffer[x0++] |= hbit;
		}
		else if (c == WHITE) {
			while (x0 <= x1)
				buffer[x0++] &= ~hbit;
		}
		else if (c == INVERT) {
			while (x0 <= x1)
				buffer[x0++] ^= hbit;
		}
		else if (c == HATCH) {
			while (x0 <= x1)
				if ((y+x0)&1) {
                    // odd bits set
                    buffer[x0++] |= hbit;
				} else {
                    // even bits turn off
                    buffer[x0++] &= ~hbit;
				}
		}
	}
} // end of draw_row

void Rboy::draw_column(int8_t x, int16_t y0, int16_t y1, int8_t c) {

	unsigned char topbits, bottombits, hatchbit=0;

    if (y0 > y1) {
			topbits = y0;
			y0 = y1;
			y1 = topbits;
		} // swap the ys correct way round

    if (y0 == y1)
		{
		//set_pixel(x,y0,c); // draw pixel if length = 1
		return;
		}

    switch (c) {
    case HATCH:
        if (x&1) hatchbit = HATCH1ODD;
        else hatchbit = HATCH1EVEN;
        break;
    case HATCH2:
        if (x&1) hatchbit = HATCH2ODD;
        else hatchbit = HATCH2EVEN;
        break;
    case HATCH3:
        if (x&1) hatchbit = HATCH3ODD;
        else hatchbit = HATCH3EVEN;
        break;
    case RANDOM:
        hatchbit = random(1,256);
        break;
    }

    topbits = 0xFF >> (y0&7); // mask for top byte
    bottombits = 0xFF << (7-(y1&7)); // mask for bottom byte

    y0 = (y0>>3)*84 + x; // y0 now points to topmost byte
    y1 = (y1>>3)*84 + x; // y1 now points to last bottom byte

    if (y0==y1) {
        if (topbits && bottombits) {
            topbits &= bottombits;
            if (topbits) bottombits = topbits;
        }
    } // same byte !!
    // Drawing loop

    if (c == BLACK) {

            buffer[y0] |= topbits; // topmost byte
            y0 += 84;           // increment, if several bytes
			while ( y0 < y1) {
				buffer[y0] = 0xFF; // its a whole byte
				y0 += 84;           // increment, if several bytes
			}
			buffer[y1] |= bottombits; // last byte
		}
		else if (c == WHITE) {

            buffer[y0] &= ~topbits; // topmost byte
            y0 += 84;           // increment, if several bytes
            while ( y0 < y1) {
				buffer[y0] = 0x00; // its a whole byte
				y0 += 84;           // increment, if several bytes
			}
			buffer[y1] &= ~bottombits; // last byte
		}
		else if (c == INVERT) {

            buffer[y0] ^= topbits; // topmost byte
            y0 += 84;           // increment, if several bytes
            while ( y0 < y1) {
				buffer[y0] ^= 0xFF; // its a whole byte
				y0 += 84;           // increment, if several bytes
			}
			buffer[y1] ^= bottombits; // last byte
		}
        else if (hatchbit) {
            buffer[y0] &= ~topbits; // switch off affected bytes
            buffer[y0] |= hatchbit & topbits; // topmost byte
            y0 += 84;           // increment, if several bytes
            while ( y0 < y1) {
				buffer[y0] = hatchbit; // its a whole byte
				y0 += 84;           // increment, if several bytes
			}
			buffer[y1] &= ~bottombits; // switch off affected bytes
			buffer[y1] |= hatchbit & bottombits; // last byte
		}
}

void Rboy::draw_rect(int8_t x0, int8_t y0, int8_t x1, int8_t y1, char c, char fc) {

    int8_t temp;
    if (x0 < 0) x0=0;
    if (x1 < 0) x1=0;
    if (x0 > XMAX) x0=XMAX;
    if (x1 > XMAX) x1=XMAX;

	if (fc != -1) {
		if (x0 < x1) {
		for (unsigned char i = x0; i < x1; i++) draw_column(i,y0,y1,fc);
		} else {
        for (unsigned char i = x1; i < x0; i++) draw_column(i,y0,y1,fc);
		}
	}

	if (c != -1) {
        draw_line(x0,y0,x1,y0,c);
        draw_line(x0,y0,x0,y1,c);
        draw_line(x1,y0,x1,y1,c);
        draw_line(x0,y1,x1,y1,c);
	}
} // end of draw_rect

void Rboy::draw_poly(uint8_t n, pointXYArray& pnts, char c, char fc){

    int16_t xmax, xmin; // extremes
    int16_t x1, x2;
    uint8_t xminPoint = 0, xmaxPoint = 0; // extremes
    uint8_t p1, p2;
    uint8_t i;

    if (n < 3) return; // not a polygon !

    if (fc != -1) { // if fillcolor is set

    xmax = xmin = pnts[0].x; // initialize to point 0 of polygon

    // FIND EXTREMES
    for (i = 1; i < n; i++)   // iterate through points of polygon
    {
        if (pnts[i].x > xmax)   // if current point is > current xmax
        {
            xmax = pnts[i].x;   // new xmax is at current point
            xmaxPoint = i;      // store this information
        }

        if (pnts[i].x < xmin)   // if current point is < current xmin
        {
            xmin = pnts[i].x;   // new xmin is at current point
            xminPoint = i;      // store this information
        }
    }

    // ROUND THE FOUND EXTREMES TO SCANLINES
    //x1 = (uint8_t)round2Scanline(xmin);
    //x2 = (uint8_t)round2Scanline(xmax);
    // disabled because already working with integer numbers
    x1 = (xmin);
    x2 = (xmax);
    if (x1 == x2) return; // polygon is 100% vertical = DO NOT DRAW !

    // START WALKING LEFT TO RIGHT
    p1 = xminPoint;     // begins here
    p2 = xminPoint + 1; // towards next point
    if (p2 >= n) p2 = 0;  // if p2 > number of points, wrap to p0

    do {
        walkEdge(&edgeTable1[0], &pnts[p1], &pnts[p2]);
        // then, ready for walking the next edge
        p1 = p2;        // last right point becomes new left point
        p2 = p2 + 1;    // next point please !
        if (p2 >= n) p2 = 0; // again, wrap to p0 if needed
    } while (p1 != xmaxPoint); //... until we arrive at right


    // START WALKING RIGHT TO LEFT
    p1 = xmaxPoint;     // begins here
    p2 = xmaxPoint + 1; // towards next point
    if (p2 >= n) p2 = 0;  // if p2 > number of points, wrap to p0

    do {
        walkEdge(&edgeTable2[0], &pnts[p1], &pnts[p2]);
        // then, ready for walking the next edge
        p1 = p2;        // last right point becomes new left point
        p2 = p2 + 1;    // next point please !
        if (p2 >= n) p2 = 0; // again, wrap to p0 if needed
    } while (p1 != xminPoint); //... until we arrive back to left

    // NOW... DRAW THE COLUMNS TO FILL IN !
    if (x1 < 0) x1 = 0;
    if (x2 > XMAX) x2 = XMAX;
    do
    {
        // x = location to draw AND index to edge table
        draw_column(x1, edgeTable1[x1], edgeTable2[x1], fc);
        x1++;
    } while (x1 < x2+1);

    } // end of if fill colour c != -1

    // DRAW EDGES IF EDGE COLOR IS SET //

    if (c!=-1) {
         //n=2; // for debug purposes
         for (i = 0; i < n-1; i++)   // iterate through points of polygon
         {
            draw_line(pnts[i].x, pnts[i].y,pnts[i+1].x,pnts[i+1].y,c);
         }
         // draw last line to p0
         draw_line(pnts[0].x, pnts[0].y,pnts[n-1].x, pnts[n-1].y,c);
    }
};


// PRIVATE

void Rboy::sp(uint8_t x, uint8_t y, char c) {
    if (c==BLACK) {buffer[(y >> 3) * 84 + x] |= (0x80 >> (y & 7)); return;}
    if (c==HATCH) {
        if ((x+y) & 1) {
                // odd pixels on
                buffer[(y >> 3) * 84 + x] |= (0x80 >> (y & 7));
                } else {
                // even pixels off
                buffer[(y >> 3) * 84 + x] &= ~(0x80 >> (y & 7));
                }
        return;
        }
	buffer[(y >> 3) * 84 + x] &= ~(0x80 >> (y & 7));
}

// walk edge horizontally, storing edge y's along the way

void Rboy::walkEdge(uint8_t *edgeTable, pointXY *p1, pointXY *p2)
{
    rb14 y, dy, dx;
    //int dx, x1, x2,temp;
    int x1, x2,temp;

    // make sure p1 of edge is leftmost
    if (p2->x < p1->x) swapWT(pointXY *, p1, p2);

    //x1 = round2Scanline(p1->x);
    //x2 = round2Scanline(p2->x);
    x1 = (p1->x);
    x2 = (p2->x);
    dx = (x2 - x1);

    // top left included, bottom right excluded
    dx = (((2) > (dx-1)) ? (2) : (dx-1));

    dx = int2rb(x2 - x1);

    if (dx == 0) return; // avoid divide by zero

    y = int2rb(p1->y);  // starting y for walk
    dy = rbDiv(int2rb((p2->y - p1->y)), dx); // y increment for walk

    if (x2 > XMAX) x2=XMAX;
    if (x1 < 0) {
        y = rbAdd(y,rbMul(dy,int2rb(-x1)));
        x1 = 0;
    }

    do {
        //if ( x1 > -1) {
         temp = rb2int(y);
         if (temp < 0) temp = 0; //check bounds
         else if (temp > YMAX) temp = YMAX; //check bounds
         edgeTable[x1] = temp;   // store current edge y at index x in table
        //}
        y += dy;        // increment y by defined step
        x1++;           // step rightward
    } while(x1 <= x2);  // until rightmost point of edge is reached

}

// clip line to window boundaries
char Rboy::clipLine(int16_t *x0, int16_t *y0, int16_t *x1, int16_t *y1)
{
    // Check X bounds
	if (*x1<*x0) {
        //std::swap (*x1,*x0); // swap so that we dont have to check x1 also
        swapWT(int16_t*,x1,x0);
        //std::swap (*y1,*y0); // y needs to be swaaped also
        swapWT(int16_t*,y1,y0);
	}

	if (*x0>XMAX) return 0; // whole line is out of bounds

	// Clip against X0 = 0
	if (*x0 < 0) {
        if ( *x1 < 0) return 0; // nothing visible
        int16_t dx = (*x1 - *x0);
        int16_t dy = ((*y1 - *y0) << 8); // 8.8 fixed point calculation trick
        int16_t m = dy/dx;
        *y0 = *y0 + ((m*-*x0)>>8); // get y0 at boundary
        *x0 = 0;
	}

	// Clip against x1 = 83
	if (*x1 > XMAX) {
        int16_t dx = (*x1 - *x0);
        int16_t dy = ((*y1 - *y0) << 8); // 8.8 fixed point calculation trick
        int16_t m = dy/dx;
        //*y1 = *y1 + ((m*(*x1-XMAX))>>8); // get y0 at boundary
        *y1 = *y1 + ((m*(XMAX-*x1))>>8); // get y0 at boundary
        *x1 = XMAX;
	}

    // Check Y bounds
	if (*y1<*y0) {
        //std::swap (*x1,*x0); // swap so that we dont have to check x1 also
        swapWT(int16_t*,x1,x0);
        //std::swap (*y1,*y0); // y needs to be swaaped also
        swapWT(int16_t*,y1,y0);
	}

	if (*y0>YMAX) return 0; // whole line is out of bounds

    if (*y0 < 0) {
        if ( *y1 < 0) return 0; // nothing visible
        int16_t dx = (*x1 - *x0) << 8;
        int16_t dy = (*y1 - *y0); // 8.8 fixed point calculation trick
        int16_t m = dx/dy;
        *x0 = *x0 + ((m*-*y0)>>8); // get x0 at boundary
        *y0 = 0;
	}

    // Clip against y1 = 47
	if (*y1 > YMAX) {
        int16_t dx = (*x1 - *x0) << 8;
        int16_t dy = (*y1 - *y0); // 8.8 fixed point calculation trick
        int16_t m = dx/dy;
        *x1 = *x1 + ((m*(YMAX-*y1))>>8); // get y0 at boundary
        //*x1 = *x1 + ((m*(*y1-YMAX))>>8); // get y0 at boundary
        *y1 = YMAX;
	}
	return 1; // clipped succesfully
}

char Rboy::clipLine(pointXY *p0, pointXY *p1)
{
    // clips int16_t values against window boundaries
    // returns 0 if points are within bounds
    // bit 1 (1) set if p1 was clipped
    // bit 2 (2) set if p2 was clipped
    // bit 3 (4) set if points were swapped
    // bit 4 (8) set if whole line is out of bounds

    char clipval = 0; // default to out of bounds

    // Check X bounds

    if (p0->x >= 0 && p0->x <= XMAX && p0->y >= 0 && p0->y <= YMAX && p1->x >= 0 && p1->x <= XMAX && p1->y >= 0 && p1->y <= YMAX)
    return clipval; // is within window, no need to clip


	if (p1->x < p0->x) {
        //std::swap (*p1,*p0); // swap so that we dont have to check x1 also
        swapWT(pointXY*,p1,p0);
        clipval ^= PSWAPPED; // points have been swapped
	}

    // calculate gradient
    rb14 dx = rbSub(int2rb(p1->x),int2rb(p0->x));
    rb14 dy = rbSub(int2rb(p1->y),int2rb(p0->y));
    rb14 m = rbDiv(dy,dx);

	// Clip against X0 = 0
	if (p0->x < 0) {
        p0->y = rbAdd(p0->y,rbMul(m,-p0->x)); // get y0 at boundary
        p0->x = 0;
        if (!(clipval & PSWAPPED)) clipval |= P0CLIPPED;
        else clipval |= P1CLIPPED;
	}

	// Clip against x1 = 83
	if (p1->x > XMAX) {
        p1->y = rbAdd(p1->y,rbMul(m,rbSub(XMAX,p1->x)));
        p1->x = XMAX;
        if (!(clipval & PSWAPPED)) clipval |= P1CLIPPED;
        else clipval |= P0CLIPPED;
	}

    // Check Y bounds
	if (p1->y<p0->y) {
        //std::swap (*p1,*p0); // swap so that we dont have to check x1 also
        swapWT(pointXY*,p1,p0);
        clipval ^= PSWAPPED;
	}

    dx = rbSub(int2rb(p1->x),int2rb(p0->x));
    dy = rbSub(int2rb(p1->y),int2rb(p0->y));
    m = rbDiv(dx,dy);

    if (p0->y < 0) {
        p0->x = rbAdd(p0->x,rbMul(m,-p0->y)); // get y0 at boundary
        p0->y = 0;
        if (!(clipval & PSWAPPED)) clipval |= P0CLIPPED;
        else clipval |= P1CLIPPED;
	}

    // Clip against y1 = 47
	if (p1->y > YMAX) {
        p1->x = rbAdd(p1->x,rbMul(m,rbSub(YMAX,p1->y)));
        p1->y = YMAX;
        if (!(clipval & PSWAPPED)) clipval |= P1CLIPPED;
        else clipval |= P0CLIPPED;
	}

	if (clipval & PSWAPPED) {
        // for clarity's sake, swap them back in order
        //std::swap (*p1,*p0);
        swapWT(pointXY*,p1,p0);
	}
	return clipval; // clipped succesfully
}
