/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

/****************************************************************************
**
** This is the info widget for windows.
**
** Some of the code was borrowed from Nate Robins wglinfo.c
**
****************************************************************************/

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

#include "glinfo.h"

#include <qstring.h>

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>


GLInfo::GLInfo(QWidget* parent, const char* name)
    : QGLWidget(parent, name)
{
    infotext = new QString("GLTest:\n");
    viewlist = new QStringList();
};

QString GLInfo::getText()
{
    int   i;
    char* s;
    char  t[80];
    char* p;
    HDC dc;

    makeCurrent();

    infotext->sprintf("%sdisplay: N/A\n"
		      "server wgl vendor string: N/A\n"
		      "server wgl version string: N/A\n"
		      "server wgl extensions (WGL_): N/A\n"
		      "client wgl version: N/A\n"
		      "client wgl extensions (WGL_): none\n"
		      "OpenGL vendor string: %s\n", (const char*)*infotext, 
		      ((const char*)glGetString(GL_VENDOR)));
    infotext->sprintf("%sOpenGL renderer string: %s\n", (const char*)*infotext, 
		      glGetString(GL_RENDERER));
    infotext->sprintf("%sOpenGL version string: %s\n", (const char*)*infotext, 
		      glGetString(GL_VERSION));
    infotext->sprintf("%sOpenGL extensions (GL_): \n", (const char*)*infotext);


    /* do the magic to separate all extensions with comma's, except
       for the last one that _may_ terminate in a space. */
    i = 0;
    s = (char*)glGetString(GL_EXTENSIONS);
    t[79] = '\0';
    while(*s) {
        t[i++] = *s;
	if(*s == ' ') {
	    if (*(s+1) != '\0') {
	        t[i-1] = ',';
		t[i] = ' ';
		p = &t[i++];
	    } else {       // zoinks! last one terminated in a space! //
	        t[i-1] = '\0';
	    }
	}
	if(i > 80 - 5) {
	    *p = t[i] = '\0';
	    infotext->sprintf("%s    %s\n", (const char*)*infotext, t);
	    p++;
	    i = strlen(p);
	    strcpy(t, p);
	}
	s++;
    }
    t[i] = '\0';
    infotext->sprintf("%s    %s.", (const char*)*infotext, t);

    dc = GetDC( winId() );
    VisualInfo( dc );	
    ReleaseDC( winId(), dc );

    return *infotext;
}

QStringList GLInfo::getViewList()
{
  return *viewlist;
}

void GLInfo::VisualInfo(HDC hDC)
{
    QString str;
    int i, maxpf;
    PIXELFORMATDESCRIPTOR pfd;
	
    /* calling DescribePixelFormat() with NULL args return maximum
       number of pixel formats */
    maxpf = DescribePixelFormat(hDC, 0, 0, NULL);
	
	
    /* loop through all the pixel formats */
    for(i = 1; i <= maxpf; i++) { 
        str = "";
	DescribePixelFormat(hDC, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
		
	/* only describe this format if it supports OpenGL */
	if(!(pfd.dwFlags & PFD_SUPPORT_OPENGL))
	    continue;
		
	/* print out the information for this pixel format */
	str.sprintf("0x%02x %d ", i, pfd.cColorBits);
		
	//printf("%2d ", pfd.cColorBits);
	if(pfd.dwFlags & PFD_DRAW_TO_WINDOW)
	    str.sprintf("%swindow ", (const char*)str);
	else if(pfd.dwFlags & PFD_DRAW_TO_BITMAP) 
	    str.sprintf("%sbitmap ", (const char*)str);
	else 
	    str.sprintf("%s. ", (const char*)str);
		
	/* should find transparent pixel from LAYERPLANEDESCRIPTOR */
	str.sprintf("%s0 %2d ", (const char*)str, pfd.cColorBits);
		
	/* bReserved field indicates number of over/underlays */
	if(pfd.bReserved) 
	    str.sprintf("%s%d ", (const char*)str, pfd.bReserved);
	else 
	    str.sprintf("%s0 ", (const char*)str);
		
	if(pfd.iPixelType == PFD_TYPE_RGBA)
	    str.sprintf("%srgba ", (const char*)str);
	else
	    str.sprintf("%scolor_index ", (const char*)str);

	str.sprintf("%s%c %c ", (const char*)str, 
		    pfd.dwFlags & PFD_DOUBLEBUFFER ? 'y' : 'n',
		    pfd.dwFlags & PFD_STEREO ? 'y' : 'n');
		
	if(pfd.cRedBits && pfd.iPixelType == PFD_TYPE_RGBA)
	    str.sprintf("%s%d ", (const char*)str, pfd.cRedBits);
	else 
	    str.sprintf("%s0 ", (const char*)str); 
		
	if(pfd.cGreenBits && pfd.iPixelType == PFD_TYPE_RGBA)
	    str.sprintf("%s%d ", (const char*)str, pfd.cGreenBits);
	else 
	    str.sprintf("%s0 ", (const char*)str);
		
	if(pfd.cBlueBits && pfd.iPixelType == PFD_TYPE_RGBA)
	    str.sprintf("%s%d ", (const char*)str, pfd.cBlueBits);
	else 
	    str.sprintf("%s0 ", (const char*)str);
		
	if(pfd.cAlphaBits && pfd.iPixelType == PFD_TYPE_RGBA)
	    str.sprintf("%s%d ", (const char*)str, pfd.cAlphaBits);
	else 
	    str.sprintf("%s0 ", (const char*)str);
		
	if(pfd.cAuxBuffers)     
	    str.sprintf("%s%d ", (const char*)str, pfd.cAuxBuffers);
	else 
	    str.sprintf("%s0 ", (const char*)str);
		
	if(pfd.cDepthBits)      
	    str.sprintf("%s%d ", (const char*)str, pfd.cDepthBits);
	else 
	    str.sprintf("%s0 ", (const char*)str);
		
	if(pfd.cStencilBits)
	    str.sprintf("%s%d ", (const char*)str, pfd.cStencilBits);
	else 
	    str.sprintf("%s0 ", (const char*)str);
		
	if(pfd.cAccumRedBits)
	    str.sprintf("%s%d ", (const char*)str, pfd.cAccumRedBits);
	else 
	    str.sprintf("%s0 ", (const char*)str);
		
	if(pfd.cAccumGreenBits)
	    str.sprintf("%s%d ", (const char*)str, pfd.cAccumGreenBits);
	else 
	    str.sprintf("%s0 ", (const char*)str);
		
	if(pfd.cAccumBlueBits)
	    str.sprintf("%s%d ", (const char*)str, pfd.cAccumBlueBits);
	else 
	    str.sprintf("%s0 ", (const char*)str);
		
	if(pfd.cAccumAlphaBits)
	    str.sprintf("%s%d ", (const char*)str, pfd.cAccumAlphaBits);
	else 
	    str.sprintf("%s0 ", (const char*)str);

	/* no multisample in Win32 */
	str.sprintf("%s0 0", (const char*)str);

	viewlist->append(str);
    }
}
