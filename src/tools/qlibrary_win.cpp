/****************************************************************************
** $Id: $
**
** Implementation of QLibraryPrivate class
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

#include "private/qlibrary_p.h"

#ifndef QT_NO_COMPONENT

#ifndef QT_H
#include "qwindowdefs.h"
#include "qfile.h"
#endif // QT_H

/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/

#include "qt_windows.h"

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
	return TRUE;

    QString filename = library->library();

#ifdef Q_OS_TEMP
	pHnd = LoadLibraryW( (TCHAR*)qt_winTchar( filename, TRUE) );
#else
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	pHnd = LoadLibraryW( (TCHAR*)qt_winTchar( filename, TRUE) );
    else
#endif
	pHnd = LoadLibraryA(QFile::encodeName( filename ).data());
#endif
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !pHnd )
	qSystemWarning( QString("Failed to load library %1!").arg( filename ) );
#endif

    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
	return TRUE;
    bool ok = FreeLibrary( pHnd );
    if ( ok )
	pHnd = 0;
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    else
	qSystemWarning( "Failed to unload library!" );
#endif

    return ok;
}

void* QLibraryPrivate::resolveSymbol( const char* f )
{
    if ( !pHnd )
	return 0;

#ifdef Q_OS_TEMP
    void* address = GetProcAddress( pHnd, (TCHAR*)qt_winTchar( f, TRUE) );
#else
    void* address = GetProcAddress( pHnd, f );
#endif
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if ( !address )
	qSystemWarning( QString("Couldn't resolve symbol \"%1\"").arg( f ) );
#endif

    return address;
}

#endif // QT_NO_COMPONENT
