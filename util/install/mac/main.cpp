/****************************************************************************
**
** Definition of ________ class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "unpackdlgimpl.h"
#include "qarchive.h"
#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>

class ConsoleOutput : public QObject
{
    Q_OBJECT
public:
    ConsoleOutput() : QObject() { }
    ~ConsoleOutput() { }
public slots:
    void updateProgress( const QString& str) {	qDebug("%s", str.latin1());   }
};
#include "main.moc"

static int usage(const char *argv0, const char *un=NULL) {
    if(un)
	fprintf(stderr, "Unknown command: %s\n", un);
    else
	fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s [options] files...\n", argv0);

    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, " -k [k]     : Use k as the key to open provided files\n");
    fprintf(stderr, " -s         : Quiet mode, will not output process\n");
    fprintf(stderr, " -h         : This help\n");
    return 665;
}

int main( int argc, char** argv )
{
    QString key;
    bool output = TRUE;
    QStringList files;
    QApplication app( argc, argv );
    for(int i = 1; i < argc; i++) {
	//options
	if(!strcmp(argv[i], "-s")) 
	    output = FALSE;
	else if(!strcmp(argv[i], "-k")) 
	    key = argv[++i];
	else if(!strcmp(argv[i], "-h"))
	    return usage(argv[0]);
	//files
	else if(*(argv[i]) != '-')  
	    files.append(argv[i]); 
	//unknown
	else 
	    return usage(argv[0], argv[i]); 
    }
    if(!files.isEmpty()) {
	QArchive archive;
	ConsoleOutput out;
	QObject::connect( &archive, SIGNAL( operationFeedback( const QString& ) ), 
			  &out, SLOT( updateProgress( const QString& ) ) );
	if(output) 
	    archive.setVerbosity( QArchive::Destination | QArchive::Verbose );
	for(QStringList::Iterator it = files.begin(); it != files.end(); ++it) {
	    archive.setPath( (*it) );
	    if( !archive.open( IO_ReadOnly ) ) {
		qDebug("Failed to open input %s", (*it).latin1());
		continue;
	    } 
	    if(!archive.readArchive( QDir::currentDirPath(), key )) 
		qDebug("Failed to unpack %s", (*it).latin1());
	    archive.close();
	}
    } else {
	UnpackDlgImpl dlg(key);
	dlg.exec();
    }
    return 0;
}

