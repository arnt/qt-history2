#include <qfile.h>

#include <qt_windows.h>

static bool attachTypeLibrary( const QString &applicationName, int resource, const QByteArray &data, QString *errorMessage )
{
    HANDLE hExe = 0;
    QT_WA( {
	TCHAR *resourceName = MAKEINTRESOURCEW(resource);
	hExe = BeginUpdateResourceW( (TCHAR*)applicationName.ucs2(), FALSE );
	if ( hExe == 0 ) {
	    if ( errorMessage )
		*errorMessage = QString("Failed to attach type library to binary %1 - could not open file.").arg(applicationName);
	    return FALSE;
	}
	if ( !UpdateResourceW(hExe,L"TYPELIB",resourceName,0,data.data(),data.count()) ) {
	    EndUpdateResource( hExe, TRUE );
	    if ( errorMessage )
		*errorMessage = QString("Failed to attach type library to binary %1 - could not update file.").arg(applicationName);
	    return FALSE;
	}
    }, {
	char *resourceName = MAKEINTRESOURCEA(resource);
	hExe = BeginUpdateResourceA( applicationName.local8Bit(), FALSE );
	if ( hExe == 0 ) {
	    if ( errorMessage )
		*errorMessage = QString("Failed to attach type library to binary %1 - could not open file.").arg(applicationName);
	    return FALSE;
	}
	if ( !UpdateResourceA(hExe,"TYPELIB",resourceName,0,data.data(),data.count()) ) {
	    EndUpdateResource( hExe, TRUE );
	    if ( errorMessage )
		*errorMessage = QString("Failed to attach type library to binary %1 - could not update file.").arg(applicationName);
	    return FALSE;
	}
    } );

    if ( !EndUpdateResource(hExe,FALSE) ) {
	if ( errorMessage )
	    *errorMessage = QString("Failed to attach type library to binary %1 - could not write file.").arg(applicationName);
	return FALSE;
    }

    if ( errorMessage )
	*errorMessage = QString("Type library attached to %1.").arg(applicationName);
    return TRUE;
}

static bool registerServer( const QString &input )
{
    HMODULE hdll = 0;
    QT_WA( {
	hdll = LoadLibraryW( (TCHAR*)input.ucs2() );
    }, {
	hdll = LoadLibraryA( input.local8Bit() );
    } );
    if ( !hdll )
	return FALSE;
    typedef HRESULT(* RegServerProc)();
    RegServerProc DllRegisterServer = (RegServerProc)GetProcAddress( hdll, "DllRegisterServer" );
    if ( !DllRegisterServer )
	return FALSE;
    return DllRegisterServer() == S_OK;
}

static bool unregisterServer( const QString &input )
{
    HMODULE hdll = 0;
    QT_WA( {
	hdll = LoadLibraryW( (TCHAR*)input.ucs2() );
    }, {
	hdll = LoadLibraryA( input.local8Bit() );
    } );
    if ( !hdll )
	return FALSE;
    typedef HRESULT(* RegServerProc)();
    RegServerProc DllUnregisterServer = (RegServerProc)GetProcAddress( hdll, "DllUnregisterServer" );
    if ( !DllUnregisterServer )
	return FALSE;
    return DllUnregisterServer() == S_OK;
}

static void slashify( QString &s )
{
    if ( !s.contains( '/' ) )
	return;

    int i = 0;
    while ( i < (int)s.length() ) {
	if ( s[i] == '/' )
	    s[i] = '\\';
	++i;
    }
}

int main( int argc, char **argv )
{
    QString error;
    QString tlbfile;
    QString idlfile;
    QString input;
    QString version = "1.0";

    int i = 1;
    while ( i < argc ) {
	QCString p = QCString(argv[i]).lower();

	if ( p == "/idl" || p == "-idl" ) {
	    ++i;
	    if ( i > argc ) {
		error = "Missing name for interface definition file!";
		break;
	    }
	    idlfile = argv[i];
	    idlfile = idlfile.stripWhiteSpace().lower();
	} else if ( p == "/version" || p == "-version" ) {
	    ++i;
	    if ( i > argc )
		version = "1.0";
	    else
		version = argv[i];
	} else if ( p == "/tlb" || p == "-tlb" ) {
	    if ( qWinVersion() & Qt::WV_DOS_based )
		qFatal( "IDC requires Windows NT/2000/XP!" );

	    ++i;
	    if ( i > argc ) {
		error = "Missing name for type library file!";
		break;
	    }
	    tlbfile = argv[i];
	    tlbfile = tlbfile.stripWhiteSpace().lower();
	} else if ( p == "/v" || p == "-v" ) {
	    qWarning( "Qt interface definition compiler version 1.0" );
	    return 0;
	} else if ( p == "/regserver" || p == "-regserver" ) {
	    if ( !registerServer( input ) ) {
		error = "Failed to register server!";
		return 1;
	    }
	    return 0;
	} else if ( p == "/unregserver" || p == "-unregserver" ) {
	    if ( !unregisterServer( input ) ) {
		error = "Failed to unregister server!";
		return 1;
	    }
	    return 0;
	} else if ( p[0] == '/' || p[0] == '-' ) {
	    error = "Unknown option \"" + p + "\"";
	    break;
	} else {
	    input = argv[i];
	    input = input.stripWhiteSpace().lower();
	}
	i++;
    }
    if ( !error.isEmpty() ) {
	qFatal( error.latin1() );
	return -1;
    }
    if ( input.isEmpty() ) {
	qFatal( "No input file specified!" );
	return 1;
    }
    if ( input.endsWith( ".exe" ) && tlbfile.isEmpty() ) {
	qFatal( "No type library file specified!" );
	return 2;
    }
    if ( input.endsWith( ".dll" ) && idlfile.isEmpty() && tlbfile.isEmpty() ) {
	qFatal( "No interface definition file and no type library file specified!" );
	return 3;
    }
    slashify( input );
    if ( tlbfile ) {
	slashify( tlbfile );
	QFile file( tlbfile );
	if ( !file.open( IO_ReadOnly ) )
	    qFatal( "Couldn't open %s for read", (const char*)tlbfile.local8Bit() );
	QByteArray data = file.readAll();
	QString error;
	bool ok = attachTypeLibrary( input, 1, data, &error );
	qWarning( error );
	return ok ? 0 : -1;
    } else if ( idlfile ) {
	slashify( idlfile );
	HMODULE hdll = 0;
	QT_WA( {
	    hdll = LoadLibraryW( (TCHAR*)input.ucs2() );
	}, {
	    hdll = LoadLibraryA( input.local8Bit() );
	} );
	if ( !hdll ) {
	    qFatal( "Couldn't load library file %s", (const char*)input.local8Bit() );
	    return 3;
	}
	typedef HRESULT(* DumpIDLProc)(const QString&, const QString&);
	DumpIDLProc DumpIDL = (DumpIDLProc)GetProcAddress( hdll, "DumpIDL" );
	if ( !DumpIDL ) {
	    qFatal( "Couldn't resolve 'DumpIDL' symbol in %s", (const char*)input.local8Bit() );
	    return 3;
	}
	HRESULT res = DumpIDL( idlfile, version );
	FreeLibrary( hdll );
	if ( res != S_OK )
	    qFatal( "Error writing IDL from %s", (const char*)input.local8Bit() );
    }
    return 0;
}
