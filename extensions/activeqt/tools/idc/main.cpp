#include <qfile.h>

#include <qt_windows.h>
#include <io.h>

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
    if ( !hdll ) {
	fprintf(stderr, "Couldn't load library file %s\n", (const char*)input.local8Bit());
	return FALSE;
    }
    typedef HRESULT(__stdcall* RegServerProc)();
    RegServerProc DllRegisterServer = (RegServerProc)GetProcAddress( hdll, "DllRegisterServer" );
    if ( !DllRegisterServer ) {
	fprintf(stderr, "Library file %s doesn't appear to be a COM library\n", (const char*)input.local8Bit());
	return FALSE;
    }
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
    if ( !hdll ) {
	fprintf(stderr, "Couldn't load library file %s\n", (const char*)input.local8Bit());
	return FALSE;
    }
    typedef HRESULT(__stdcall* RegServerProc)();
    RegServerProc DllUnregisterServer = (RegServerProc)GetProcAddress( hdll, "DllUnregisterServer" );
    if ( !DllUnregisterServer ) {
	fprintf(stderr, "Library file %s doesn't appear to be a COM library\n", (const char*)input.local8Bit());
	return FALSE;
    }
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
		fprintf(stderr, "IDC requires Windows NT/2000/XP!\n");

	    ++i;
	    if ( i > argc ) {
		error = "Missing name for type library file!";
		break;
	    }
	    tlbfile = argv[i];
	    tlbfile = tlbfile.stripWhiteSpace().lower();
	} else if ( p == "/v" || p == "-v" ) {
	    fprintf(stdout, "Qt interface definition compiler version 1.0\n");
	    return 0;
	} else if ( p == "/regserver" || p == "-regserver" ) {
	    if ( !registerServer( input ) ) {
		fprintf(stderr, "Failed to register server!\n");
		return 1;
	    }
	    return 0;
	} else if ( p == "/unregserver" || p == "-unregserver" ) {
	    if ( !unregisterServer( input ) ) {
		fprintf(stderr, "Failed to unregister server!\n");
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
	fprintf(stderr, error.latin1() );
	fprintf(stderr, "\n" );
	return 5;
    }
    if ( input.isEmpty() ) {
	fprintf(stderr, "No input file specified!\n");
	return 1;
    }
    if ( input.endsWith( ".exe" ) && tlbfile.isEmpty() ) {
	fprintf(stderr, "No type library file specified!\n");
	return 2;
    }
    if ( input.endsWith( ".dll" ) && idlfile.isEmpty() && tlbfile.isEmpty() ) {
	fprintf(stderr, "No interface definition file and no type library file specified!\n");
	return 3;
    }
    slashify( input );
    if ( tlbfile ) {
	slashify( tlbfile );
	QFile file( tlbfile );
	if ( !file.open( IO_ReadOnly ) )
	    fprintf(stderr, "Couldn't open %s for read\n", (const char*)tlbfile.local8Bit());
	QByteArray data = file.readAll();
	QString error;
	bool ok = attachTypeLibrary( input, 1, data, &error );
	fprintf(stderr, error.latin1());
	fprintf(stderr, "\n");
	return ok ? 0 : 4;
    } else if ( idlfile ) {
	slashify( idlfile );
	HMODULE hdll = 0;
	QT_WA( {
	    hdll = LoadLibraryW( (TCHAR*)input.ucs2() );
	}, {
	    hdll = LoadLibraryA( input.local8Bit() );
	} );
	if ( !hdll ) {
	    fprintf(stderr, "Couldn't load library file %s\n", (const char*)input.local8Bit());
	    return 3;
	}
	typedef HRESULT(__stdcall* DumpIDLProc)(const QString&, const QString&);
	DumpIDLProc DumpIDL = (DumpIDLProc)GetProcAddress( hdll, "DumpIDL" );
	if ( !DumpIDL ) {
	    fprintf(stderr, "Couldn't resolve 'DumpIDL' symbol in %s\n", (const char*)input.local8Bit());
	    return 3;
	}
	HRESULT res = DumpIDL( idlfile, version );
	FreeLibrary( hdll );
	switch(res) {
	case S_OK:
	    break;
	case -1:
	    fprintf(stderr, "Couldn't open %s for writing!\n", (const char*)idlfile.local8Bit());
	    return res;
	case 1:
	    fprintf(stderr, "Malformed appID value in %s!\n", (const char*)input.local8Bit());
	    return res;
	case 2:
	    fprintf(stderr, "Malformed typeLibID value in %s!\n", (const char*)input.local8Bit());
	    return res;
	case 3:
	    fprintf(stderr, "Class has no metaobject information (error in %s)!\n", (const char*)input.local8Bit());
	    return res;
	case 4:
	    fprintf(stderr, "Malformed classID value in %s!\n", (const char*)input.local8Bit());
	    return res;
	case 5:
	    fprintf(stderr, "Malformed interfaceID value in %s!\n", (const char*)input.local8Bit());
	    return res;
	case 6:
	    fprintf(stderr, "Malformed eventsID value in %s!\n", (const char*)input.local8Bit());
	    return res;

	default:
	    fprintf(stderr, "Unknown error writing IDL from %s\n", (const char*)input.local8Bit());
	    return 7;
	}
    }
    return 0;
}
