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
#include <qlistview.h>
#include <qlayout.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qsplitter.h>

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include "wglext.h"

GLInfo::GLInfo(QWidget* parent, const char* name)
    : QDialog(parent, name)
{
    glw = new QGLWidget( this );
    
    infotext = new QString("GLTest:\n");
    viewlist = new QStringList();
    
    QSplitter *sp = new QSplitter( Qt::Vertical, this, "splitter" );
    QVBoxLayout *layout = new QVBoxLayout(this);
    infoView = new QTextView( sp, "infoView" );
    infoView->setTextFormat( Qt::PlainText );
    layout->addWidget( sp );
 
    infoList = new QListView( sp, "infoList" );
    infoList->addColumn( trUtf8( "Id", "" ) );
    infoList->addColumn( trUtf8( "Colorbits", "" ) );
    infoList->addColumn( trUtf8( "Draw to", "" ) );
    infoList->addColumn( trUtf8( "Transparent", "" ) );
    infoList->addColumn( trUtf8( "Buff size", "" ) );
    infoList->addColumn( trUtf8( "Level", "" ) );
    infoList->addColumn( trUtf8( "Render Type", "" ) );
    infoList->addColumn( trUtf8( "DB", "" ) );
    infoList->addColumn( trUtf8( "Stereo", "" ) );
    infoList->addColumn( trUtf8( "R sz", "" ) );
    infoList->addColumn( trUtf8( "G sz", "" ) );
    infoList->addColumn( trUtf8( "B sz", "" ) );
    infoList->addColumn( trUtf8( "A sz", "" ) );
    infoList->addColumn( trUtf8( "Aux Buff", "" ) );
    infoList->addColumn( trUtf8( "Depth", "" ) );
    infoList->addColumn( trUtf8( "Stencil", "" ) );
    infoList->addColumn( trUtf8( "R accum", "" ) );
    infoList->addColumn( trUtf8( "G accum", "" ) );
    infoList->addColumn( trUtf8( "B accum", "" ) );
    infoList->addColumn( trUtf8( "A accum", "" ) );
    infoList->addColumn( trUtf8( "MS num", "" ) );
    infoList->addColumn( trUtf8( "MS bufs", "" ) );
    infoList->setSelectionMode( QListView::Extended );
    infoList->setAllColumnsShowFocus( TRUE );
    
    QHBoxLayout *buttonLayout = new QHBoxLayout( 0 );
    layout->addLayout( buttonLayout );
    buttonLayout->addStretch();
    
    QPushButton *ok = new QPushButton( "Ok", this );
    connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ));
    buttonLayout->addWidget( ok );
    
    int i;
    QListViewItem *item;
    QStringList list, listItem;
    infoView->setText( getText() );
    for ( QStringList::Iterator it = viewlist->begin(); it != viewlist->end(); ++it ) {
        i = 0;
        item = new QListViewItem(infoList);
        listItem = QStringList::split(" ", (*it).latin1());
        for ( QStringList::Iterator ti = listItem.begin(); ti != listItem.end(); ++ti ) {
            item->setText(i, (*ti).latin1());
            i++;
        }
        infoList->insertItem(item);
    }
}

QString GLInfo::getText()
{
    HDC dc;

    glw->makeCurrent();
    // get hold of WGL extensions
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = 
	(PFNWGLGETEXTENSIONSSTRINGARBPROC) wglGetProcAddress("wglGetExtensionsStringARB");
    QString wglExts;
    if ( wglGetExtensionsStringARB ) {
	wglExts = (char *) wglGetExtensionsStringARB( wglGetCurrentDC() );
	wglExts.replace( ' ', '\n' );
    }
    infotext->sprintf( "OpenGL vendor string: %s\n", (const char*) glGetString(GL_VENDOR) );
    infotext->sprintf( "%sOpenGL renderer string: %s\n",
		       infotext->latin1(), 
		       glGetString(GL_RENDERER));
    infotext->sprintf( "%sOpenGL version string: %s\n",
		       infotext->latin1(), 
		       glGetString(GL_VERSION));
    infotext->sprintf( "%s\nWGL extension version: %.1f\nWGL extensions (WGL_):\n%s\n",
		       infotext->latin1(),
		       (float) WGL_WGLEXT_VERSION,
		       !wglExts.isEmpty() ? wglExts.latin1() : "None\n" );
    infotext->sprintf("%sOpenGL extensions (GL_): \n", infotext->latin1() );
    *infotext += QString( (char *) glGetString( GL_EXTENSIONS ) ).replace( ' ', '\n' );
    
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
	if(pfd.dwFlags & PFD_DRAW_TO_WINDOW && pfd.dwFlags & PFD_DRAW_TO_BITMAP )
	    str.sprintf("%swin/bmp ", (const char*)str);
	else if(pfd.dwFlags & PFD_DRAW_TO_WINDOW )
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
