/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

#include "glinfo.h"
#include <qgl.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
//#include "wglext.h"

typedef const char * (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);

GLInfo::GLInfo()
{
    QGLWidget gl( 0 );
    gl.makeCurrent();

    // get hold of WGL extensions
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = 
	(PFNWGLGETEXTENSIONSSTRINGARBPROC) wglGetProcAddress( "wglGetExtensionsStringARB" );
    QString wglExts;
    if ( wglGetExtensionsStringARB ) {
	wglExts = (char *) wglGetExtensionsStringARB( wglGetCurrentDC() );
	wglExts.replace( ' ', '\n' );
    }
    infotext.sprintf( "OpenGL vendor string: %s\n", (const char *) glGetString( GL_VENDOR ) );
    infotext.sprintf( "%sOpenGL renderer string: %s\n", infotext.ascii(), glGetString( GL_RENDERER ) );
    infotext.sprintf( "%sOpenGL version string: %s\n", infotext.ascii(), glGetString( GL_VERSION ) );
    infotext.sprintf( "%s\nWGL extensions (WGL_):\n%s\n", infotext.ascii(),
		       !wglExts.isEmpty() ? wglExts.ascii() : "None\n" );
    infotext.sprintf( "%sOpenGL extensions (GL_): \n", infotext.ascii() );
    infotext += QString( (char *) glGetString( GL_EXTENSIONS ) ).replace( ' ', '\n' );
    
    HDC dc;
    dc = GetDC( gl.winId() );
    
    QString str;
    int i, maxpf;
    PIXELFORMATDESCRIPTOR pfd;
	
    /* calling DescribePixelFormat() with NULL args return maximum
       number of pixel formats */
    maxpf = DescribePixelFormat( dc, 0, 0, NULL );
	
    for(i = 1; i <= maxpf; i++) { 
        str = "";
	DescribePixelFormat( dc, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd );
	if ( !(pfd.dwFlags & PFD_SUPPORT_OPENGL) )
	    continue;
	str.sprintf( "0x%02x   %2d  ", i, pfd.cColorBits );
	if( pfd.dwFlags & PFD_DRAW_TO_WINDOW && pfd.dwFlags & PFD_DRAW_TO_BITMAP )
	    str.sprintf( "%swin/bmp  ", str.ascii() );
	else if( pfd.dwFlags & PFD_DRAW_TO_WINDOW )
	    str.sprintf( "%swindow   ", str.ascii() );
	else if( pfd.dwFlags & PFD_DRAW_TO_BITMAP ) 
	    str.sprintf( "%sbitmap   ", str.ascii() );
	else 
	    str.sprintf( "%s.        ", str.ascii() );
		
	/* should find transparent pixel from LAYERPLANEDESCRIPTOR */
	str.sprintf( "%s0     %2d   ", str.ascii(), pfd.cColorBits );
		
	/* bReserved field indicates number of over/underlays */
	str.sprintf( "%s%d   ", str.ascii(), pfd.bReserved );
	str.sprintf( "%srgba  ", str.ascii() );
	str.sprintf( "%s%c   %c   ", str.ascii(),
		     pfd.dwFlags & PFD_DOUBLEBUFFER ? 'y' : 'n',
		     pfd.dwFlags & PFD_STEREO ? 'y' : 'n' );
	str.sprintf( "%s%d   ", str.ascii(), pfd.cRedBits );
	str.sprintf( "%s%d   ", str.ascii(), pfd.cGreenBits );
	str.sprintf( "%s%d   ", str.ascii(), pfd.cBlueBits );
	str.sprintf( "%s%d   ", str.ascii(), pfd.cAlphaBits );
	str.sprintf( "%s%d  ", str.ascii(), pfd.cAuxBuffers );
	str.sprintf( "%s%2d  ", str.ascii(), pfd.cDepthBits );
	str.sprintf( "%s%2d   ", str.ascii(), pfd.cStencilBits );
	str.sprintf( "%s%2d  ", str.ascii(), pfd.cAccumRedBits );
	str.sprintf( "%s%2d  ", str.ascii(), pfd.cAccumGreenBits );
	str.sprintf( "%s%2d  ", str.ascii(), pfd.cAccumBlueBits );
	str.sprintf( "%s%2d  ", str.ascii(), pfd.cAccumAlphaBits );

	/* no multisample in Win32 */
	str.append( "0   0\n" );
	config.append( str );
    }
    config = " PF  Color  Draw  Trans  buff lev render DB ste  r   g   b   a  aux dep ste  accum buffers  MS   MS\n"
	     " id  depth   to   parent size el   type     reo sz  sz  sz  sz  buf th  ncl  r   g   b   a  num bufs\n"
	     "----------------------------------------------------------------------------------------------------\n" + config;
    ReleaseDC( gl.winId(), dc );
}

QString GLInfo::extensions()
{
    return infotext;
}

QString GLInfo::configs()
{
    return config;
}
