#include "qarchive.h"
#include <qapplication.h>
#include <qfileinfo.h>
#include "generatordlgimpl.h"
#include "keyinfo.h"

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
    fprintf(stderr, " -o file    : Outputs archive to file\n");
    fprintf(stderr, " -n         : Follow symbolic links, do not archive the link\n");
    fprintf(stderr, " -s         : Quiet mode, will not output process\n");
    fprintf(stderr, " -h         : This help\n");

    fprintf(stderr, "\nKey Info:\n");
    fprintf(stderr, " -us         : Enable US\n");
    fprintf(stderr, " -enterprise : Enable Enterprise Package\n");
    fprintf(stderr, " -win        : Windows Archive\n");
    fprintf(stderr, " -unix       : Unix Archive\n");
    fprintf(stderr, " -mac        : MacOSX Archive\n");
    fprintf(stderr, " -embedded   : Embedded Archive\n");
    return 665;
}

int main( int argc, char** argv )
{
    uint features = 0;
    bool output = TRUE, doSyms = TRUE;
    QString dest, key;
    QStringList files;
    for(int i = 1; i < argc; i++) {
	//options
	if(!strcmp(argv[i], "-o")) 
	    dest = argv[++i];
	else if(!strcmp(argv[i], "-n")) 
	    doSyms = FALSE;
	else if(!strcmp(argv[i], "-s")) 
	    output = FALSE;
	else if(!strcmp(argv[i], "-h")) 
	    return usage(argv[0]);
	//keyinfo
	else if(!strcmp(argv[i], "-us"))
	    features |= Feature_US;
	else if(!strcmp(argv[i], "-enterprise"))
	    features |= Feature_Enterprise;
	else if(!strcmp(argv[i], "-unix"))
	    features |= Feature_Unix;
	else if(!strcmp(argv[i], "-win"))
	    features |= Feature_Windows;
	else if(!strcmp(argv[i], "-mac"))
	    features |= Feature_Mac;
	else if(!strcmp(argv[i], "-embedded"))
	    features |= Feature_Embedded;
	//files
	else if(*(argv[i]) != '-')  
	    files.append(argv[i]); 
	//unknown
	else 
	    return usage(argv[0], argv[i]); 
    }
    if(!files.isEmpty()) {
	if(dest.isEmpty()) {
	    qDebug("Please specify an output package");
	    return 666;
	}

	QArchive archive;
	ConsoleOutput out;
	QObject::connect( &archive, SIGNAL( operationFeedback( const QString& ) ), 
			  &out, SLOT( updateProgress( const QString& ) ) );
	if(output) 
	    archive.setVerbosity( QArchive::Destination | QArchive::Verbose );
	archive.setSymbolicLinks(doSyms);
	archive.setPath( dest );
	if( !archive.open( IO_WriteOnly ) ) {
	    qDebug("Failed to open output %s", dest.latin1());
	    return 666;
	}
	archive.writeFeatures( features );
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
	QApplication app( argc, argv );
	GeneratorDlgImpl dlg( dest );
	dlg.exec();
    }
    return 0;
}

#include "main.moc"
