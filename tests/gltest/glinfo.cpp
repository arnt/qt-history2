/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

#include "glinfo.h"

#include "qstring.h"

#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct visual_attribs {
    /* X visual attribs */
    int id;
    int klass;
    int depth;
    int redMask, greenMask, blueMask;
    int colormapSize;
    int bitsPerRGB;
 
    /* GL visual attribs */
    int supportsGL;
    int transparent;
    int bufferSize;
    int level;
    int rgba;
    int doubleBuffer;
    int stereo;
    int auxBuffers;
    int redSize, greenSize, blueSize, alphaSize;
    int depthSize;
    int stencilSize;
    int accumRedSize, accumGreenSize, accumBlueSize, accumAlphaSize;
    int numSamples, numMultisample;
    int visualCaveat;
};
 
#ifndef GLX_NONE_EXT
#define GLX_NONE_EXT  0x8000
#endif

void mesa_hack(Display *dpy, int scrnum)
{
    static int attribs[] = {
        GLX_RGBA,
	GLX_RED_SIZE, 1,
	GLX_GREEN_SIZE, 1,
	GLX_BLUE_SIZE, 1,
	GLX_DEPTH_SIZE, 1,
	GLX_STENCIL_SIZE, 1,
	GLX_ACCUM_RED_SIZE, 1,
	GLX_ACCUM_GREEN_SIZE, 1,
	GLX_ACCUM_BLUE_SIZE, 1,
	GLX_ACCUM_ALPHA_SIZE, 1,
	GLX_DOUBLEBUFFER,
	None
    };
    XVisualInfo *visinfo;
 
    visinfo = glXChooseVisual(dpy, scrnum, attribs);
    if (visinfo)
        XFree(visinfo);
}

void print_extension_list(const char *ext)
{
    const char *indentString = "    ";
    const int indent = 4;
    const int max = 79;
    int width, i, j;
 
    if (!ext || !ext[0])
        return;
 
    width = indent;
    printf(indentString);
    i = j = 0;
    while (1) {
        if (ext[j] == ' ' || ext[j] == 0) {
	    /* found end of an extension name */
	    const int len = j - i;
	    if (width + len > max) {
                /* start a new line */
	        printf("\n");
		width = indent;
		printf(indentString);
	    }
	    /* print the extension name between ext[i] and ext[j] */
	    while (i < j) {
	        printf("%c", ext[i]);
		i++;
	    }
	    /* either we're all done, or we'll continue with next extension */
	    width += len + 1;
	    if (ext[j] == 0) {
	        break;
	    }
	    else {
	        i++;
		j++;
		if (ext[j] == 0)
		    break; 
		printf(", ");
		width += 2;
	    }
	}   
	j++;
    }
    printf("\n");
}

