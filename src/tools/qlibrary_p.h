/****************************************************************************
** $Id: $
**
** Definition of an internal QLibrary class
**
** Created : 2000-01-01
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#ifndef QLIBRARY_P_H
#define QLIBRARY_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_NO_COMPONENT
#include "qlibrary.h"

#ifndef QT_H
#include "qwindowdefs.h"
#ifndef QT_LITE_COMPONENT
#include "qtimer.h"
#endif
#endif // QT_H

//#define QT_DEBUG_COMPONENT 1

/*
  Private helper class that saves the platform dependent handle
  and does the unload magic using a QTimer.
*/
#ifndef QT_LITE_COMPONENT
class QLibraryPrivate : public QObject
{
    Q_OBJECT
public:
    QLibraryPrivate( QLibrary *lib )
	: QObject( 0, lib->library().latin1() ), pHnd( 0 ), libIface( 0 ), unloadTimer( 0 ), library( lib )
    {}

    ~QLibraryPrivate()
    {
	if ( libIface )
	    libIface->release();
	killTimer();
    }
 
    void startTimer()
    {
	unloadTimer = new QTimer( this );
	connect( unloadTimer, SIGNAL( timeout() ), this, SLOT( tryUnload() ) );
	unloadTimer->start( 5000, FALSE );
    }

    void killTimer()
    {
	delete unloadTimer;
	unloadTimer = 0;
    }

#ifdef Q_WS_WIN
    HINSTANCE pHnd;
#else
    void *pHnd;
#endif

    QLibraryInterface *libIface;

    bool loadLibrary();
    bool freeLibrary();
    void *resolveSymbol( const char * );

public slots:
    /*
      Only components that implement the QLibraryInterface can
      be unloaded automatically.
    */
    void tryUnload()
    {
	if ( library->policy() == QLibrary::Manual || !pHnd || !libIface )
	    return;

	if ( !libIface->canUnload() )
	    return;

#if QT_DEBUG_COMPONENT == 1
	if ( library->unload() )
	    qDebug( "%s has been automatically unloaded", library->library().latin1() );
#else
	library->unload();
#endif
    }

private:
    QTimer *unloadTimer;
    QLibrary *library;
};

#else // QT_LITE_COMPONENT
class QLibraryPrivate
{
public:
    QLibraryPrivate( QLibrary *lib )
	: pHnd( 0 ), libIface( 0 ), library( lib )
    {}

    void startTimer()
    {
    }

    void killTimer()
    {
    }

#ifdef Q_WS_WIN
    HINSTANCE pHnd;
#else
    void *pHnd;
#endif
    QLibraryInterface *libIface;

    bool loadLibrary();
    bool freeLibrary();
    void *resolveSymbol( const char * );

private:
    QLibrary *library;
};
#endif // QT_LITE_COMPONENT

#endif // QT_NO_COMPONENT

#endif // QLIBRARY_P_H
