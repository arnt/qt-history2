
/* Copyright (c) Mark J. Kilgard, 1996. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <stdlib.h>
#include <stdio.h>
#include <Carbon.h>
#include <agl.h>

static char *ClassOf(int c);
static char *Format(int n, int w);

int
main()
{
    GLint bufferSize, level, renderType, doubleBuffer, stereo, 
	auxBuffers, redSize, greenSize, blueSize, alphaSize, depthSize,
	stencilSize, acRedSize, acGreenSize, acBlueSize, acAlphaSize,
	renderID, major, minor;

    aglGetVersion(&major, &minor);
    printf("using AGL version: %d.%d\n\n", (int)major, (int)minor);
    printf("   visual     bf lv rg d st  r  g  b a   ax dp st accum buffs\n");
    printf(" id dep cl    sz l  ci b ro sz sz sz sz  bf th cl  r  g  b  a\n");
    printf("-------------------------------------------------------------\n");

    for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) {
	for(AGLRendererInfo nfs = aglQueryRendererInfo(&g, 1); nfs; nfs = aglNextRendererInfo(nfs)) {
		aglDescribeRenderer( nfs, AGL_RENDERER_ID, &renderID );
		aglDescribeRenderer( nfs, AGL_BUFFER_SIZE, &bufferSize);
		aglDescribeRenderer( nfs, AGL_LEVEL, &level);
		aglDescribeRenderer( nfs, AGL_RGBA, &renderType);
		aglDescribeRenderer( nfs, AGL_DOUBLEBUFFER, &doubleBuffer);
		aglDescribeRenderer( nfs, AGL_STEREO, &stereo);
		aglDescribeRenderer( nfs, AGL_AUX_BUFFERS, &auxBuffers);
		aglDescribeRenderer( nfs, AGL_RED_SIZE, &redSize);
		aglDescribeRenderer( nfs, AGL_GREEN_SIZE, &greenSize);
		aglDescribeRenderer( nfs, AGL_BLUE_SIZE, &blueSize);
		aglDescribeRenderer( nfs, AGL_ALPHA_SIZE, &alphaSize);
		aglDescribeRenderer( nfs, AGL_DEPTH_SIZE, &depthSize);
		aglDescribeRenderer( nfs, AGL_STENCIL_SIZE, &stencilSize);
		aglDescribeRenderer( nfs, AGL_ACCUM_RED_SIZE, &acRedSize);
		aglDescribeRenderer( nfs, AGL_ACCUM_GREEN_SIZE, &acGreenSize);
		aglDescribeRenderer( nfs, AGL_ACCUM_BLUE_SIZE, &acBlueSize);
		aglDescribeRenderer( nfs, AGL_ACCUM_ALPHA_SIZE, &acAlphaSize);

		printf("0x%x %2d %s", renderID, 0xDE, "foo..");
		printf("    %2s %2s %1s  %1s  %1s ",
		       Format(bufferSize, 2), Format(level, 2),
		       renderType ? "r" : "c",
		       doubleBuffer ? "y" : ".", 
		       stereo ? "y" : ".");
		printf("%2s %2s %2s %2s ",
		       Format(redSize, 2), Format(greenSize, 2),
		       Format(blueSize, 2), Format(alphaSize, 2));
		printf("%2s %2s %2s %2s %2s %2s %2s",
		       Format(auxBuffers, 2), Format(depthSize, 2), Format(stencilSize, 2),
		       Format(acRedSize, 2), Format(acGreenSize, 2),
		       Format(acBlueSize, 2), Format(acAlphaSize, 2));
		printf("\n");
	}
    }
}

static char *
ClassOf(int)
{
    return "fo";
}

static char *
Format(int n, int w)
{
  static char buffer[256];
  static unsigned int bufptr;
  char *buf;

  if (bufptr >= sizeof(buffer) - w)
    bufptr = 0;
  buf = buffer + bufptr;
  if (n == 0)
    sprintf(buf, "%*s", w, ".");
  else
    sprintf(buf, "%*d", w, n);
  bufptr += w + 1;
  return buf;
}
