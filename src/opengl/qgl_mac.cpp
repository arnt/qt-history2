/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/src/qgl_mac.cpp#2 $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses for Unix/X11 may
** use this file in accordance with the Qt Commercial License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qgl.h"

#if defined(Q_WS_MAC)
#include <agl.h>
#include <gl.h>
#include <CodeFragments.h>
#include <Gestalt.h>

#include <qpixmap.h>
#include <qapplication.h>
#include <qintdict.h>

/*****************************************************************************
  QGLFormat UNIX/AGL-specific code
 *****************************************************************************/

bool QGLFormat::hasOpenGL()
{
    return TRUE;
}


bool QGLFormat::hasOpenGLOverlays()
{
    return FALSE;
}



/*****************************************************************************
  QGLContext AGL-specific code
 *****************************************************************************/

bool QGLContext::chooseContext( const QGLContext* shareContext )
{
    vi = chooseMacVisual();
    if ( !vi )
	return FALSE;
    return TRUE;
}


/*
  <strong>Mac only</strong>: This virtual function tries to find a
  visual that matches the format, reducing the demands if the original
  request cannot be met.

  The algorithm for reducing the demands of the format is quite
  simple-minded, so override this method in your subclass if your
  application has spcific requirements on visual selection.

  \sa chooseContext()
*/

void *QGLContext::chooseMacVisual()
{
    return 0;
}

void QGLContext::reset()
{
    if ( !valid )
	return;
//    aglDestroyContext( paintDevice->x11Display(), (AGLContext)cx );
#if 0
    if ( vi )
	XFree( vi );
#endif
    vi = 0;
    crWin = FALSE;
    sharing = FALSE;
    valid = FALSE;
    transpColor = QColor();
    initDone = FALSE;
}


void QGLContext::makeCurrent()
{
    if ( !valid ) {
#if defined(QT_CHECK_STATE)
	qWarning("QGLContext::makeCurrent(): Cannot make invalid context current.");
#endif
	return;
    }
}

void QGLContext::doneCurrent()
{
    currentCtx = 0;
}


void QGLContext::swapBuffers() const
{
    if ( !valid )
	return;
}

QColor QGLContext::overlayTransparentColor() const
{
    return QColor();		// Invalid color
}


uint QGLContext::colorIndex( const QColor& c ) const
{
    return 0;
}


/*****************************************************************************
  QGLWidget AGL-specific code
 *****************************************************************************/

void QGLWidget::init( const QGLFormat& format, const QGLWidget* shareWidget )
{
    glcx = 0;
    autoSwap = TRUE;

    if ( shareWidget )
	setContext( new QGLContext( format, this ), shareWidget->context() );
    else
	setContext( new QGLContext( format, this ) );
    setBackgroundMode( NoBackground );
}


void QGLWidget::reparent( QWidget* parent, WFlags f, const QPoint& p,
			  bool showIt )
{
    QWidget::reparent( parent, f, p, showIt);
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
//    aglWaitX();
    resizeGL( width(), height() );
}



const QGLContext* QGLWidget::overlayContext() const
{
    return 0;
}


void QGLWidget::makeOverlayCurrent()
{
}


void QGLWidget::updateOverlayGL()
{
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

    bool createFailed = FALSE;
    if ( !glcx->isValid() ) {
	if ( !glcx->create( shareContext ? shareContext : oldcx ) )
	    createFailed = TRUE;
    }
    if ( createFailed ) {
	if ( deleteOldContext )
	    delete oldcx;
	return;
    }

    if ( glcx->windowCreated() || glcx->deviceIsPixmap() ) {
	if ( deleteOldContext )
	    delete oldcx;
	return;
    }

    bool visible = isVisible();
    if ( visible )
	hide();

    //blah
    glcx->setWindowCreated( TRUE );
}


bool QGLWidget::renderCxPm( QPixmap* pm )
{
    return TRUE;
}

#endif
