#include <Arduino.h>
#include "Rboy.h"
#include "Game.h"
#include "Graphics.h"

#define GROUND
#define RUMBLES
#define CURVED
//#define DEMO
#define ROADEFFECT 7 // was 4
#define SEGMENTS
#define WORLDSPEED 10 // was 32, smaller is faster
#define ZCARSPEED 3
#define LEANTIMER 1 // was 5
#define LEANEFFECT 1
#define LEANFORCE -10
#define LEANRETURN 10
#define MINREVS 50
#define BRAKESPEED 35
#define DRAGSPEED 5
//#define DRAWFORCES
#define FORCETOLERANCE 9
#define RUMBLEDECELERATE 30
#define ROADSIDE

unsigned long z_world=0,nextz,nexto;
int z_car=0,zspeed = 0, wheeltick=0, carx=TVXCENTER-12,cary=CARY,
    acceltick=0, deceltick=1,fumeframe=0,roadx=0,skytick=0,
    skyx=0,skyy=-1,segment=0,osegment=0,curvtime=0,curvcount=0,xaccel=0,dynamichz=HORIZON;

int8_t lean = 0, leantick = 0, gear=0, soundEvent=0;
int16_t cgvector=0,leanvector,revs=0, best = 0, laptime = millis()/100;

int nexthill,nexthz,hilloffset=256;
byte lanes=3,car_dir=UP,lastRoad,segvisible=false,curveover=true,hillstate;
signed char wheeloffset=0;int zlookup[YTABS],xlookup[YTABS];
int curvature[YTABS+5];
int8_t leftside[48];
int8_t rightside[48];
int xspots[NUMSPOTS];
int zspots[NUMSPOTS];
int xonscreen[NUMSPOTS];
int yonscreen[NUMSPOTS];
int xcars[NUMCARS];
int zcars[NUMCARS];
int drawn=0;



void Game::update() {
    //delay_frame(10);

    draw_rect(0,0,TVX,dynamichz,-1,WHITE);

    if (pgm_read_byte((uint32_t)(track1) + segment)&CURVELEFT) skyx+= zspeed/200;
    if (pgm_read_byte((uint32_t)(track1) + segment)&CURVERIGHT) skyx-= zspeed/200;
    skyy=dynamichz-17;
    if (skyy < -8) skyy=-8; // leaves at least 1 byte to draw
    //skyy = 0;
    if (skyx>=TVX) skyx=0;
    if (skyx<0) skyx = TVX+skyx;
    //skyy =-1;
    drawSky(skyx,skyy);



    if ((pgm_read_byte((uint32_t)(track1) + segment+2)==0xff)) {
            segment = 0;
            if (best == 0 || best > int (millis()/100 - laptime)) best = int (millis()/100 - laptime);
            laptime = millis()/100;
    }
    drawRoad();
    updatecar();
    updateHUD();
    rlebitmap(10,10,lean0A);
    refresh();
    //delay(5);
    //refresh();
}


void Game::updateHUD(){
    //draw_line(0,HORIZON+1,TVX,HORIZON+1,BLACK);
    //set_cursor(TVXCENTER+20,0);
    set_cursor(1,1);
    font_bg = WHITE;
    font_color = BLACK;
    //println(1234567);
    //println(z_world);
    alphabitmap(TVX-18,TVY-20,rev,revalpha,0);
    overlaybitmap(TVX-11,TVY-9,&numbers[gear][0],0,8,8);

    uint8_t revptr;
    revptr = revs*17/355;
    if (revptr > 16 ) revptr = 16;
    if (revptr < 0 ) revptr = 0;

    draw_line(73,35,pgm_read_byte((uint32_t)(revcoords) + revptr*2),pgm_read_byte((uint32_t)(revcoords) + revptr*2+1),1);

    set_cursor(0,1);

    //print(0,8,dynamichz);
    /*
    if (hillstate==0) println("FLAT");
    if (hillstate==1) println("HILLC");
    if (hillstate==2) println("UP");
    if (hillstate==3) println("CREST");
    if (hillstate==4) println("DROP");
    if (hillstate==5) println("DROP2");
    if (hillstate==6) println("DOWN");
    if (hillstate==7) println("VALLEY");
    */
    println(segment);
    print(0,42,int (millis()/100-laptime));
    print(0,36,best);
    print(0,30,roadx);
    if (zspeed < 0) zspeed = 0;
    if (zspeed < 10) print(TVXCENTER+20+3*6,1,zspeed);
    else if (zspeed < 100) print(TVXCENTER+20+2*6,1,zspeed);
    if (zspeed > 100) print(TVXCENTER+20+1*6,1,zspeed);

    #ifdef DRAWFORCES
        draw_rect(TVXCENTER,10,TVXCENTER-(cgvector/2),5,1,0);
        draw_rect(TVXCENTER,5,TVXCENTER-(leanvector/2),1,1,0);
    #endif // DRAWFORCES

}

