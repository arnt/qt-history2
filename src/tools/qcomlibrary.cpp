/****************************************************************************
** $Id$
**
** Implementation of QComLibrary class
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
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

#include "qcomlibrary_p.h"

#ifndef QT_NO_COMPONENT
#include "qapplication.h"
#include "qsettings.h"
#include "qfileinfo.h"
#include "qdatetime.h"

#ifndef QT_DEBUG_COMPONENT
# if defined(QT_DEBUG)
#  define QT_DEBUG_COMPONENT 1
# endif
#endif

#define QT_WARN_PLUGINS

QComLibrary::QComLibrary( const QString &filename )
: QLibrary( filename ), entry( 0 ), libiface( 0 )
{
}

QComLibrary::~QComLibrary()
{
    if ( autoUnload() )
	unload();
}

bool QComLibrary::unload()
{
    if ( libiface ) {
	libiface->cleanup();
	if ( !libiface->canUnload() )
	    return FALSE;
	libiface->release();
	libiface = 0;
    }
    int refs = entry ? entry->release() : 0;
    if ( refs )
	return FALSE;

    entry = 0;

    return QLibrary::unload();
}


static bool verify( const QString& library, uint version, uint flags, const char* key, bool didLoad =
#ifndef QT_DEBUG_PLUGINS
		    FALSE
#else
		    TRUE
#endif
		    )
{
    uint our_flags = 1;
#if defined(QT_THREAD_SUPPORT)
    our_flags |= 2;
#endif

    if ( (flags & 1) == 0 ) {
	if ( didLoad )
	    qWarning( "Conflict in %s:\n Plugin cannot be queried successfully!",
		      (const char*) QFile::encodeName(library) );
    } else if ( qstrcmp( key, QT_BUILD_KEY ) ) {
	if ( didLoad )
	    qWarning( "Conflict in %s:\n Plugin uses incompatible Qt library (expected build key \"%s\", got \"%s\")!",
		      (const char*) QFile::encodeName(library),
		      QT_BUILD_KEY, key ? key : "<null>" );
    } else if ( (version >  QT_VERSION)  ||
	 ( ( QT_VERSION & 0xffff00 ) > ( version & 0xffff00 ) ) ) {
	if ( didLoad )
	    qWarning( "Conflict in %s:\n Plugin uses incompatible Qt library (%d.%d.%d)!", (const char*) QFile::encodeName(library),
		      version&0xff0000,version&0xff00,version&0xff );
    } else if ( (flags & 2) != (our_flags & 2 ) ) {
	if ( didLoad )
	    qWarning( "Conflict in %s:\n Plugin uses %s Qt library!", (const char*) QFile::encodeName(library),
		      (flags & 2) ? "multi threaded" : "single threaded" );
    } else {
	return TRUE;
    }
    return FALSE;
}

void QComLibrary::createInstanceInternal()
{
    if ( library().isEmpty() )
	return;

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    QString regkey = QString("/qt_plugin%1.%2/%3").arg( QT_MAJOR_VERSION ).arg( QT_MINOR_VERSION ).arg( library() );
    QStringList reg;

    uint version, flags;
    const char* key = 0;
    QFileInfo fileinfo( library() );
    QString lastModified  = fileinfo.lastModified().toString();

    reg =  settings.readListEntry( regkey );
    if ( reg.count() == 4 ) {
	version = reg[0].toUInt(0, 16);
	flags = reg[1].toUInt(0, 16);
	key = reg[2].latin1();
	// check timestamp
	if ( lastModified == reg[3] &&
	     !verify( library(), version, flags, key ) )
	    return;
    }

    if ( !isLoaded() ) {
	Q_ASSERT( entry == 0 );
	if ( !load() )
	    return;
    }
    if ( isLoaded() && !entry ) {
#  ifdef Q_CC_BOR
	typedef int __stdcall (*UCMQueryProc)(uint*, uint*, const char** );
#  else
	typedef int (*UCMQueryProc)(uint*, uint*, const char** );
#  endif
	UCMQueryProc ucmQueryProc;
	ucmQueryProc = (UCMQueryProc) resolve( "qt_ucm_query" );
	if ( !ucmQueryProc  || ucmQueryProc( &version, &flags, &key ) != 0 ) {
	    version = 0;
	    flags = 0;
	    key = "unknown";
	}
	QStringList queried;
	queried << QString::number( version,16 ) << QString::number( flags, 16 ) << QString::fromLatin1( key ) << lastModified;
	if ( queried != reg )
	    settings.writeEntry( regkey, queried );
	if ( !verify( library(), version, flags, key ,TRUE ) ) {
	    unload();
	    return;
	}
    }

#ifdef Q_CC_BOR
    typedef QUnknownInterface* __stdcall (*UCMInstanceProc)();
#else
    typedef QUnknownInterface* (*UCMInstanceProc)();
#endif
    UCMInstanceProc ucmInstanceProc;
    ucmInstanceProc = (UCMInstanceProc) resolve( "ucm_instantiate" );
#if defined(QT_DEBUG_COMPONENT)
    if ( !ucmInstanceProc )
	qWarning( "%s: Not a UCOM library.", library().latin1() );
#endif
    entry = ucmInstanceProc ? ucmInstanceProc() : 0;

    if ( entry ) {
	if ( entry->queryInterface( IID_QLibrary, (QUnknownInterface**)&libiface ) == QS_OK ) {
	    if ( libiface && !libiface->init() ) {
		libiface->release();
		libiface = 0;
		unload();
		return;
	    }
	}
    } else {
#if defined(QT_DEBUG_COMPONENT)
	qWarning( "%s: No exported component provided.", library().latin1() );
#endif
	unload();
    }
}

QRESULT QComLibrary::queryInterface( const QUuid& request, QUnknownInterface** iface )
{
    if ( !entry ) {
	createInstanceInternal();
    }

    return entry ? entry->queryInterface( request, iface ) : QE_NOCOMPONENT;
}

#endif // QT_NO_COMPONENT
