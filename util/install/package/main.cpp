#include "qarchive.h"
#include <qapplication.h>
#include <qfileinfo.h>
#include "keyinfo.h"
#if defined(Q_OS_WIN32)
#include <windows.h>
#endif

class ConsoleOutput : public QObject
{
    Q_OBJECT
public:
    ConsoleOutput() : QObject() { }
    ~ConsoleOutput() { }
public slots:
    void updateProgress( const QString& str) {	qDebug("%s", str.latin1());   }
};

static int usage(const char *argv0, const char *un=NULL) {
    if(un)
	fprintf(stderr, "Unknown command: %s\n", un);
    else
	fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [options] [keyinfo] files...\n", argv0);

    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, " -o file     : Outputs archive to file\n");
    fprintf(stderr, " -n          : Follow symbolic links, do not archive the link\n");
    fprintf(stderr, " -s          : Quiet mode, will not output process\n");
    fprintf(stderr, " -desc text  : Add the description text to the archive (e.g. \"Qt 3.0.0\")\n");
    fprintf(stderr, " -extra k v  : Adds the extra value v with the key k to the archive\n");
    fprintf(stderr, " -h          : This help\n");

    fprintf(stderr, "\nKey Info:\n");
    fprintf(stderr, " -win        : Windows Archive\n");
    fprintf(stderr, " -unix       : Unix Archive\n");
    fprintf(stderr, " -mac        : MacOSX Archive\n");
    fprintf(stderr, " -embedded   : Embedded Archive\n");
#if defined(Q_OS_WIN32)
    fprintf(stderr, "\nOr:\n\n", argv0);
    fprintf(stderr, "%s -res qt.arq install.exe\n\n", argv0);
    fprintf(stderr, " -res        : Add qt.arq as the binary resource\n");
    fprintf(stderr, "               QT_ARQ to install.exe\n");
#endif
    return 665;
}

