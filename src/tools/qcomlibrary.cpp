#include "qcomlibrary_p.h"
#include "qapplication.h"

#ifndef QT_DEBUG_COMPONENT
# if defined(QT_DEBUG)
#  define QT_DEBUG_COMPONENT 1
# endif
#endif

QComLibrary::QComLibrary( const QString &filename, QLibrary::Policy pol )
: QLibrary( filename, pol ), entry( 0 ), libiface( 0 )
{
}

QComLibrary::~QComLibrary()
{
    if ( policy() != Manual )
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

void QComLibrary::createInstanceInternal()
{
    if ( library().isEmpty() )
	return;

    if ( !isLoaded() ) {
	Q_ASSERT( entry == 0 );
	load();
    }

    if ( isLoaded() && !entry ) {
#if defined(QT_DEBUG_COMPONENT) && QT_DEBUG_COMPONENT == 2
	qWarning( "%s has been loaded.", library().latin1() );
#endif
#ifndef QT_LITE_COMPONENT
#  ifdef Q_CC_BOR
	typedef int __stdcall (*UCMInitProc)(QApplication*, bool*, bool* );
#  else
	typedef int (*UCMInitProc)(QApplication*, bool*, bool* );
#  endif
#else
#  ifdef Q_CC_BOR
	typedef int __stdcall (*UCMInitProc)(void*, bool*, bool* );
#  else
	typedef int (*UCMInitProc)(void*, bool*, bool* );
#  endif
#endif
	UCMInitProc ucmInitProc;
	ucmInitProc = (UCMInitProc) resolve( "ucm_initialize" );

	bool ucm_init = TRUE;
	if ( ucmInitProc ) {
	    bool plugQtThreaded;
	    bool plugQtDebug;
#ifndef QT_LITE_COMPONENT
	    int plugQtVersion = ucmInitProc( qApp, &plugQtThreaded, &plugQtDebug );
#else
	    int plugQtVersion = ucmInitProc( 0, &plugQtThreaded, &plugQtDebug );
#endif
	    if ( QABS(plugQtVersion - QT_VERSION ) > 99 ) {
#if defined(QT_DEBUG_COMPONENT)
		qWarning( "Conflict in %s: Plugin links against incompatible Qt library (%d)!", library().latin1(), plugQtVersion );
#endif
		ucm_init = FALSE;
	    }
	    if ( plugQtThreaded != QT_THREADED_BUILD ) {
#if defined(QT_DEBUG_COMPONENT)
		qWarning( "Conflict in %s: Plugin uses %s Qt library!", library().latin1(), plugQtThreaded ? "multi threaded" : "single threaded" );
#endif
		// the plugin is threaded, but the application is not. If we live long enough to cancel the load, do it...
		if ( plugQtThreaded )
		    ucm_init = FALSE;
	    }
#if defined(QT_DEBUG_COMPONENT)
	    if ( plugQtDebug != QT_DEBUG_BUILD )
		qWarning( "Possible conflict in %s: Plugin %s debug symbols!", library().latin1(), plugQtDebug ? "has" : "has no" );
#endif
	}
	if ( !ucm_init ) {
	    unload();
	    return;
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
}

QRESULT QComLibrary::queryInterface( const QUuid& request, QUnknownInterface** iface )
{
    if ( !entry ) {
	createInstanceInternal();
    }

    return entry ? entry->queryInterface( request, iface ) : QE_NOCOMPONENT;
}