void Game::updatecar() {
    // draw car
    byte nothingpressed = true, leanstopped=true; leantick++;
    soundEvent = REVSND;
    //if (car_dir == UP) { overlaybitmap(carx,cary, ferrari_fwd, 0, 0, 0); set_pixel(carx+13,cary+15,1); set_pixel(carx+21,cary+15,1);}
    if (leantick > LEANTIMER+1) leantick = 0;
    if (lean == 0) alphabitmap(carx,cary, lean0A, alpha0, 0);
    if (lean == -1) alphabitmap(carx,cary, lean1A, alpha1, 0);
    if (lean == -2) alphabitmap(carx,cary, lean2A, alpha2, 0);
    if (lean == -3) alphabitmap(carx,cary, lean3A, alpha3, 0);
    if (lean == 1) alphabitmap(carx,cary, lean1A, alpha1, 1);
    if (lean == 2) alphabitmap(carx,cary, lean2A, alpha2, 1);
    if (lean == 3) alphabitmap(carx,cary, lean3A, alpha3, 1);

    pollButtons();

    //if (revs>350) revs -= 5;

    if (upBtn()) {
            if (gear <7 && !upHeld()) { gear++; revs >>= 1; }
    }

    if (downBtn() && !aBtn()) {
            if (gear > 0 && !downHeld()) { gear--; revs <<= 1; if (revs>350) revs=350;}
    }

    /** ACCELERATION **/
    if (aBtn()) {
            if (revs < (350-gear*40)) {
               revs+=64/(gear+1); // Hi torque
            } else {
               revs+=24/(gear+1); // Low torque
            }
            if (revs>350) revs = 350;
            if (revs<20) revs=20;
            nothingpressed = false;
    }

    /** BRAKING **/
    if (bBtn()) {
        zspeed-= BRAKESPEED;
        revs -= BRAKESPEED;
        if (revs<20) revs=20;
        if (zspeed <0) zspeed = 0;
        nothingpressed = false;
        if (zspeed>0 ) { fumeframe = 9; acceltick = 1;}
        }

    if (leftBtn()) {
            leanstopped = false;
            if (leantick>LEANTIMER) lean--;
            if (lean < -3) lean = -3;
            //leanvector -= LEANFORCE;
            if (zspeed>180 && acceltick==0) {
                //acceltick = ACCELTIME/4;
                //zspeed-=10; // decelerate due to hard cornering, was -5
            }
            nothingpressed = false;
            }
    if (rightBtn()) {
            leanstopped = false;
            if (leantick > LEANTIMER) lean++;
            if (lean > 3) lean = 3;
            //leanvector += LEANFORCE;
            if (zspeed>180 && acceltick==0) {
                //acceltick = ACCELTIME/4;
                //zspeed-=5; // decelerate due to hard cornering
            }
            nothingpressed = false;
    }


    /** ROAD EFFECT **/
    uint8_t speedeffect=6; //was 4
    if (zspeed < 10) speedeffect = 0;
    else if (zspeed < 201) speedeffect = 10; // was 7
    else if (zspeed < 401) speedeffect = 8; // was 5
    else if (zspeed < 550) speedeffect = 6; // was 5
    else if (zspeed < 650) speedeffect = 6; // was 5
    leanvector = -lean*speedeffect*LEANEFFECT;
    if (zspeed < 10) leanvector = 0;


    /** SIDEWAYS CONTROL **/
    if (cgvector + leanvector < 0 - FORCETOLERANCE) {
        if (lean == 0) roadx-=3;
        else {
                roadx-=1;
                if (cgvector > -leanvector) soundEvent = SLIPSND;
        }
    } else if (cgvector + leanvector > 0 + FORCETOLERANCE) {
        if (lean == 0) roadx+=3;
        else {
                roadx+=1;
                if (-cgvector > leanvector) soundEvent = SLIPSND;
        }
    }

    if (acceltick>0) {
                /*if (fumeframe == 0) overlaybitmap(carx,cary, bike2, 0, 0, 0);
                if (fumeframe == 3) overlaybitmap(carx,cary, bike3, 0, 0, 0);
                if (fumeframe == 9) overlaybitmap(carx,cary, bike4, 0, 0, 0);
                fumeframe++;
                if (fumeframe == 12) fumeframe=0;*/

    }

    /** DECELERATION **/
    if (!aBtn()) {
            acceltick = 0;
            fumeframe=0;
            revs-=64/(gear+1);
            if (revs < MINREVS ) {
                    soundEvent = IDLESND;
            }
    }

    /** SET BIKE SPEED **/


    if (gear) {
        if (aBtn()) zspeed = revs*2*gear/7;
        else zspeed -= DRAGSPEED;
    } else zspeed -= DRAGSPEED;

    if (zspeed<0) zspeed = 0;
    if (zspeed>700) zspeed = 700;

    /** CRASH / RUMBLE DETECT **/

    //scalebitmap(carx-lean*3+3,cary+17,puff1,puff1alpha,0);
    if (roadx>32 || roadx<-32) {
            alphabitmap(carx-lean*3+4,cary+17,puff1,puff1alpha,1);
            revs -= RUMBLEDECELERATE;
            if (revs < MINREVS) revs = MINREVS;
            //zspeed -= RUMBLEDECELERATE;
            if (zspeed>0) soundEvent = OFFROADSND;
            else soundEvent = IDLESND;
    }

    /** SET SOUND **/
    switch (soundEvent) {
        case IDLESND:
            revs = MINREVS-10+int(random(9));
            setPitch((revs));
            break;
        case REVSND:
            setPitch((revs));
            break;
        case SLIPSND:
            setPitch(500+random(150));
            break;
        case OFFROADSND:
            setPitch(random(70));
            break;
    }


    if (leanstopped && leantick) {
        if (lean > 0) {
                lean--; // face forward unless left or right is pressed
                leanvector += LEANRETURN;
        }
        if (lean < 0) {
                lean++; // face forward unless left or right is pressed
                leanvector += LEANRETURN;
        }
        if (lean == 0) leanvector = 0;
    }


    z_car += zspeed>>ZCARSPEED ; // was /10
    #ifdef DEMO
    zspeed = 240 ;
    #endif
    z_world += zspeed/WORLDSPEED;
    skytick+= zspeed/100;
    if (z_car>100) {
            z_car=0;
    }
    if (zspeed>10) {
            if (wheeltick == 1) {
                    if (lean == 0) alphabitmap(carx,cary, lean0B, alpha0,0);
                    if (lean == -1) alphabitmap(carx,cary, lean1B, alpha1, 0);
                    if (lean == -2) alphabitmap(carx,cary, lean2B, alpha2, 0);
                    if (lean == -3) alphabitmap(carx,cary, lean3B, alpha3, 0);
                    if (lean == 1) alphabitmap(carx,cary, lean1B, alpha1, 1);
                    if (lean == 2) alphabitmap(carx,cary, lean2B, alpha2, 1);
                    if (lean == 3) alphabitmap(carx,cary, lean3B, alpha3, 1);
                    wheeltick = 0;
            } else { wheeltick++; }

    }


}



