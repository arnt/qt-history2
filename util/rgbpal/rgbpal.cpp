/****************************************************************************
** $Id: //depot/qt/main/util/rgbpal/rgbpal.cpp#3 $
**
** Generates code to create a Windows 8 bit RGB palette. This palette
** is required by Qt for OpenGL RGBA rendering.
**
** This code is based on win8map.c on Microsoft Developer Network CD.
**
** Created : 920604
**
** Copyright (C) 1997 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/


#include <stdio.h>
#include <math.h>

#define DEFAULT_GAMMA 1.4F

#define MAX_PAL_ERROR (3*256*256L)

struct colorentry {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

struct rampentry {
    struct colorentry color;
    long defaultindex;
    unsigned char flags;
};

struct defaultentry {
    struct colorentry color;
    long rampindex;
    unsigned char flags;
};

/* values for flags */
#define EXACTMATCH	0x01
#define CHANGED		0x02	    /* one of the default entries is close */

/*
 * These arrays hold bit arrays with a gamma of 1.0
 * used to convert n bit values to 8-bit values
 */

unsigned char threeto8[8] = {
    0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
};

unsigned char twoto8[4] = {
    0, 0x55, 0xaa, 0xff
};

unsigned char oneto8[2] = {
    0, 255
};

struct defaultentry defaultpal[20] = {
    { 0,   0,	0 },
    { 0x80,0,	0 },
    { 0,   0x80,0 },
    { 0x80,0x80,0 },
    { 0,   0,	0x80 },
    { 0x80,0,	0x80 },
    { 0,   0x80,0x80 },
    { 0xC0,0xC0,0xC0 },

    { 192, 220, 192 },
    { 166, 202, 240 },
    { 255, 251, 240 },
    { 160, 160, 164 },

    { 0x80,0x80,0x80 },
    { 0xFF,0,	0 },
    { 0,   0xFF,0 },
    { 0xFF,0xFF,0 },
    { 0,   0,	0xFF },
    { 0xFF,0,	0xFF },
    { 0,   0xFF,0xFF },
    { 0xFF,0xFF,0xFF }
};

struct rampentry rampmap[256];

void
gammacorrect(double gamma)
{
    int i;
    unsigned char v, nv;
    double dv;

    for (i=0; i<8; i++) {
	v = threeto8[i];
	dv = (255.0 * pow(v/255.0, 1.0/gamma)) + 0.5;
	nv = (unsigned char)dv;
	threeto8[i] = nv;
    }
    for (i=0; i<4; i++) {
	v = twoto8[i];
	dv = (255.0 * pow(v/255.0, 1.0/gamma)) + 0.5;
	nv = (unsigned char)dv;
	twoto8[i] = nv;
    }
    printf("\n");
}

main(int argc, char *argv[])
{
    long i, j, error, min_error;
    long error_index, delta;
    double gamma;
    struct colorentry *pc;

    if (argc == 2)
	gamma = atof(argv[1]);
    else
	gamma = DEFAULT_GAMMA;

    gammacorrect(gamma);

    /* First create a 256 entry RGB color cube */

    for (i = 0; i < 256; i++) {
	/* BGR: 2:3:3 */
	rampmap[i].color.red = threeto8[(i&7)];
	rampmap[i].color.green = threeto8[((i>>3)&7)];
	rampmap[i].color.blue = twoto8[(i>>6)&3];
    }

    /* Go through the default palette and find exact matches */
    for (i=0; i<20; i++) {
	for(j=0; j<256; j++) {
	    if ( (defaultpal[i].color.red == rampmap[j].color.red) &&
		 (defaultpal[i].color.green == rampmap[j].color.green) &&
		 (defaultpal[i].color.blue == rampmap[j].color.blue)) {

		rampmap[j].flags = EXACTMATCH;
		rampmap[j].defaultindex = i;
		defaultpal[i].rampindex = j;
		defaultpal[i].flags = EXACTMATCH;
		break;
	    }
	}
    }

    /* Now find close matches */
    for (i=0; i<20; i++) {
	if (defaultpal[i].flags == EXACTMATCH)
	    continue;	      /* skip entries w/ exact matches */
	min_error = MAX_PAL_ERROR;

	/* Loop through RGB ramp and calculate least square error */
	/* if an entry has already been used, skip it */
	for(j=0; j<256; j++) {
	    if (rampmap[j].flags != 0)	    /* Already used */
		continue;

	    delta = defaultpal[i].color.red - rampmap[j].color.red;
	    error = (delta * delta);
	    delta = defaultpal[i].color.green - rampmap[j].color.green;
	    error += (delta * delta);
	    delta = defaultpal[i].color.blue - rampmap[j].color.blue;
	    error += (delta * delta);
	    if (error < min_error) {	    /* New minimum? */
		error_index = j;
		min_error = error;
	    }
	}
	defaultpal[i].rampindex = error_index;
	rampmap[error_index].flags = CHANGED;
	rampmap[error_index].defaultindex = i;
    }

    /* Print out logical palette code */

    printf("    static struct {\n");
    printf("\tWORD\t     palVersion;\n");
    printf("\tWORD\t     palNumEntries;\n");
    printf("\tPALETTEENTRY palPalEntries[256];\n");
    printf("    } rgb8palette = {\n");
    printf("\t0x300,\n");
    printf("\t256,");

    for (i=0; i<256; i++) {
	if ( i % 4 == 0 )
	    printf( "\n\t" );
	else
	    printf( " " );
	if (rampmap[i].flags == 0)
	    pc = &rampmap[i].color;
	else
	    pc = &defaultpal[rampmap[i].defaultindex].color;
	printf("%3d,%3d,%3d,  0,", pc->red, pc->green, pc->blue );
    }

    printf("};\n");

    return 0;
}
