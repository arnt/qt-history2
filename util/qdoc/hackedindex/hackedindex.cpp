/*
  hackedindex.cpp
*/

#include <qdir.h>
#include <qfile.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstringlist.h>

#include <errno.h>

static QString protect( const QString& str, QChar metaCh )
{
    /*
      Suppose metaCh is '|' and str is '\ is not |'. The result should be
      '\\ is not \|' or, in C++ notation, "\\\\ is not \\|".
    */
    QString t = str;
    t.replace( QRegExp(QString("\\\\")), QString("\\\\") );
    t.replace( QRegExp(QString("\\") + metaCh), QString("\\") + metaCh );
    return t;
}

static QString fileContents( const QString& fileName )
{
    QFile f( fileName );
    if ( !f.open(IO_ReadOnly) ) {
	qWarning( "Cannot open file '%s' for reading: %s", fileName.latin1(),
		  strerror(errno) );
	return QString::null;
    }

    QTextStream t( &f );
    QString contents = t.read();
    f.close();
    if ( contents.isEmpty() )
	qWarning( "File '%s' is empty", fileName.latin1() );
    return contents;
}

int main( int argc, char **argv )
{
    QRegExp indexCmd( QString("<!-- index ([^\n]*) -->") );
    QMap<QString, QStringList> indexMap;
    int yunique = 1;

    indexCmd.setMinimal( TRUE );

    if ( argc != 2 ) {
	qWarning( "Usage: hackedindex doc-dir" );
	return -1;
    }

    QDir dir( argv[1] );
    dir.setNameFilter( QString("*.html") );
    dir.setSorting( QDir::Name );
    dir.setFilter( QDir::Files );
    QStringList fileNameList = dir.entryList();

    QStringList::ConstIterator f = fileNameList.begin();
    while ( f != fileNameList.end() ) {
	QString t = fileContents( dir.filePath(*f) );
	int k = 0;
	while ( (k = indexCmd.search(t, k)) != -1 ) {
	    k += indexCmd.matchedLength();
	    indexMap[indexCmd.cap(1)].append(
		    QString("%1#y%2").arg(*f).arg(yunique) );
	    t.insert( k, QString("<a name=\"y%1\"></a>").arg(yunique) );
	    yunique++;
	}

	QFile out( dir.filePath(*f) );
	if ( out.open(IO_WriteOnly) ) {
	    out.writeBlock( t.latin1(), t.length() );
	    out.close();
	} else {
	    qWarning( "Cannot write to '%s'", out.name().latin1() );
	}
	++f;
    }

    QFile index( dir.filePath(QString("index")) );
    if ( index.open(IO_WriteOnly) ) {
	QMap<QString, QStringList>::Iterator x = indexMap.begin();
	while ( x != indexMap.end() ) {
	    QStringList value = *x;
	    QStringList::Iterator y = value.begin();
	    while ( y != value.end() ) {
		QString line = QString( "\"%1\" %2\n" )
			       .arg(protect(x.key(), QChar('"')))
			       .arg(*y)
			       .latin1();
		index.writeBlock( line.latin1(), line.length() );
		++y;
	    }
	    ++x;
	}
    } else {
	qWarning( "Cannot write to '%s'", index.name().latin1() );
    }
    return 0;
}