//=========================================================================
// Draw sky
//=========================================================================

void Game::drawSky(int x,int y){
     overlaybitmap(x,y,sky,0,0,0);
}

//=========================================================================
// BUILD ROAD GEOMETRY
//=========================================================================

void Game::gameSetup() {
    int k=0;

    lastRoad== pgm_read_byte((uint32_t)(track1));
    // table of spots
    for (int i=0; i < NUMSPOTS; i+=2) {
        xspots[i] = 22;   // distributed PALMS !!
        xspots[i+1] = -22;//*-8;   // distributed PALMS !!
        zspots[i] = (z_car)+i*(1000/NUMSPOTS);
        zspots[i+1] = (z_car)+i*(1000/NUMSPOTS);
    }
    k=0;

    // table for cars
    for (int i=0; i < NUMCARS; i++) {
        xcars[i] = ROADW;   // distributed cars
        zcars[i] = (i+1)*(1000);
    }
    zcars[0] = 400;
    zcars[1] = 800;
    zcars[2] = 1000;
    zcars[3] = 1300;
    xcars[0] = 25;
    xcars[1] = -35;
    xcars[2] = -5;
    xcars[3] = -35;

    // lookup table for z and road sides
    float ztemp, yxtra=1;
    for (int y=TVY-1; y>=dynamichz-XTRA ;y--) {
        int zy=dynamichz;
        if (y - dynamichz > 0) {zy = Y_CAMERA / (y-dynamichz);
        } else { yxtra *= 1.5; zy=Y_CAMERA*yxtra; }
        zlookup[k] = zy;
        ztemp = (((float)ROADW/(float)zy)*(float)Z_MULT);
        xlookup[k] = 2*ztemp;
        k++;
    }
    // road curvature table

    for (k=0; k<YTABS+5;k++) {
        curvature[k] = k*k/500; // was 300
    }

}


