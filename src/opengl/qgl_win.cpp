/****************************************************************************
** $Id: $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition licenses for Windows
** may use this file in accordance with the Qt Commercial License Agreement
** provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


#include <qgl.h>
#include <qshared.h>
#include <qmemarray.h>
#include <qmap.h>
#include <qpixmap.h>

#include <windows.h>


class QGLCmapPrivate : public QShared
{
public:
    enum AllocState{ UnAllocated = 0, Allocated = 0x01, Reserved = 0x02 };

    int maxSize;
    QMemArray<uint> colorArray;
    QMemArray<Q_UINT8> allocArray;
    QMemArray<Q_UINT8> contextArray;
    QMap<uint,int> colorMap;
};

/*****************************************************************************
  QColorMap class - temporarily here, until it is ready for prime time
 *****************************************************************************/

/****************************************************************************
** $Id: $
**
** Definition of QColorMap class
**
** Created : 20000510
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/



#ifndef QGLCMAP_H
#define QGLCMAP_H

#include <qcolor.h>

class QGLCmapPrivate;

class /*Q_EXPORT*/ QGLCmap
{
public:
    enum Flags { Reserved = 0x01 };

    QGLCmap( int maxSize = 256 );
    QGLCmap( const QGLCmap& map );
    ~QGLCmap();

    QGLCmap& operator=( const QGLCmap& map );

    // isEmpty and/or isNull ?
    int size() const;
    int maxSize() const;

    void resize( int newSize );

    int find( QRgb color ) const;
    int findNearest( QRgb color ) const;
    int allocate( QRgb color, uint flags = 0, Q_UINT8 context = 0 );

    void setEntry( int idx, QRgb color, uint flags = 0, Q_UINT8 context = 0 );

    const QRgb* colors() const;

private:
    void detach();
    QGLCmapPrivate* d;
};
    
#endif

    
QGLCmap::QGLCmap( int maxSize ) // add a bool prealloc?
{
    d = new QGLCmapPrivate;
    d->maxSize = maxSize;
}


QGLCmap::QGLCmap( const QGLCmap& map )
{
    d = map.d;
    d->ref();
}


QGLCmap::~QGLCmap()
{
    if ( d && d->deref() )
	delete d;
    d = 0;
}


QGLCmap& QGLCmap::operator=( const QGLCmap& map )
{
    map.d->ref();
    if ( d->deref() )
	delete d;
    d = map.d;
    return *this;
}


int QGLCmap::size() const
{
    return d->colorArray.size();
}


int QGLCmap::maxSize() const
{
    return d->maxSize;
}


void QGLCmap::detach()
{
    if ( d->count != 1 ) {
	d->deref();
	QGLCmapPrivate* newd = new QGLCmapPrivate;
	newd->maxSize = d->maxSize;
	newd->colorArray = d->colorArray.copy();
	newd->allocArray = d->allocArray.copy();
	newd->contextArray = d->contextArray.copy();
	newd->colorMap = d->colorMap;
	d = newd;
    }
}


void QGLCmap::resize( int newSize )
{
#if defined (QT_CHECK_RANGE)
    if ( newSize < 0 || newSize > d->maxSize ) {
	qWarning( "QGLCmap::resize(): size out of range" );
	return;
    }
#endif
    int oldSize = size();
    detach();
    //### if shrinking; remove the lost elems from colorMap
    d->colorArray.resize( newSize );
    d->allocArray.resize( newSize );
    d->contextArray.resize( newSize );
    if ( newSize > oldSize ) {
	memset( d->allocArray.data() + oldSize, 0, newSize - oldSize );
	memset( d->contextArray.data() + oldSize, 0, newSize - oldSize );
    }
}


int QGLCmap::find( QRgb color ) const
{
    QMap<uint,int>::ConstIterator it = d->colorMap.find( color );
    if ( it != d->colorMap.end() )
	return *it;
    return -1;
}


