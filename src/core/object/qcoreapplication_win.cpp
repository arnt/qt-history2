#include "qcoreapplication.h"
#include "qt_windows.h"
#include "qvector.h"
#include "qmutex.h"


// ############### DONT EXPORT HERE!!!
Q_CORE_EXPORT char	 appFileName[256];		// application file name
Q_CORE_EXPORT char	 appName[256];			// application name
Q_CORE_EXPORT HINSTANCE appInst	= 0;		// handle to app instance
Q_CORE_EXPORT HINSTANCE appPrevInst	= 0;		// handle to prev app instance
Q_CORE_EXPORT int appCmdShow = 0;

Q_CORE_EXPORT HINSTANCE qWinAppInst()		// get Windows app handle
{
    return appInst;
}

Q_CORE_EXPORT HINSTANCE qWinAppPrevInst()		// get Windows prev app handle
{
    return appPrevInst;
}

Q_CORE_EXPORT bool qt_winEventFilter(MSG* msg)
{
    return QCoreApplication::instance()->winEventFilter( msg );
}

static void	msgHandler( QtMsgType, const char* );

void set_winapp_name()
{
    static bool already_set = FALSE;
    if ( !already_set ) {
	already_set = TRUE;
#ifndef Q_OS_TEMP
	GetModuleFileNameA( 0, appFileName, sizeof(appFileName) );
#else
	QString afm;
	afm.setLength( 256 );
	afm.setLength( GetModuleFileName( 0, (unsigned short*)afm.unicode(), 255 ) );
	strncpy( appFileName, afm.latin1(), afm.length() );
#endif
	const char *p = strrchr( appFileName, '\\' );	// skip path
	if ( p )
	    memcpy( appName, p+1, qstrlen(p) );
	int l = qstrlen( appName );
	if ( (l > 4) && !qstricmp( appName + l - 4, ".exe" ) )
	    appName[l-4] = '\0';		// drop .exe extension
    }
}

Q_CORE_EXPORT const char *qAppFileName()		// get application file name
{
    return appFileName;
}

Q_CORE_EXPORT const char *qAppName()			// get application name
{
    if ( !appName[0] )
	set_winapp_name();
    return appName;
}


#if defined(Q_CC_MSVC) && !defined(Q_OS_TEMP)
#include <crtdbg.h>
#endif

static void msgHandler( QtMsgType t, const char* str )
{
#if defined(QT_THREAD_SUPPORT)
    // OutputDebugString is not threadsafe.
    static QMutex staticMutex;
#endif

    if ( !str )
	str = "(null)";

#if defined(QT_THREAD_SUPPORT)
    staticMutex.lock();
#endif
    QT_WA( {
	QString s(str);
	s += "\n";
	OutputDebugStringW( (TCHAR*)s.ucs2() );
    }, {
	QByteArray s(str);
	s += "\n";
	OutputDebugStringA( s.data() );
    } )
#if defined(QT_THREAD_SUPPORT)
    staticMutex.unlock();
#endif
    if ( t == QtFatalMsg )
#ifndef Q_OS_TEMP
#if defined(Q_CC_MSVC) && defined(_DEBUG) && defined(_CRT_ERROR)
	_CrtDbgReport( _CRT_ERROR, __FILE__, __LINE__, QT_VERSION_STR, str );
#else
	ExitProcess( 1 );
#endif
#else
	exit(1);
#endif
}


/*****************************************************************************
  qWinMain() - Initializes Windows. Called from WinMain() in qtmain_win.cpp
 *****************************************************************************/

#if defined( Q_OS_TEMP )
Q_CORE_EXPORT void __cdecl qWinMain( HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdParam,
	       int cmdShow, int &argc, QVector<pchar> &argv )
#else
Q_CORE_EXPORT
void qWinMain( HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdParam,
	       int cmdShow, int &argc, QVector<pchar> &argv )
#endif
{
    static bool already_called = FALSE;

    if ( already_called ) {
	qWarning( "Qt internal error: qWinMain should be called only once" );
	return;
    }
    already_called = TRUE;

  // Install default debug handler

    qInstallMsgHandler( msgHandler );

  // Create command line

    set_winapp_name();

    char *p = cmdParam;
    char *p_end = p + strlen(p);

    argc = 1;
    argv[0] = appFileName;

    while ( *p && p < p_end ) {				// parse cmd line arguments
	while ( isspace((uchar) *p) )			// skip white space
	    p++;
	if ( *p && p < p_end ) {				// arg starts
	    int quote;
	    char *start, *r;
	    if ( *p == '\"' || *p == '\'' ) {	// " or ' quote
		quote = *p;
		start = ++p;
	    } else {
		quote = 0;
		start = p;
	    }
	    r = start;
	    while ( *p && p < p_end ) {
		if ( *p == '\\' ) {		// escape char?
		    p++;
		    if ( *p == '\"' || *p == '\'' )
			;			// yes
		    else
			p--;			// treat \ literally
		} else if ( quote ) {
		    if ( *p == quote ) {
			p++;
			if ( isspace((uchar) *p) )
			    break;
			quote = 0;
		    }
		} else {
		    if ( *p == '\"' || *p == '\'' ) {	// " or ' quote
			quote = *p++;
			continue;
		    } else if ( isspace((uchar) *p) )
			break;
		}
		if ( p )
		    *r++ = *p++;
	    }
	    if ( *p && p < p_end )
		p++;
	    *r = '\0';

	    if ( argc >= (int)argv.size()-1 )	// expand array
		argv.resize( argv.size()*2 );
	    argv[argc++] = start;
	}
    }
    argv[argc] = 0;
  // Get Windows parameters

    appInst = instance;
    appPrevInst = prevInstance;
    appCmdShow = cmdShow;

#ifdef Q_OS_TEMP
    TCHAR uniqueAppID[256];
    GetModuleFileName( 0, uniqueAppID, 255 );
    appUniqueID = RegisterWindowMessage(
		  QString::fromUcs2(uniqueAppID)
		  .lower().remove('\\').ucs2() );
#endif
}

/*!
    The message procedure calls this function for every message
    received. Reimplement this function if you want to process window
    messages \e msg that are not processed by Qt. If you don't want
    the event to be processed by Qt, then return TRUE; otherwise
    return FALSE.
*/
bool QCoreApplication::winEventFilter( MSG * /*msg*/ )	// Windows event filter
{
    return FALSE;
}