void GLInfo::print_screen_info(Display *dpy, int scrnum)
{
   Window win;
   int attribSingle[] = {
      GLX_RGBA,
      GLX_RED_SIZE, 1,
      GLX_GREEN_SIZE, 1,
      GLX_BLUE_SIZE, 1,
      None };
   int attribDouble[] = {
      GLX_RGBA,
      GLX_RED_SIZE, 1,
      GLX_GREEN_SIZE, 1,
      GLX_BLUE_SIZE, 1,
      GLX_DOUBLEBUFFER,
      None };
 
   XSetWindowAttributes attr;
   unsigned long mask;
   Window root;
   GLXContext ctx;
   XVisualInfo *visinfo;
   int width = 100, height = 100;
 
   root = RootWindow(dpy, scrnum);
 
   visinfo = glXChooseVisual(dpy, scrnum, attribSingle);
   if (!visinfo) {
      visinfo = glXChooseVisual(dpy, scrnum, attribDouble);
      if (!visinfo) {
         fprintf(stderr, "Error: couldn't find RGB GLX visual\n");
         return;
      }
   }
 
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask;
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
   win = XCreateWindow(dpy, root, 0, 0, width, height,
                       0, visinfo->depth, InputOutput,
                       visinfo->visual, mask, &attr);
 
   ctx = glXCreateContext( dpy, visinfo, NULL, GL_FALSE );
   if (!ctx) {
      fprintf(stderr, "Error: glXCreateContext failed\n");
      XDestroyWindow(dpy, win);
      return;
   }
   
   if (glXMakeCurrent(dpy, win, ctx)) {
      const char *serverVendor = glXQueryServerString(dpy, scrnum, GLX_VENDOR);
      const char *serverVersion = glXQueryServerString(dpy, scrnum, GLX_VERSION);
      const char *serverExtensions = glXQueryServerString(dpy, scrnum, GLX_EXTENSIONS);
      const char *clientVendor = glXGetClientString(dpy, GLX_VENDOR);
      const char *clientVersion = glXGetClientString(dpy, GLX_VERSION);
      const char *clientExtensions = glXGetClientString(dpy, GLX_EXTENSIONS);
      const char *glxExtensions = glXQueryExtensionsString(dpy, scrnum);
      const char *glVendor = (const char *) glGetString(GL_VENDOR);
      const char *glRenderer = (const char *) glGetString(GL_RENDERER);
      const char *glVersion = (const char *) glGetString(GL_VERSION);
      const char *glExtensions = (const char *) glGetString(GL_EXTENSIONS);
      char *displayName = NULL;
      char *colon = NULL, *period = NULL;
      //const char *gluVersion = (const char *) gluGetString(GLU_VERSION);
      //const char *gluExtensions = (const char *) gluGetString(GLU_EXTENSIONS);
      /* Strip the screen number from the display name, if present. */ 
      /*if (!(displayName = malloc(strlen(DisplayString(dpy)) + 1))) {
          fprintf(stderr, "Error: malloc() failed\n");
	  exit(1);
      }
      strcpy(displayName, DisplayString(dpy));
      colon = strrchr(displayName, ':');
      if (colon) {
         period = strchr(colon, '.');
         if (period)
            *period = '\0';
      }
      printf("display: %s  screen: %d\n", displayName, scrnum);
      free(displayName);*/
      infotext->sprintf("direct rendering: %s\n", glXIsDirect(dpy, ctx) ? "Yes" : "No");
      printf("direct rendering: %s\n", glXIsDirect(dpy, ctx) ? "Yes" : "No");
      printf("server glx vendor string: %s\n", serverVendor);
      printf("server glx version string: %s\n", serverVersion);
      printf("server glx extensions:\n");
      print_extension_list(serverExtensions);
      printf("client glx vendor string: %s\n", clientVendor);
      printf("client glx version string: %s\n", clientVersion);
      printf("client glx extensions:\n");
      print_extension_list(clientExtensions);
      printf("GLX extensions:\n");
      print_extension_list(glxExtensions);
      printf("OpenGL vendor string: %s\n", glVendor);
      printf("OpenGL renderer string: %s\n", glRenderer);
      printf("OpenGL version string: %s\n", glVersion);
      printf("OpenGL extensions:\n");
      print_extension_list(glExtensions);
      //printf("glu version: %s\n", gluVersion);
      //printf("glu extensions:\n");
      //print_extension_list(gluExtensions);
   }
   else {
      fprintf(stderr, "Error: glXMakeCurrent failed\n");
   }
 
      glXDestroyContext(dpy, ctx);
   XDestroyWindow(dpy, win);
};

const char * visual_class_name(int cls)
{
    switch (cls) {
    case StaticColor:
        return "StaticColor";
    case PseudoColor:
        return "PseudoColor";
    case StaticGray:
        return "StaticGray";
    case GrayScale:
        return "GrayScale";
    case TrueColor:
        return "TrueColor";
    case DirectColor:
        return "DirectColor";
    default:
        return "";
   }
}

void print_visual_attribs_long_header(void)
{
    printf("Vis  Vis   Visual Trans  buff lev render DB ste  r   g   b   a  aux dep ste  accum buffers  MS   MS\n");
    printf(" ID Depth   Type  parent size el   type     reo sz  sz  sz  sz  buf th  ncl  r   g   b   a  num bufs\n");
    printf("----------------------------------------------------------------------------------------------------\n");
}
 
 
void print_visual_attribs_long(const struct visual_attribs *attribs)
{
    printf("0x%2x %2d %-11s %2d     %2d %2d  %4s %3d %3d %3d %3d %3d %3d",
	   attribs->id,
	   attribs->depth,
	   visual_class_name(attribs->klass),
	   attribs->transparent,
	   attribs->bufferSize,
	   attribs->level,
	   attribs->rgba ? "rgba" : "ci  ",
	   attribs->doubleBuffer,
	   attribs->stereo,
	   attribs->redSize, attribs->greenSize,
	   attribs->blueSize, attribs->alphaSize
	   );
    
    printf(" %3d %4d %2d %3d %3d %3d %3d  %2d  %2d\n",
	   attribs->auxBuffers,
	   attribs->depthSize,
	   attribs->stencilSize,
	   attribs->accumRedSize, attribs->accumGreenSize,
	   attribs->accumBlueSize, attribs->accumAlphaSize,
	   attribs->numSamples, attribs->numMultisample
	   );
}

void get_visual_attribs(Display *dpy, XVisualInfo *vInfo,
			struct visual_attribs *attribs)
{
    const char *ext = glXQueryExtensionsString(dpy, vInfo->screen);
 