int QGLCmap::findNearest( QRgb color ) const
{
    int idx = find( color );
    if ( idx >= 0 )
	return idx;
    int mapSize = size();
    int mindist = 200000;
    int r = qRed( color );
    int g = qGreen( color );
    int b = qBlue( color );
    int rx, gx, bx, dist;
    for ( int i=0; i < mapSize; i++ ) {
	if ( !(d->allocArray[i] & QGLCmapPrivate::Allocated) )
	    continue;
	QRgb ci = d->colorArray[i];
	rx = r - qRed( ci );
	gx = g - qGreen( ci );
	bx = b - qBlue( ci );
	dist = rx*rx + gx*gx + bx*bx;		// calculate distance
	if ( dist < mindist ) {			// minimal?
	    mindist = dist;
	    idx = i;
	}
    }
    return idx;
}




// Does not always allocate; returns existing c idx if found

int QGLCmap::allocate( QRgb color, uint flags, Q_UINT8 context )
{
    int idx = find( color );
    if ( idx >= 0 )
	return idx;

    int mapSize = d->colorArray.size();
    int newIdx = d->allocArray.find( QGLCmapPrivate::UnAllocated );

    if ( newIdx < 0 ) {			// Must allocate more room
	if ( mapSize < d->maxSize ) {
	    newIdx = mapSize;
	    mapSize++;
	    resize( mapSize );
	}
	else {
	    //# add a bool param that says what to do in case no more room -
	    // fail (-1) or return nearest?
	    return -1;
	}
    }

    d->colorArray[newIdx] = color;
    if ( flags & QGLCmap::Reserved ) {
	d->allocArray[newIdx] = QGLCmapPrivate::Reserved;
    }
    else {
	d->allocArray[newIdx] = QGLCmapPrivate::Allocated;
	d->colorMap.insert( color, newIdx );
    }
    d->contextArray[newIdx] = context;
    return newIdx;
}


void QGLCmap::setEntry( int idx, QRgb color, uint flags, Q_UINT8 context )
{
#if defined (QT_CHECK_RANGE)
    if ( idx < 0 || idx >= d->maxSize ) {
	qWarning( "QGLCmap::set(): Index out of range" );
	return;
    }
#endif
    detach();
    int mapSize = size();
    if ( idx >= mapSize ) {
	mapSize = idx + 1;
	resize( mapSize );
    }
    d->colorArray[idx] = color;
    if ( flags & QGLCmap::Reserved ) {
	d->allocArray[idx] = QGLCmapPrivate::Reserved;
    }
    else {
	d->allocArray[idx] = QGLCmapPrivate::Allocated;
	d->colorMap.insert( color, idx );
    }
    d->contextArray[idx] = context;
}


const QRgb* QGLCmap::colors() const
{
    return d->colorArray.data();
}



/*****************************************************************************
  QGLFormat Win32/WGL-specific code
 *****************************************************************************/


void qwglError( const char* method, const char* func )
{
#if defined(QT_CHECK_NULL)
    char* lpMsgBuf;
    FormatMessageA(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		  0, GetLastError(),
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  (char*) &lpMsgBuf, 0, 0 );
    qWarning( "%s : %s failed: %s", method, func, lpMsgBuf );
    LocalFree( lpMsgBuf );
#else
    Q_UNUSED( method );
    Q_UNUSED( func );
#endif
}



bool QGLFormat::hasOpenGL()
{
    return TRUE;
}