int main( int argc, char** argv )
{
    uint features = 0;
    bool output = TRUE, doSyms = TRUE;
    QString desc;
    QString dest;
    QMap<QString,QString> extra;
#if defined(Q_OS_WIN32)
    QString arq, exe;
    bool doRes = FALSE;
    bool getRes = FALSE;
#endif
    QStringList files;
    for(int i = 1; i < argc; i++) {
	//options
	if(!strcmp(argv[i], "-o")) {
	    if ( ++i < argc )
		dest = argv[i];
	    else
		return usage(argv[0]);
	} else if(!strcmp(argv[i], "-n")) {
	    doSyms = FALSE;
	} else if(!strcmp(argv[i], "-s")) {
	    output = FALSE;
	} else if(!strcmp(argv[i], "-desc")) {
	    if ( ++i < argc )
		desc = argv[i];
	    else
		return usage(argv[0]);
	} else if(!strcmp(argv[i], "-extra")) {
	    QString key, value;
	    if ( ++i < argc )
		key = argv[i];
	    else
		return usage(argv[0]);
	    if ( ++i < argc )
		value = argv[i];
	    else
		return usage(argv[0]);
	    extra.insert( key, value );
	} else if(!strcmp(argv[i], "-h")) {
	    return usage(argv[0]);
	//keyinfo
	} else if(!strcmp(argv[i], "-unix")) {
	    features |= Feature_Unix;
	} else if(!strcmp(argv[i], "-win")) {
	    features |= Feature_Windows;
	} else if(!strcmp(argv[i], "-mac")) {
	    features |= Feature_Mac;
	} else if(!strcmp(argv[i], "-embedded")) {
	    features |= Feature_Embedded;
#if defined(Q_OS_WIN32)
	//res (Windows only)
	} else if(!strcmp(argv[i], "-res")) {
	    doRes = TRUE;
	    if ( ++i < argc )
		arq = argv[i];
	    if ( ++i < argc )
		exe = argv[i];
	//res (Windows only)
	} else if(!strcmp(argv[i], "-getres")) {
	    getRes = TRUE;
	    if ( ++i < argc )
		exe = argv[i];
#endif
	//files
	} else if(*(argv[i]) != '-') {
	    files.append(argv[i]); 
	//unknown
	} else {
	    return usage(argv[0], argv[i]); 
	}
    }
#if defined(Q_OS_WIN32)
    if ( doRes ) {
	if ( arq.isEmpty() || exe.isEmpty() )
	    return usage(argv[0], argv[i]); 
	QFile fArq( arq );
	if ( !fArq.open( IO_ReadOnly ) ) {
	    fprintf(stderr, "Could not open archive %s", arq.latin1() );
	    return -1;
	}
	QByteArray ba = fArq.readAll();
	// ignore wide character versions (this is for internal use only)
	HANDLE hExe = BeginUpdateResourceA( exe.latin1(), FALSE );
	if ( hExe == 0 ) {
	    fprintf(stderr, "Could not load executable %s\n", exe.latin1() );
	    qSystemWarning( "" );
	    return -1;
	}
	if ( !UpdateResourceA(hExe,RT_RCDATA,"QT_ARQ",0,ba.data(),ba.count()) ) {
	    EndUpdateResource( hExe, TRUE );
	    fprintf(stderr, "Could not update executable %s\n", exe.latin1() );
	    qSystemWarning( "" );
	    return -1;
	}
	if ( !EndUpdateResource(hExe,FALSE) ) {
	    fprintf(stderr, "Could not update executable %s\n", exe.latin1() );
	    qSystemWarning( "" );
	    return -1;
	}
	return 0;
    }
    if ( getRes ) {
	if ( exe.isEmpty() )
	    return usage(argv[0], argv[i]); 
	arq = "qt.arq";
	QFile fArq( arq );
	if ( !fArq.open( IO_WriteOnly ) ) {
	    fprintf(stderr, "Could not open archive %s\n", arq.latin1() );
	    return -1;
	}
	// ignore wide character versions (this is for internal use only)
	HMODULE hExe = LoadLibraryA( exe.latin1() );
	if ( hExe == NULL ) {
	    fprintf(stderr, "Could not load executable %s\n", exe.latin1() );
	    qSystemWarning( "" );
	    return -1;
	}
	HRSRC resource = FindResource( hExe, "QT_ARQ", RT_RCDATA );
	HGLOBAL hglobal = LoadResource( hExe, resource );
	int arSize = SizeofResource( hExe, resource );
	if ( arSize == 0 ) {
	    fprintf(stderr, "Could not get size of resource\n" );
	    qSystemWarning( "" );
	    return -1;
	}
	char *arData = (char*)LockResource( hglobal );
	if ( arData == 0 ) {
	    fprintf(stderr, "Could not lock resource\n" );
	    qSystemWarning( "" );
	    return -1;
	}
	fArq.writeBlock( arData, arSize );
	FreeLibrary( hExe );
	return 0;
    }
#endif
    if(!files.isEmpty()) {
	if(dest.isEmpty()) {
	    qDebug("Please specify an output package");
	    return 666;
	}

	QArchive archive;
	ConsoleOutput out;
	if(output) {
	    QObject::connect( &archive, SIGNAL( operationFeedback( const QString& ) ), 
		    &out, SLOT( updateProgress( const QString& ) ) );
	    archive.setVerbosity( QArchive::Destination | QArchive::Verbose );
	}
	archive.setSymbolicLinks(doSyms);
	archive.setPath( dest );
	if( !archive.open( IO_WriteOnly ) ) {
	    qDebug("Failed to open output %s", dest.latin1());
	    return 666;
	}
	QArchiveHeader header( features, desc );
	QMap<QString,QString>::Iterator exIt;
	for ( exIt = extra.begin(); exIt != extra.end(); ++exIt ) {
	    header.addExtraData( exIt.key(), exIt.data() );
	}
	archive.writeHeader( header );
	for(QStringList::Iterator it = files.begin(); it != files.end(); ++it) {
	    QFileInfo f((*it));
	    if(!f.exists()) {
		qDebug("Failed to open %s", (*it).latin1());
		continue;
	    }
	    if(f.isDir()) 
		archive.writeDir( (*it), TRUE, (*it) );
	    else
		archive.writeFile( (*it), (*it) );
	}
	archive.close();
    } else {
	return usage(argv[0]);
    }
    return 0;
}

#include "main.moc"
