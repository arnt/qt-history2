#include "qarchive.h"
#include <qapplication.h>
#include <qfileinfo.h>
#include "generatordlgimpl.h"

class ConsoleOutput : public QObject
{
    Q_OBJECT
public:
    ConsoleOutput() : QObject() { }
    ~ConsoleOutput() { }
public slots:
    void updateProgress( const QString& str) {	qDebug("%s", str.latin1());   }
};

int main( int argc, char** argv )
{
    bool output = TRUE;
    QString dest;
    QStringList files;
    for(int i = 1; i < argc; i++) {
	if(!strcmp(argv[i], "-o")) {
	    dest = argv[++i];
	} else if(!strcmp(argv[i], "-s")) {
	    output = FALSE;
	} else {
	    files.append(argv[i]);
	}
    }
    if(!files.isEmpty()) {
	if(dest.isEmpty()) {
	    qDebug("Please specify an output package");
	    return 666;
	}

	QArchive archive;
	ConsoleOutput out;
	if(output) {	
	    archive.setVerbosity( QArchive::Destination | QArchive::Verbose );
	    QObject::connect( &archive, SIGNAL( operationFeedback( const QString& ) ), 
			      &out, SLOT( updateProgress( const QString& ) ) );
	}
	archive.setPath( dest );
	if( !archive.open( IO_WriteOnly ) ) {
	    qDebug("Failed to open output %s", dest.latin1());
	    return 666;
	}
	for(QStringList::Iterator it = files.begin(); it != files.end(); ++it) {
	    QFileInfo f((*it));
	    if(!f.exists()) {
		qDebug("Failed to open %s", (*it).latin1());
		continue;
	    }
	    if(f.isDir()) 
		archive.writeDir( (*it), true, (*it) );
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
