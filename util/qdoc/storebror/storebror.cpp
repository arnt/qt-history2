/*
  storebror.cpp
*/

#include <qfile.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include <errno.h>
#include <string.h>

#include "../config.h"

int lastSafeChangeNo = 37000;
QStringList supervisors = QStringList() << "mark";
QStringList okayAuthors = QStringList() << "andy" << "jasmin" << "monica";

static QString fileContents( const QString& fileName )
{
    QFile f( fileName );
    if ( !f.open(IO_ReadOnly) ) {
	qWarning( "  %s", strerror(errno) );
	return QString::null;
    }

    QTextStream t( &f );
    QString contents = t.read();
    f.close();
    if ( contents.isEmpty() )
	qWarning( "  File is empty" );
    return contents;
}

static void analyzeComment( const QString& /* fileName */,
			    const QString& comment )
{
    static QRegExp lineExp( "\\s*(\\d+)\\s+(\\S+)\\s+(\\d+)\\s+\\d+[^\n]*" );

    QMap<QString, int> authors;
    int lastChangeNo = 0;
    QString lastAuthor;
    int lastLineNo = 0;

    if ( comment.find(QString("\\internal")) != -1 )
	return;
    QStringList lines = QStringList::split( QChar('\n'), comment );
    QStringList::ConstIterator x = lines.begin();
    while ( x != lines.end() ) {
	if ( !lineExp.exactMatch(*x) )
	    qFatal( "  Internal error parsing annotated file (fix storebror)" );

	int lineNo = lineExp.cap( 1 ).toInt();
	QString author = lineExp.cap( 2 );
	int changeNo = lineExp.cap( 3 ).toInt();

	if ( changeNo > lastSafeChangeNo ) {
	    if ( supervisors.contains(author) == 0 )
		authors.insert( author, 0 );
	    if ( changeNo > lastChangeNo &&
		 okayAuthors.contains(author) == 0 ) {
		lastChangeNo = changeNo;
		lastAuthor = author;
		lastLineNo = lineNo;
	    }
	}
	++x;
    }

    if ( lastChangeNo > 0 && supervisors.contains(lastAuthor) == 0 ) {
	QString msg = QString( "Line %1 edited by %2" )
		      .arg( lastLineNo ).arg( lastAuthor );
	authors.remove( lastAuthor );
	if ( !authors.isEmpty() ) {
	    QMap<QString, int>::ConstIterator a = authors.begin();
	    msg += QString( " (also " );
	    msg += a.key();
	    ++a;
	    while ( a != authors.end() ) {
		msg += QString( ", " ) + a.key();
		++a;
	    }
	    msg += QString( ")" );
	}
	msg += QString( " change %1" ).arg( lastChangeNo );
	qWarning( "  %s", msg.latin1() );
    }
}

static void analyzeFile( const QString& fileName )
{
    static QRegExp docComment( QString("\n[^\n]*/\\*!.*\\*/.*\n") );
    docComment.setMinimal( TRUE );

    qWarning( "Processing %s", fileName.latin1() );

    system( QString("annotate %1 2>&1 > /tmp/storebror")
	    .arg(fileName).latin1() );
    QString str = fileContents( "/tmp/storebror" );
    int k = 0;
    while ( (k = str.find(docComment, k)) != -1 ) {
	analyzeComment( fileName,
			QConstString(str.unicode() + k + 1,
				     docComment.matchedLength() - 1).string() );
	k += docComment.matchedLength();
    }

    if ( str.find(QString("internal error applying diffs")) != -1 )
	qWarning( "  Internal error applying diffs (fix annotate)" );
}

int main( int argc, char **argv )
{
    config = new Config( argc, argv );

    QStringList sourceFiles =
	    config->findAll( QString("*.cpp"), config->sourceDirList() ) +
	    config->findAll( QString("*.doc"), config->docDirList() );
    QStringList::ConstIterator s = sourceFiles.begin();
    while ( s != sourceFiles.end() ) {
	analyzeFile( *s );
	++s;
    }
    return 0;
}