    memset(attribs, 0, sizeof(struct visual_attribs));
 
    attribs->id = vInfo->visualid;
#if defined(__cplusplus) || defined(c_plusplus)
    attribs->klass = vInfo->c_class;
#else
    attribs->klass = vInfo->class;
#endif
    attribs->depth = vInfo->depth;
    attribs->redMask = vInfo->red_mask;
    attribs->greenMask = vInfo->green_mask;
    attribs->blueMask = vInfo->blue_mask;
    attribs->colormapSize = vInfo->colormap_size;
    attribs->bitsPerRGB = vInfo->bits_per_rgb;
 
    if (glXGetConfig(dpy, vInfo, GLX_USE_GL, &attribs->supportsGL) != 0)
        return;
    glXGetConfig(dpy, vInfo, GLX_BUFFER_SIZE, &attribs->bufferSize);
    glXGetConfig(dpy, vInfo, GLX_LEVEL, &attribs->level);
    glXGetConfig(dpy, vInfo, GLX_RGBA, &attribs->rgba);
    glXGetConfig(dpy, vInfo, GLX_DOUBLEBUFFER, &attribs->doubleBuffer);
    glXGetConfig(dpy, vInfo, GLX_STEREO, &attribs->stereo);
    glXGetConfig(dpy, vInfo, GLX_AUX_BUFFERS, &attribs->auxBuffers);
    glXGetConfig(dpy, vInfo, GLX_RED_SIZE, &attribs->redSize);
    glXGetConfig(dpy, vInfo, GLX_GREEN_SIZE, &attribs->greenSize);
    glXGetConfig(dpy, vInfo, GLX_BLUE_SIZE, &attribs->blueSize);
    glXGetConfig(dpy, vInfo, GLX_ALPHA_SIZE, &attribs->alphaSize);
    glXGetConfig(dpy, vInfo, GLX_DEPTH_SIZE, &attribs->depthSize);
    glXGetConfig(dpy, vInfo, GLX_STENCIL_SIZE, &attribs->stencilSize);
    glXGetConfig(dpy, vInfo, GLX_ACCUM_RED_SIZE, &attribs->accumRedSize);
    glXGetConfig(dpy, vInfo, GLX_ACCUM_GREEN_SIZE, &attribs->accumGreenSize);
    glXGetConfig(dpy, vInfo, GLX_ACCUM_BLUE_SIZE, &attribs->accumBlueSize);
    glXGetConfig(dpy, vInfo, GLX_ACCUM_ALPHA_SIZE, &attribs->accumAlphaSize);
    
    /* transparent pixel value not implemented yet */
    attribs->transparent = 0;
 
    /* multisample tests not implemented yet */
    attribs->numSamples = 0;
    attribs->numMultisample = 0;
 
#if defined(GLX_EXT_visual_rating)
    if (ext && strstr(ext, "GLX_EXT_visual_rating")) {
        glXGetConfig(dpy, vInfo, GLX_VISUAL_CAVEAT_EXT, &attribs->visualCaveat);
    }
    else {
        attribs->visualCaveat = GLX_NONE_EXT;
    }
#else
    attribs->visualCaveat = 0;
#endif
}

void print_visual_info(Display *dpy, int scrnum)
{
    XVisualInfo *visuals, temp;
    int numVisuals;
    long mask;
    int i;
 
    /* get list of all visuals on this screen */
    temp.screen = scrnum;
    mask = VisualScreenMask;
    visuals = XGetVisualInfo(dpy, mask, &temp, &numVisuals);
 

    print_visual_attribs_long_header();
    for (i = 0; i < numVisuals; i++) {
        struct visual_attribs attribs;
	get_visual_attribs(dpy, &visuals[i], &attribs);
	print_visual_attribs_long(&attribs);
    }

 
    XFree(visuals);
}

GLInfo::GLInfo()
{
    infotext = new QString("GLTest:\n");
    char *displayName = NULL;
    Display *dpy;
    int numScreens, scrnum;
    
 
    dpy = XOpenDisplay(displayName);
    if (!dpy) {
        fprintf(stderr, "Error: unable to open display %s\n", displayName);
    }
 
    numScreens = ScreenCount(dpy);
    infotext->append("name of display: ");
    infotext->append(DisplayString(dpy));
    infotext->append("\n");
    
    for (scrnum = 0; scrnum < numScreens; scrnum++) {
        mesa_hack(dpy, scrnum);
	print_screen_info(dpy, scrnum);
	printf("\n");
	print_visual_info(dpy, scrnum);
	if (scrnum + 1 < numScreens)
	    printf("\n\n");
    }
 
   XCloseDisplay(dpy);
 
};

QString GLInfo::getText()
{
  return *infotext;
}