bool QGLFormat::hasOpenGLOverlays()
{
    static bool checkDone = FALSE;
    static bool hasOl = FALSE;

    if ( !checkDone ) {
	checkDone = TRUE;
	HDC dc = qt_display_dc();
	int pfiMax = DescribePixelFormat( dc, 0, 0, NULL );
	PIXELFORMATDESCRIPTOR pfd;
	for ( int pfi = 1; pfi <= pfiMax; pfi++ ) {
	    DescribePixelFormat( dc, pfi, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	    if ( (pfd.bReserved & 0x0f) && (pfd.dwFlags & PFD_SUPPORT_OPENGL)) {
		// This format has overlays/underlays
		LAYERPLANEDESCRIPTOR lpd;
		wglDescribeLayerPlane( dc, pfi, 1,
				       sizeof(LAYERPLANEDESCRIPTOR), &lpd );
		if ( lpd.dwFlags & LPD_SUPPORT_OPENGL ) {
		    hasOl = TRUE;
		    break;
		}
	    }
	}
    }
    return hasOl;
}


/*****************************************************************************
  QGLContext Win32/WGL-specific code
 *****************************************************************************/

static uchar qgl_rgb_palette_comp( int idx, uint nbits, uint shift )
{
    const uchar map_3_to_8[8] = {
	0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
    };
    const uchar map_2_to_8[4] = {
	0, 0x55, 0xaa, 0xff
    };
    const uchar map_1_to_8[2] = {
	0, 255
    };

    uchar val = (uchar) (idx >> shift);
    uchar res = 0;
    switch ( nbits ) {
    case 1:
        val &= 0x1;
        res =  map_1_to_8[val];
	break;
    case 2:
        val &= 0x3;
        res = map_2_to_8[val];
	break;
    case 3:
        val &= 0x7;
        res = map_3_to_8[val];
	break;
    default:
        res = 0;
    }
    return res;
}


static QRgb* qgl_create_rgb_palette( const PIXELFORMATDESCRIPTOR* pfd )
{
    if ( ( pfd->iPixelType != PFD_TYPE_RGBA ) ||
	 !(pfd->dwFlags & PFD_NEED_PALETTE) ||
	 ( pfd->cColorBits != 8 ) )
	return 0;
    int numEntries = 1 << pfd->cColorBits;
    QRgb* pal = new QRgb[numEntries];
    for ( int i = 0; i < numEntries; i++ ) {
	int r = qgl_rgb_palette_comp( i, pfd->cRedBits, pfd->cRedShift );
	int g = qgl_rgb_palette_comp( i, pfd->cGreenBits, pfd->cGreenShift );
	int b = qgl_rgb_palette_comp( i, pfd->cBlueBits, pfd->cBlueShift );
	pal[i] = qRgb( r, g, b );
    }

    const int syscol_indices[12] = {
	3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
    };

    const uint syscols[20] = {
	0x000000, 0x800000, 0x008000, 0x808000, 0x000080, 0x800080,
	0x008080, 0xc0c0c0, 0xc0dcc0, 0xa6caf0, 0xfffbf0, 0xa0a0a4,
	0x808080, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff,
	0x00ffff, 0xffffff
    };	// colors #1 - #12 are not present in pal; gets added below

    if ( ( pfd->cColorBits == 8 )				&&
	 ( pfd->cRedBits   == 3 ) && ( pfd->cRedShift   == 0 )	&&
	 ( pfd->cGreenBits == 3 ) && ( pfd->cGreenShift == 3 )	&&
	 ( pfd->cBlueBits  == 2 ) && ( pfd->cBlueShift  == 6 ) ) {
	for ( int j = 0 ; j < 12 ; j++ )
	    pal[syscol_indices[j]] = QRgb( syscols[j+1] );
    }

    return pal;
}


static QGLFormat pfdToQGLFormat( const PIXELFORMATDESCRIPTOR* pfd )
{
    QGLFormat fmt;
    fmt.setDoubleBuffer( pfd->dwFlags & PFD_DOUBLEBUFFER );
    fmt.setDepth( pfd->cDepthBits );
    fmt.setRgba( pfd->iPixelType == PFD_TYPE_RGBA );
    fmt.setAlpha( pfd->cAlphaBits );
    fmt.setAccum( pfd->cAccumBits );
    fmt.setStencil( pfd->cStencilBits );
    fmt.setStereo( pfd->dwFlags & PFD_STEREO );
    fmt.setDirectRendering( (pfd->dwFlags & PFD_GENERIC_ACCELERATED) ||
			    !(pfd->dwFlags & PFD_GENERIC_FORMAT) );
    fmt.setOverlay( (pfd->bReserved & 0x0f) != 0 );
    return fmt;
}

bool QGLContext::chooseContext( const QGLContext* shareContext )
{
    HDC myDc;

    if ( deviceIsPixmap() ) {
	if ( glFormat.plane() )
	    return FALSE;		// Pixmaps can't have overlay
	win = 0;
	myDc = d->paintDevice->handle();
    }
    else {
	win = ((QWidget*)d->paintDevice)->winId();
	myDc = GetDC( win );
    }

    if ( !myDc ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QGLContext::chooseContext(): Paint device cannot be null" );
#endif
	if ( win )
	    ReleaseDC( win, myDc );
	return FALSE;
    }

    if ( glFormat.plane() ) {
	pixelFormatId = ((QGLWidget*)d->paintDevice)->context()->pixelFormatId;
	if ( !pixelFormatId ) {		// I.e. the glwidget is invalid
#if defined(QT_CHECK_STATE)
	    qWarning( "QGLContext::chooseContext(): Cannot create overlay context for invalid widget" );
#endif
	    if ( win )
		ReleaseDC( win, myDc );
	    return FALSE;
	}

	rc = wglCreateLayerContext( myDc, glFormat.plane() );
	if ( !rc ) {
	    qwglError( "QGLContext::chooseContext()", "CreateLayerContext" );
	    if ( win )
		ReleaseDC( win, myDc );
	    return FALSE;
	}

	LAYERPLANEDESCRIPTOR lpfd;
	wglDescribeLayerPlane( myDc, pixelFormatId, glFormat.plane(),
			       sizeof( LAYERPLANEDESCRIPTOR ), &lpfd );
	glFormat.setDoubleBuffer( lpfd.dwFlags & LPD_DOUBLEBUFFER );
	glFormat.setDepth( lpfd.cDepthBits );
	glFormat.setRgba( lpfd.iPixelType == PFD_TYPE_RGBA );
	glFormat.setAlpha( lpfd.cAlphaBits );
	glFormat.setAccum( lpfd.cAccumBits );
	glFormat.setStencil( lpfd.cStencilBits );
	glFormat.setStereo( lpfd.dwFlags & LPD_STEREO );
	glFormat.setDirectRendering( FALSE );

	if ( glFormat.rgba() ) {
	    if ( lpfd.dwFlags & LPD_TRANSPARENT )
		d->transpColor = QColor( lpfd.crTransparent & 0xff,
				      (lpfd.crTransparent >> 8) & 0xff, 
				      (lpfd.crTransparent >> 16) & 0xff );
	    else
		d->transpColor = QColor( 0, 0, 0 );
	}
	else {
	    if ( lpfd.dwFlags & LPD_TRANSPARENT )
		d->transpColor = QColor( qRgb( 1, 2, 3 ), lpfd.crTransparent );
	    else
		d->transpColor = QColor( qRgb( 1, 2, 3 ), 0 );

	    cmap = new QGLCmap( 1 << lpfd.cColorBits );
	    cmap->setEntry( lpfd.crTransparent, qRgb( 1, 2, 3 ),
			    QGLCmap::Reserved );
	}

        if ( shareContext && shareContext->isValid() )
	    d->sharing = ( wglShareLists( shareContext->rc, rc ) != 0 );

	if ( win )
	    ReleaseDC( win, myDc );
	return TRUE;
    }

    PIXELFORMATDESCRIPTOR pfd;
    PIXELFORMATDESCRIPTOR realPfd;
    pixelFormatId = choosePixelFormat( &pfd, myDc );
    if ( pixelFormatId == 0 ) {
	qwglError( "QGLContext::chooseContext()", "ChoosePixelFormat" );
	if ( win )
	    ReleaseDC( win, myDc );
	return FALSE;
    }
    DescribePixelFormat( myDc, pixelFormatId, sizeof(PIXELFORMATDESCRIPTOR),
			 &realPfd );
    bool overlayRequested = glFormat.hasOverlay();
    glFormat = pfdToQGLFormat( &realPfd );
    glFormat.setOverlay( glFormat.hasOverlay() && overlayRequested );

    if ( deviceIsPixmap() && !(realPfd.dwFlags & PFD_DRAW_TO_BITMAP) ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QGLContext::chooseContext(): Failed to get pixmap rendering context." );
#endif
	return FALSE;
    }

    if ( deviceIsPixmap() && 
	 (((QPixmap*)d->paintDevice)->depth() != realPfd.cColorBits ) ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QGLContext::chooseContext(): Failed to get pixmap rendering context of suitable depth." );
#endif
	return FALSE;
    }

    if ( !SetPixelFormat(myDc, pixelFormatId, &realPfd) ) {
	qwglError( "QGLContext::chooseContext()", "SetPixelFormat" );
	if ( win )
	    ReleaseDC( win, myDc );
	return FALSE;
    }

    if ( !(rc = wglCreateLayerContext( myDc, 0 ) ) ) { //### just createcontext() if not overlay?
	qwglError( "QGLContext::chooseContext()", "wglCreateContext" );
	if ( win )
	    ReleaseDC( win, myDc );
	return FALSE;
    }

    if ( shareContext && shareContext->isValid() )
	d->sharing = ( wglShareLists( shareContext->rc, rc ) != 0 );

    if ( win )
	ReleaseDC( win, myDc );

    QRgb* pal = qgl_create_rgb_palette( &realPfd );
    if ( pal ) {
	QColor::setPaletteEntries( pal, 256, 0 );
	delete[] pal;
    }

    return TRUE;
}



static bool qLogEq( bool a, bool b )
{
    return ( ((!a) && (!b)) || (a && b) );
}

/*! 

<strong>Win32 only</strong> This virtual function chooses a pixel format
that matches the OpenGL \link setFormat() format\endlink. Reimplement this function in a subclass if you need a custom context.

  \warning The \a dummyPfd pointer and \a pdc are used as a \c PIXELFORMATDESCRIPTOR*.
  We use \c void to avoid using Windows-specific types in our header files.

  \sa chooseContext() */

int QGLContext::choosePixelFormat( void* dummyPfd, HDC pdc )
{
    int pmDepth = deviceIsPixmap() ? ((QPixmap*)d->paintDevice)->depth() : 0;
    PIXELFORMATDESCRIPTOR* p = (PIXELFORMATDESCRIPTOR*)dummyPfd;
    memset( p, 0, sizeof(PIXELFORMATDESCRIPTOR) );
    p->nSize = sizeof(PIXELFORMATDESCRIPTOR);
    p->nVersion = 1;
    p->dwFlags  = PFD_SUPPORT_OPENGL;
    if ( deviceIsPixmap() )
	p->dwFlags |= PFD_DRAW_TO_BITMAP;
    else
	p->dwFlags |= PFD_DRAW_TO_WINDOW;
    if ( glFormat.doubleBuffer() && !deviceIsPixmap() )
	p->dwFlags |= PFD_DOUBLEBUFFER;
    if ( glFormat.stereo() )
	p->dwFlags |= PFD_STEREO;
    if ( glFormat.depth() )
	p->cDepthBits = 32;
    else
	p->dwFlags |= PFD_DEPTH_DONTCARE;
    if ( glFormat.rgba() ) {
	p->iPixelType = PFD_TYPE_RGBA;
	if ( deviceIsPixmap() )
	    p->cColorBits = pmDepth;
	else
	    p->cColorBits = 32;
    } else {
	p->iPixelType = PFD_TYPE_COLORINDEX;
	p->cColorBits = 8;
    }
    if ( glFormat.alpha() )
	p->cAlphaBits = 8;
    if ( glFormat.accum() )
	p->cAccumBits = p->cColorBits + p->cAlphaBits;
    if ( glFormat.stencil() )
	p->cStencilBits = 4;
    p->iLayerType = PFD_MAIN_PLANE;
    int chosenPfi = ChoosePixelFormat( pdc, p );
#ifndef QT_NO_DEBUG
    if ( !chosenPfi )
	qSystemWarning( "QGLContext: Call of ChoosePixelFormat failed!" );
#endif

    // Since the GDI function ChoosePixelFormat() does not handle
    // overlay and direct-rendering requests, we must roll our own here 

    bool doSearch = chosenPfi <= 0;
    PIXELFORMATDESCRIPTOR pfd;
    QGLFormat fmt;
    if ( !doSearch ) {
	DescribePixelFormat( pdc, chosenPfi, sizeof(PIXELFORMATDESCRIPTOR), 
			     &pfd );
	fmt = pfdToQGLFormat( &pfd );
	if ( glFormat.hasOverlay() && !fmt.hasOverlay() )
	    doSearch = TRUE;
	else if ( !qLogEq( glFormat.directRendering(), fmt.directRendering() ))
	    doSearch = TRUE;
	else if ( deviceIsPixmap() && ( !(pfd.dwFlags & PFD_DRAW_TO_BITMAP) || 
					pfd.cColorBits != pmDepth ) )
	    doSearch = TRUE;
	else if ( !deviceIsPixmap() && !(pfd.dwFlags & PFD_DRAW_TO_WINDOW) )
	    doSearch = TRUE;
	else if ( !qLogEq( glFormat.rgba(), fmt.rgba() ) )
	    doSearch = TRUE;
    }

    if ( doSearch ) {
	int pfiMax = DescribePixelFormat( pdc, 0, 0, NULL );
	int bestScore = -1;
	int bestPfi = -1;
	for ( int pfi = 1; pfi <= pfiMax; pfi++ ) {
	    DescribePixelFormat( pdc, pfi, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	    if ( !(pfd.dwFlags & PFD_SUPPORT_OPENGL) )
		continue;
	    if ( deviceIsPixmap() && ( !(pfd.dwFlags & PFD_DRAW_TO_BITMAP) ||
				       pfd.cColorBits != pmDepth ) )
		continue;
	    if ( !deviceIsPixmap() && !(pfd.dwFlags & PFD_DRAW_TO_WINDOW) )
		continue;

	    fmt = pfdToQGLFormat( &pfd );
	    if ( glFormat.hasOverlay() && !fmt.hasOverlay() )
		continue;

	    int score = pfd.cColorBits;
	    if ( qLogEq( glFormat.depth(), fmt.depth() ) )
		score += pfd.cDepthBits;
	    if ( qLogEq( glFormat.alpha(), fmt.alpha() ) )
		score += pfd.cAlphaBits;
	    if ( qLogEq( glFormat.accum(), fmt.accum() ) )
		score += pfd.cAccumBits;
	    if ( qLogEq( glFormat.stencil(), fmt.stencil() ) )
		score += pfd.cStencilBits;
	    if ( qLogEq( glFormat.doubleBuffer(), fmt.doubleBuffer() ) )
		score += 1000;
	    if ( qLogEq( glFormat.stereo(), fmt.stereo() ) )
		score += 2000;
	    if ( qLogEq( glFormat.directRendering(), fmt.directRendering() ) )
		score += 4000;
	    if ( qLogEq( glFormat.rgba(), fmt.rgba() ) )
		score += 8000;
	    
	    if ( score > bestScore ) {
		bestScore = score;
		bestPfi = pfi;
	    }
	}

	if ( bestPfi > 0 )
	    chosenPfi = bestPfi;
    }
    return chosenPfi;
}



void QGLContext::reset()
{
    if ( !d->valid )
	return;
    doneCurrent();
    if ( rc )
	wglDeleteContext( rc );
    rc  = 0;
    if ( win && dc )
	ReleaseDC( win, dc );
    dc  = 0;
    win = 0;
    pixelFormatId = 0;
    d->sharing = FALSE;
    d->valid = FALSE;
    d->transpColor = QColor();
    delete cmap;
    cmap = 0;
    d->initDone = FALSE;
}



//
// NOTE: In a multi-threaded environment, each thread has a current
// context. If we want to make this code thread-safe, we probably
// have to use TLS (thread local storage) for keeping current contexts.
//

void QGLContext::makeCurrent()
{
    if ( currentCtx ) {
	if ( currentCtx == this )		// already current
	    return;
	currentCtx->doneCurrent();
    }
    if ( !d->valid )
	return;
    if ( win )
	dc = GetDC( win );
    else
	dc = d->paintDevice->handle();
    if ( QColor::hPal() ) {
	SelectPalette( dc, QColor::hPal(), FALSE );
	RealizePalette( dc );
    }
    if ( glFormat.plane() ) {
	wglRealizeLayerPalette( dc, glFormat.plane(), TRUE );
    }

    if ( !wglMakeCurrent( dc, rc ) )
	qwglError( "QGLContext::makeCurrent()", "wglMakeCurrent" );
    currentCtx = this;
}


void QGLContext::doneCurrent()
{
    if ( currentCtx != this )
	return;
    currentCtx = 0;
    //#### should use wglRealizeLayerPalette to release colors here?
    // depending on visibility of window?
    wglMakeCurrent( 0, 0 );
    if ( win && dc ) {
	ReleaseDC( win, dc );
	dc = 0;
    }
}


void QGLContext::swapBuffers() const
{
    if ( dc && glFormat.doubleBuffer() && !deviceIsPixmap() ) {
	if ( glFormat.plane() )
	    wglSwapLayerBuffers( dc, WGL_SWAP_OVERLAY1 );  //### hardcoded ol1
	else {
	    if ( glFormat.hasOverlay() )
		wglSwapLayerBuffers( dc, WGL_SWAP_MAIN_PLANE );
	    else
		SwapBuffers( dc );
	}
    }
}


QColor QGLContext::overlayTransparentColor() const
{
    return d->transpColor;
}


uint QGLContext::colorIndex( const QColor& c ) const
{
    if ( !isValid() )
	return 0;
    if ( cmap ) {
	int idx = cmap->find( c.rgb() );
	if ( idx >= 0 )
	    return idx;
	if ( dc && glFormat.plane() ) {
	    idx = cmap->allocate( c.rgb() );
	    if ( idx >= 0 ) {
		COLORREF r = RGB(qRed(c.rgb()),qGreen(c.rgb()),qBlue(c.rgb()));
		wglSetLayerPaletteEntries( dc, glFormat.plane(), idx, 1, &r );
		wglRealizeLayerPalette( dc, glFormat.plane(), TRUE );
		return idx;
	    }
	}
	return cmap->findNearest( c.rgb() );
    }
    return c.pixel() & 0x00ffffff;		// Assumes standard palette
}

/*****************************************************************************
  QGLWidget Win32/WGL-specific code
 *****************************************************************************/

void QGLWidget::init( const QGLFormat& fmt, const QGLWidget* shareWidget )
{
    glcx = 0;
    autoSwap = TRUE;

    if ( shareWidget )
	setContext( new QGLContext( fmt, this ), shareWidget->context() );
    else
	setContext( new QGLContext( fmt, this ) );
    setBackgroundMode( NoBackground );

    if ( isValid() && format().hasOverlay() ) {
	olcx = new QGLContext( QGLFormat::defaultOverlayFormat(), this );
        if ( !olcx->create(shareWidget ? shareWidget->overlayContext() : 0) ) {
	    delete olcx;
	    olcx = 0;
	    glcx->glFormat.setOverlay( FALSE );
	}
    }
    else {
	olcx = 0;
    }
}


void QGLWidget::reparent( QWidget* parent, WFlags f, const QPoint& p,
			  bool showIt )
{
    QWidget::reparent( parent, f, p, FALSE );
    setContext( new QGLContext( glcx->requestedFormat(), this ) );
    if ( showIt )
	show();
}


void QGLWidget::setMouseTracking( bool enable )
{
    QWidget::setMouseTracking( enable );
}


void QGLWidget::resizeEvent( QResizeEvent * )
{
    if ( !isValid() )
	return;
    makeCurrent();
    if ( !glcx->initialized() )
	glInit();
    resizeGL( width(), height() );
    if ( olcx ) {
	makeOverlayCurrent();
	resizeOverlayGL( width(), height() );
    }
}


const QGLContext* QGLWidget::overlayContext() const
{
    return olcx;
}


void QGLWidget::makeOverlayCurrent()
{
    if ( olcx ) {
	olcx->makeCurrent();
	if ( !olcx->initialized() ) {
	    initializeOverlayGL();
	    olcx->setInitialized( TRUE );
	}
    }
}


void QGLWidget::updateOverlayGL()
{
    if ( olcx ) {
	makeOverlayCurrent();
	paintOverlayGL();
	if ( olcx->format().doubleBuffer() ) {
	    if ( autoSwap )
		olcx->swapBuffers();
	}
	else {
	    glFlush();
	}
    }
}


void QGLWidget::setContext( QGLContext *context,
			    const QGLContext* shareContext,
			    bool deleteOldContext )
{
    if ( context == 0 ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QGLWidget::setContext: Cannot set null context" );
#endif
	return;
    }
    if ( !context->deviceIsPixmap() && context->device() != this ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QGLWidget::setContext: Context must refer to this widget" );
#endif
	return;
    }

    if ( glcx )
	glcx->doneCurrent();
    QGLContext* oldcx = glcx;
    glcx = context;

    bool doShow = FALSE;
    if ( oldcx && oldcx->win == winId() && !glcx->deviceIsPixmap() ) {
	// We already have a context and must therefore create a new
	// window since Windows does not permit setting a new OpenGL
	// context for a window that already has one set.
	doShow = isVisible();
	QWidget::reparent( parentWidget(), 0, geometry().topLeft(), FALSE );
    }

    if ( !glcx->isValid() )
	glcx->create( shareContext ? shareContext : oldcx );

    if ( deleteOldContext )
	delete oldcx;

    if ( doShow )
	show();
}


bool QGLWidget::renderCxPm( QPixmap* )
{
    return FALSE;
}


const QGLColormap & QGLWidget::colormap() const
{
    return cmap;
}

/*\internal
  Store color values in the given colormap.
*/
static void qStoreColors( HPALETTE cmap, const QGLColormap & cols )
{
    QRgb color;
    PALETTEENTRY pe;
    
    for ( int i = 0; i < cols.size(); i++ ) {
	color = cols.entryRgb( i );
	pe.peRed   = qRed( color );
	pe.peGreen = qGreen( color );
	pe.peBlue  = qBlue( color );
	pe.peFlags = 0;

	SetPaletteEntries( cmap, i, 1, &pe );
    }
}

void QGLWidget::setColormap( const QGLColormap & c )
{
    QWidget * tlw   = topLevelWidget(); // must return a valid widget

    cmap = c;
    if ( !cmap.d )
	return;
    
    if ( cmap.d->cmapHandle ) { // already have an allocated cmap
        HDC hdc = GetDC( winId() );
	SelectPalette( hdc, (HPALETTE) cmap.d->cmapHandle, FALSE );
	qStoreColors( (HPALETTE) cmap.d->cmapHandle, c );
        RealizePalette( hdc );
	ReleaseDC( winId(), hdc );
    } else {
        LOGPALETTE * lpal = (LOGPALETTE *) malloc( sizeof(LOGPALETTE)
			    + c.size() * sizeof(PALETTEENTRY) );

	lpal->palVersion    = 0x300;
	lpal->palNumEntries = c.size();
	cmap.d->cmapHandle  = CreatePalette( lpal );

	if ( cmap.d->cmapHandle ) {
	    HDC hdc = GetDC( winId() );
	    SelectPalette( hdc, (HPALETTE) cmap.d->cmapHandle, FALSE );
	    qStoreColors( (HPALETTE) cmap.d->cmapHandle, c );
	    RealizePalette( hdc );
	    ReleaseDC( winId(), hdc );
	}
	free( lpal );
    }
}

void QGLWidget::cleanupColormaps()
{	
    if ( !cmap.d )
	return;
    
    if ( cmap.d->cmapHandle ) {
	HDC hdc = GetDC( winId() ); 
	SelectPalette( hdc, (HPALETTE) GetStockObject( DEFAULT_PALETTE ),
		       FALSE );
	DeleteObject( (HPALETTE) cmap.d->cmapHandle );
	ReleaseDC( winId(), hdc );
	cmap.d->cmapHandle = 0;
    }
}