//=========================================================================
// DRAW ROAD
//=========================================================================

void Game::drawRoad() {
    unsigned int ztemp, easeout = true;
    int yTransition = dynamichz,curvacceltop=0,curvaccelbot=0,curvoffset=0;
    int q_step,q_pointer;
    char track, trackplus1, trackplus2;

    // determine segment transition points
    ztemp = (z_world >> SEGSLOWDOWN) & (SEGLENGTH-1);
    ztemp = SEGLENGTH - ztemp; // segment edges otherwise come too late

    //check if transition points are within visible range
    if (!segvisible && ztemp > 900) { //was 600 and 800
        segvisible = true;
        yTransition = dynamichz;
        nextz = z_world+ZTICKER*3;
        nexto = z_world+ZTICKER;
    }

    // determine hill state
    // seg  seg+1   seg+2   equals
    // flat flat    flat    FLATLAND
    // flat flat    up      HILLCOMING
    // flat up      any     UPSLOPE
    // up   up      any     UPSLOPE
    // up   flat/down any   CREST
    // flat flat    down    DROPCOMING
    // flat down    any     DROPCOMING2
    // down flat    any     DOWNSLOPE
    // down up      any     VALLEY

    hillstate=FLATLAND;
    track = pgm_read_byte((uint32_t)(track1) + segment);
    trackplus1 = pgm_read_byte((uint32_t)(track1) + segment+1);
    trackplus2 = pgm_read_byte((uint32_t)(track1) + segment+2);
    if (!(track&UPHILL) && !(track&DOWNHILL)) {
        // current segment FLAT
        if (!(trackplus1&UPHILL) && !(trackplus1&DOWNHILL)) {
            // next segment also FLAT
            hillstate = FLATLAND; // unless changed, is FLAT
            if (trackplus2&UPHILL) hillstate = HILLCOMING;
            if (trackplus2&DOWNHILL) hillstate = DROPCOMING;
        }

        if (trackplus1&UPHILL) hillstate = UPSLOPE;
        if (trackplus1&DOWNHILL) hillstate = DROPCOMING2;
    } else {
        // current segment is NOT FLAT
        if (track&DOWNHILL) {
                hillstate = DOWNSLOPE; // unless it goes straight to upslope
                if (trackplus1&UPHILL) hillstate = VALLEY;
            } else {
                hillstate = CREST; // crest, unless
                if (trackplus1&UPHILL) hillstate = UPSLOPE;
            }
    }


    switch (hillstate) {
    case HILLCOMING:
        nexthz = HORIZON - 3; // was -3
        nexthill =250; //was 250
        break;
    case FLATLAND:
        nexthz = HORIZON; // was 0
        nexthill=256; // was 256
        break;
    case UPSLOPE:
        nexthz = HORIZON - 11; // was - 11
        nexthill=210; // was 210
        break;
    case CREST:
        nexthz = HORIZON + 6; // was +6
        nexthill=512; // was 512
        break;
    case DROPCOMING:
        nexthz = HORIZON + 3; // was +3
        nexthill=400; // was 400
        break;
    case DROPCOMING2:
        nexthz = HORIZON + 2; // was +2
        nexthill=512; // was 512
        break;
    case DOWNSLOPE:
        nexthz = HORIZON ; // was 0
        nexthill=300; // was 300
        break;
    case VALLEY:
        nexthz = HORIZON -4 ; // was -4
        nexthill=300; // was 300
        break;
    default:
        nexthz = HORIZON; // was 0
        nexthill=256; // was 256
    }

    if (nexthz != dynamichz && z_world > nextz) {
        if (dynamichz < nexthz) dynamichz++;
        if (dynamichz > nexthz) dynamichz--;
        nextz = z_world+ZTICKER*2;
    }

    if (nexthill != hilloffset && z_world > nexto) {
        if (hilloffset < nexthill) {
            hilloffset+=5;
            if (hilloffset > nexthill) hilloffset = nexthill;
            } else if (hilloffset > nexthill) {
            hilloffset-=5;
            if (hilloffset < nexthill) hilloffset = nexthill;
            }

        nexto = z_world+ZTICKER;
    }


    if (segvisible) { // was 512
        yTransition = dynamichz+1+(SEGLENGTH-ztemp)/8;
        q_step = YTABS;
        q_step *= 256; // some weird compiler bug
        q_step = q_step/(yTransition-dynamichz);
        // reset transition
        if (yTransition >= TVY) {
            segment++;
            segvisible = false;
            yTransition = dynamichz;
        }
    }


    // set up curve counter
    if (track&CURVERIGHT) {curvaccelbot=1;}
    if (track&CURVELEFT) {curvaccelbot=-1;}
    if (trackplus1&CURVERIGHT) {curvacceltop=1;}
    if (trackplus1&CURVELEFT) {curvacceltop=-1;}

    if (curvacceltop == curvaccelbot) easeout = false; // disable easeout

    // effect of road on car
    #ifdef ROADEFFECT

    cgvector = curvaccelbot*zspeed/100*ROADEFFECT+random(FORCETOLERANCE);
    if (zspeed < 100) cgvector = 0;
    #endif




    // draw ground
    #ifdef GROUND
    draw_rect(0,dynamichz-1,TVX,dynamichz+10,-1,1);

    for (int screeny=TVY-1; screeny > dynamichz; screeny--) {
        int a,a2,b,b2,c,d,dx,zy,rumblew,poffset,temp;
        int q = TVY-screeny; // define index for lookup

        poffset = roadx*xlookup[q]/DXDIV;
        if (screeny < yTransition && easeout) {
            // road ABOVE transition
            q_pointer = ((yTransition-screeny+1)*q_step)/256;
            if(q_pointer>YTABS) q_pointer=YTABS;
            curvoffset += curvacceltop*curvature[q_pointer];
            q_pointer -= 6;
            if (q_pointer<0) q_pointer =0;
        } else {
            // road BELOW transition
            curvcount = yTransition - dynamichz;
            if (easeout) q_pointer = q-curvcount;
            else q_pointer=q;
            if (q_pointer<0) q_pointer = 0;
            curvoffset += curvaccelbot*curvature[q_pointer];
        }

        q=q*hilloffset/256;

        a = TVXCENTER - xlookup[q] + poffset + curvoffset;

        a2 = TVXCENTER - (xlookup[q]>>1) + poffset + curvoffset;
        b = TVXCENTER + xlookup[q] + poffset + curvoffset;
        b2 = TVXCENTER + (xlookup[q]>>1) + poffset + curvoffset;


        temp = a-(xlookup[q]>>2)-2;
        if (temp < 0 || temp > TVX ) leftside[screeny] = 0;
        else leftside[screeny] = temp;
        temp = b + (xlookup[q]>>2)+2;
        if (temp > TVX || temp < 0) rightside[screeny] = TVX;
        else rightside [screeny] = temp;


        // Clean the road
        if (a>=0 && b <=TVX) {
                if (screeny > cary + 14 && screeny < cary + 19 ) {
                    draw_line (a,screeny,carx-14,screeny,0);
                    draw_line (carx+60,screeny,b,screeny,0);
                } else {
                draw_line (a,screeny,b,screeny,0);
                }
        } else {
                draw_line (0,screeny,TVX,screeny,0);
        }

         // check if a or b are out of bounds and draw road sides

            if (a>=0) draw_line (0,screeny,a,screeny,HATCH);
            if (a>TVX) draw_line (0,screeny,TVX,screeny,HATCH);
            if (b<=TVX) draw_line (b,screeny,TVX,screeny,HATCH);
            if (b<0) draw_line (0,screeny,TVX,screeny,HATCH);

        // draw rumbles
        zy = zlookup[q];
        //if (zy<1) break; // invalid line
        zy += z_car;
        //byte col = colorlookup[zy&63]; // z_car modulus 64 x%64 == x&63 to have only a 64 byte rumble buffer
        byte col = pgm_read_byte((uint32_t)(colorlookup) + (zy&63));
        dx = xlookup[q]; // find dx width for this Z depth
        rumblew = dx/RUMBLEW;
        //a = a + rumblew; // rumble left edge
        //c = b - rumblew; // right side rumble right edge
        c = b; // right side rumble right edge
        d = c - rumblew; // right side rumble left edge
        b = a + rumblew; // rumble right edge

        if (a<0) a=0; // rumble left edge out of screen check
        if (b>0) draw_line (a,screeny,b,screeny,col); // if rumble is visible, draw
        if (c>TVX) c=TVX; // check if roadside is out of screen
        if (d<TVX) draw_line (c,screeny,d,screeny,col); // if visible, draw
        /*
        a = a2 + 2.5 *rumblew; // rumble left edge
        c = b2 - 2.5 *rumblew; // right side rumble right edge
        d = c - rumblew; // right side rumble left edge
        b = a + rumblew; // rumble right edge
        if (a<0) a=0; // rumble left edge out of screen check
        if (b>0) draw_line (a,screeny,b,screeny,col); // if rumble is visible, draw
        if (c>TVX) c=TVX; // check if roadside is out of screen
        if (d<TVX) draw_line (c,screeny,d,screeny,col); // if visible, draw
        */
    }
    #endif

    #ifdef SEGMENTS
        if (yTransition > dynamichz && yTransition <TVY ) {
        draw_line(0,yTransition,TVX,yTransition,0);
        }
    #endif // SEGMENTS

    #ifdef ROADSIDE
    //draw spots
    for (int i=0; i < NUMSPOTS; i++) {
        int y,x,z,coffset;
        uint8_t h,w,a,b;
        int top1, top2;
        x = xspots[i];
        z = zspots[i];

        if (z < z_world+20) {
            zspots[i] = z_world+ 1000; // push spot to dynamichz
            x = xspots[i];
            z = zspots[i];
        }

        y = (Y_CAMERA / (z-z_world)) + dynamichz;


        coffset = (rightside[y] - leftside[y]);
        h = xlookup[TVY-y] >> 1;
        w = h >> 1;
        a = 0; b = TVX; top1= top2=0;
        if (coffset > 3)  {
            if (leftside[y] > 0){
                a = leftside[y]-w;
                draw_rect(leftside[y]-w,y,leftside[y]-1,y-h,-1,0);
                draw_line(leftside[y],y,leftside[y],y-h,1);

            }
            if (rightside[y] < TVX){
                b = rightside[y]+w;
                draw_line(rightside[y],y,rightside[y],y-h,1);
                draw_rect(rightside[y]+1,y,rightside[y]+w,y-h,-1,0);
            }

            top1=y-h-w; top2=y-h;
                if (top1 < 0) top1=0;
                if (top2 > 2 && z > z_world+20) draw_rect(a,top1,b,top2,-1,0);

        }

    }
    #endif // ROADSIDE


    #ifdef TRAFFIC
            // draw cars

   for (int i=NUMCARS; i >-1; i--) {
        int y,x,z,coffset;
        x = xcars[i];
        zcars[i] += OTHERCARSSPEED;
        z = zcars[i];

        if (z < z_world-30) {
            zcars[i] = z_world+ 1000*NUMCARS; // push spot to dynamichz
            x = xcars[i];
            z = zcars[i];
        }
        if(z-z_world==0) z++;
        y = (Y_CAMERA / (z-z_world)) + dynamichz;
        #ifdef CURVED
            coffset = curvature[TVY-y-5];
        #else
            coffset=0;
        #endif
            if (z-z_world > 2000) {
            } else {
            if (z-z_world > 1500) {
                    #ifdef CURVED
                    coffset = curvature[TVY-y-3];
                    #else
                    coffset=0;
                    #endif
                    x = TVXCENTER+(roadx*dxlookup[TVY-y-3]/DXDIV)+x*dxlookup[TVY-y-3]/DXDIV+coffset;
                    if(x>0 && x < TVX-10)draw_rect(x,y+2,2,1,0,0);
            } else {
            if (z-z_world > 1100) {
                    #ifdef CURVED
                    coffset = curvature[TVY-y-3];
                    #else
                    coffset=0;
                    #endif
                    x = TVXCENTER+(roadx*dxlookup[TVY-y-3]/DXDIV)+x*dxlookup[TVY-y-3]/DXDIV+coffset;
                    if(x>0 && x < TVX-10)draw_rect(x,y+1,3,2,0,1);
            } else {
            if (z-z_world > 500) {
                    #ifdef CURVED
                    coffset = curvature[TVY-y-5];
                    #else
                    coffset=0;
                    #endif
                     x = TVXCENTER+(roadx*dxlookup[TVY-y-5]/DXDIV)+x*dxlookup[TVY-y-5]/DXDIV+coffset;
                    if(x>0 && x < TVX-10)alphabitmap(x-1,y,porsche_tiny,porsche_tiny_alpha);
            } else {
            if (z-z_world > 200) {
                    #ifdef CURVED
                    coffset = curvature[TVY-y-8];
                    #else
                    coffset=0;
                    #endif
                    x = TVXCENTER+(roadx*dxlookup[TVY-y-8]/DXDIV)+x*dxlookup[TVY-y-8]/DXDIV+coffset;
                    if(x>0 && x < TVX-10)alphabitmap(x,y,porsche_small,porsche_small_alpha);
            } else {
            if (z-z_world > 30) {
                    #ifdef CURVED
                    coffset = curvature[TVY-y-9];
                    #else
                    coffset=0;
                    #endif
                    x = TVXCENTER+(roadx*dxlookup[TVY-y-9]/DXDIV)+x*dxlookup[TVY-y-9]/DXDIV+coffset;
                    if(x>0 && y < TVY-21 && x< TVX-10) alphabitmap(x,y,porsche_med,porsche_med_alpha);
            }}}}}}}
    #endif // TRAFFIC
}
