/****************************************************************************
**
** Implementation of win32 ActiveX server startup routines.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qstringlist.h>

#include "qaxfactory.h"

#include <qt_windows.h>

extern HANDLE qAxInstance;
extern char qAxModuleFilename[MAX_PATH];
extern void qAxInit();
extern void qAxCleanup();
extern HRESULT UpdateRegistry(BOOL bRegister);
#if defined(Q_CC_BOR)
extern "C" __stdcall HRESULT DumpIDL( const QString &outfile, const QString &ver );
#else
STDAPI DumpIDL( const QString &outfile, const QString &ver );
#endif


#if defined(NEEDS_QMAIN)
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
int qMain( int, char ** );
#else
#if defined( Q_OS_TEMP )
extern void __cdecl qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
EXTERN_C int __cdecl main( int, char ** );
#else
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> &);
EXTERN_C int main( int, char ** );
#endif
#endif

EXTERN_C int WINAPI WinMain(HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    GetModuleFileNameA( 0, qAxModuleFilename, MAX_PATH-1 );
    qAxInstance = hInstance;

    lpCmdLine = GetCommandLineA(); //this line necessary for _ATL_MIN_CRT
    QString cmdLine = QString::fromLatin1( lpCmdLine );

    QStringList cmds = QStringList::split( " ", cmdLine );
    int nRet = 0;
    bool run = TRUE;
    bool runServer = FALSE;
    for ( QStringList::Iterator it = cmds.begin(); it != cmds.end(); ++it ) {
	QString cmd = (*it).lower();
	if ( cmd == "-activex" || cmd == "/activex" ) {
	    runServer = TRUE;
	} else if ( cmd == "-unregserver" || cmd == "/unregserver" ) {
	    qWarning( "Unregistering COM objects in %s", cmds[0].latin1() );
 	    nRet = UpdateRegistry(FALSE);
            run = FALSE;
	    break;
	} else if ( cmd == "-regserver" || cmd == "/regserver" ) {
	    qWarning( "Registering COM objects in %s", cmds[0].latin1() );
 	    nRet = UpdateRegistry(TRUE);
            run = FALSE;
            break;
	} else if ( cmd == "-dumpidl" || cmd == "/dumpidl" ) {
	    ++it;
	    if ( it != cmds.end() ) {
		QString outfile = *it;
		++it;
		QString version;
		if ( it != cmds.end() && ( *it == "-version" || *it == "/version" ) ) {
		    ++it;
		    if ( it != cmds.end() )
			version = *it;
		    else
			version = "1.0";
		}

		nRet = DumpIDL( outfile, version );
		switch (nRet) {
		case S_OK:
		    break;
		case -1:
		    qWarning( "Couldn't open %s for writing", (const char*)outfile.local8Bit() );
		    return nRet;
		case 1:
		    qWarning( "Malformed appID value in %s!", qAxModuleFilename );
		    return nRet;
		case 2:
		    qWarning( "Malformed typeLibID value in %s!", qAxModuleFilename );
		    return nRet;
		case 3:
		    qWarning( "Class has no metaobject information (error in %s)!", qAxModuleFilename );
		    return nRet;
		case 4:
		    qWarning( "Malformed classID value in %s!", qAxModuleFilename );
		    return nRet;
		case 5:
		    qWarning( "Malformed interfaceID value in %s!", qAxModuleFilename );
		    return nRet;
		case 6:
		    qWarning( "Malformed eventsID value in %s!", qAxModuleFilename );
		    return nRet;

		default:
		    qFatal( "Unknown error writing IDL from %s", qAxModuleFilename );
		}
	    } else {
		qWarning( "Wrong commandline syntax: <app> -dumpidl <idl file> [-version <x.y.z>]" );
	    }
	    run = FALSE;
	    break;
	}
    }

    if (run) {
	int argc;
	char* cmdp = 0;
	cmdp = new char[ cmdLine.length() + 1 ];
	qstrcpy( cmdp, cmdLine.latin1() );

	QMemArray<pchar> argv( 8 );
	qWinMain( hInstance, hPrevInstance, cmdp, nShowCmd, argc, argv );
	qAxInit();
	if (runServer)
	    QAxFactory::startServer();
	nRet = main( argc, argv.data() );
	QAxFactory::stopServer();
	qAxCleanup();

	delete[] cmdp;
    }

    return nRet;
}


// until such time as mingw runtime calls winmain instead of main 
// in a GUI app we need this.
#if defined(Q_OS_WIN32) && defined(Q_CC_GNU)
#include <qtcrtentrypoint.cpp>
#endif

