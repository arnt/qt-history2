/*
  LocalSQL

  This file contains the LocalSQL command line processor

  Copyright (C) 2001 Trolltech AS

  Contact:
	 Dave Berton (db@trolltech.com)
	 Jasmin Blanchette (jasmin@trolltech.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "../include/localsql.h"
#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <xdb/xbase.h>

static QString appname;

void usage( bool details = FALSE )
{
    qWarning( "Usage: " + appname + " <options> [command] ..." );
    if ( details ) {
	qWarning( " General Options:" );
	qWarning( " -a             Analyse and quit" );
	qWarning( " -c <commands>  Execute <commands>" );
	qWarning( " -d <dir>       Specify db directory (default:current dir)" );
	qWarning( " -e             Echo commands" );
	qWarning( " -f <file>      Read commands from file" );
	qWarning( " -h             Supress query header output" );
	qWarning( " -o <file>      Place query output in file" );
	qWarning( " -s <char>      Specify query column separation char" );
	qWarning( " -v             Verbose" );
	qWarning( "\n Diagnostic Options:" );
	qWarning( " -r <table>     Rebuild indexes for table" );
	qWarning( " -t <table>     Dump table description to stdout and exit" );
	qWarning( "\nExit status is 0 if command(s) successful, 1 if trouble." );
    }
}

void simple_usage()
{
    usage();
    qWarning( "Try " + appname + " --help for more information." );
}

void die( const QString& message = QString::null, bool doUsage = FALSE )
{
    if ( !message.isNull() ) {
	qWarning( appname + ": " + message );
	if ( doUsage )
	    simple_usage();
    }
    exit(1);
}

int main( int argc, char** argv )
{
    QApplication app( argc, argv, QApplication::Tty ); /* console */
    QFileInfo fi( QString(qApp->argv()[0]) );
    appname = fi.baseName();
    if ( app.argc() == 1 ) {
	simple_usage();
	return 1;
    }

    QString sep = "|";
    QString outfilename;
    QString infilename;
    QString commands;
    QString dbdirname;
    bool verbose = FALSE;
    bool echo = FALSE;
    bool analyse = FALSE;
    QString tablename;
    bool rebuildindexes = FALSE;
    bool suppressheader = FALSE;

    /* process all command line options, die if problem */
    for ( int i = 1; i < app.argc(); ++i ) {
	QString arg = app.argv()[i];
	if ( arg == "-a" ) {
	    analyse = TRUE;
	} else if ( arg == "-c" ) {
	    if ( i+1 > app.argc()-1 )
		die( "no command(s) specified", TRUE );
	    commands = app.argv()[++i];
	} else if ( arg == "-d" ) {
	    if ( i+1 > app.argc()-1 )
		die( "-d requires dirname", TRUE );
	    dbdirname = app.argv()[++i];
	} else if ( arg == "-e" ) {
	    echo = TRUE;
	} else if ( arg == "-f" ) {
	    if ( i+1 > app.argc()-1 )
		die( "-f requires filename", TRUE );
	    infilename = app.argv()[++i];
	} else if ( arg == "-h" ) {
	    suppressheader = TRUE;
	} else if ( arg == "-o" ) {
	    if ( i+1 > app.argc()-1 )
		die( "-o requires filename", TRUE );
	    outfilename = app.argv()[++i];
	} else if ( arg == "-r" ) {
	    rebuildindexes = TRUE;
	    if ( i+1 > app.argc()-1 )
		die( "-r requires table name", TRUE );
	    tablename = app.argv()[++i];
	} else if ( arg == "-s" ) {
	    if ( i+1 > app.argc()-1 )
		die( "-s requires char", TRUE );
	    sep = app.argv()[++i];
	} else if ( arg == "-t" ) {
	    if ( i+1 > app.argc()-1 )
		die( "-t requires table name", TRUE );
	    tablename = app.argv()[++i];
	} else if ( arg == "-v" ) {
	    verbose = TRUE;
	} else if ( arg == "-help" || arg == "-h" || arg == "--help" ) {
	    usage( TRUE );
	    return 0;
	} else if ( arg[0] == '-' )
	    die( "invalid option: " + arg, TRUE );
	else
	    commands += commands.length() ? (" " + arg) : arg;
    }

    /* output file */
    QFile outfile;
    QTextStream outstream( stdout, IO_WriteOnly ); /* default to stdout */
    if ( !outfilename.isNull() ) {
	outfile.setName( outfilename );
	if ( !outfile.open( IO_Truncate | IO_WriteOnly ) )
	    die( "could not open file:" + outfilename );
	outstream.setDevice( &outfile );
    }
    if ( outfile.isOpen() && verbose )
	outstream << "output to file:" + outfilename << endl;


    /* check db dir */
    if ( !dbdirname )
	dbdirname = QDir::currentDirPath();
    QDir dbdir ( dbdirname );
    if ( !dbdir.exists() )
	die( "directory does not exist: " + dbdirname );
    if ( verbose )
	outstream << "using database in " + dbdirname << endl;

    /* index stuff */
    if ( rebuildindexes && tablename.length() ) {
	char buf[XB_MAX_NDX_NODE_SIZE];
	xbShort rc;
	xbXBase x;
	xbDbf file( &x );
	if( ( rc =  file.OpenDatabase( dbdirname + "/" + tablename ) ) != 0 )
	    die( "could not open table " + dbdirname + "/" + tablename );
	xbNdx idx( &file );
	QFileInfo fi( dbdirname + "/" + tablename );
	QString basename = fi.baseName();
	QDir dir( dbdirname );
	QStringList indexnames = dir.entryList( basename + "*.ndx", QDir::Files );
	for ( uint i = 0; i < indexnames.count(); ++i ) {
	    if( ( rc = idx.OpenIndex( dbdirname + "/" + indexnames[i] )) != XB_NO_ERROR )
		die( "could not open index " + dbdirname + "/" + indexnames[i] );
	    idx.GetExpression( buf,XB_MAX_NDX_NODE_SIZE  );
	    QString output = indexnames[i] + ": " + buf;
	    outstream << "Reindexing: " << output << flush;
	    if ( idx.ReIndex() != XB_NO_ERROR )
		output = "...FAILED";
	    else
		output = "...done";
	    outstream << output << endl;
	    idx.CloseIndex();
	}
	return 0;
    }

    /* table description */
    if ( tablename.length() ) {
	LocalSQL env;
	env.setPath( dbdirname );
	uint i = 0;
	env.addFileDriver( 0, tablename );
	LocalSQLFileDriver* driver = env.fileDriver( 0 );
	if ( !driver->open() )
	    die( "unable to open table:" + tablename );
	if ( !suppressheader ) {
	    outstream << "Table \"" << tablename << "\"" << endl;
	    outstream << sep << "   Attribute   " << sep << "     Type      " << sep
		      << "    Size       " << sep << "    Prec       " << sep << endl;
	    outstream << sep << "---------------" << sep << "---------------" << sep
		      << "---------------" << sep << "---------------" << sep << endl;
	}
	QStringList cols = driver->columnNames();
	QValueList<QVariant::Type> types = driver->columnTypes();
	QValueList<uint> sizes = driver->columnSizes();
	QValueList<uint> precs = driver->columnPrecs();
	for ( i = 0; i < cols.count(); ++i ) {
	    QVariant v;
	    QString name = cols[i];
	    v.cast( types[i] );
	    QString typeName( v.typeName() );
	    typeName = typeName.lower();
	    if ( typeName[0] == 'q' )
		typeName = typeName.mid( 1, typeName.length() );
	    uint size = sizes[i];
	    uint prec = precs[i];
	    outstream << sep << " " << name.leftJustify( 14 ) << sep
		      << " " << typeName.leftJustify( 14 ) << sep
		      << " " << QString::number( size ).leftJustify( 14 ) << sep
		      << " " << QString::number( prec ).leftJustify( 14 ) <<  sep << endl;
	}
	QStringList priIdxDesc = driver->primaryIndex();
	QString priIdx;
	if ( priIdxDesc.count() )
	     priIdx = priIdxDesc.join( "+" );
	if ( priIdx.length() )
	    outstream << "primary index: " << priIdx << endl;
	QStringList idx = driver->indexNames();
	for ( i = 0; i < idx.count(); ++i ) {
	    if ( idx[i] != priIdx )
		outstream << "index: " << idx[i] << endl;
	}
	outstream << "records in table:" << driver->size() << endl;
	return 0;
    }

    /* get commands */
    if ( !commands ) {
	if ( infilename ) {
	    if ( verbose )
		outstream << "reading commands from:" + infilename << endl;
	    QFile f( infilename );
	    if ( !f.exists() )
		die( "file does not exist:" + infilename );
	    if ( !f.open( IO_ReadOnly ) )
		die( "could not open file:" + infilename );
	    QTextStream ts( &f );
	    commands = ts.read();
	    f.close();
	} else {
	    /* read from stdin */
	    //## todo
	}
    }
    if ( !commands )
	die( "no commands specified" );

    /* execute commands */
    LocalSQL env;
    env.setPath( dbdirname );
    env.setOutput( outstream );
    if ( env.parse( commands, echo ) ) {
	if ( analyse )
	    outstream << env.program()->listing().join( "\n" ) << endl;
	else if ( !env.execute( verbose ) )
	    die( env.lastError() );
    } else
	die( env.lastError() );

    /* output results */
    LocalSQLResultSet* rs = env.resultSet( 0 ); //## what about more than one result set?
    if ( rs->size() ) {
	uint fieldcount = rs->count();
	uint i = 0;
	QString line = " ";
	if ( !suppressheader ) {
	    QStringList cols = rs->columnNames();
	    for ( i = 0; i < fieldcount; ++i )
		line += cols[i].rightJustify( 15 ) + " ";
	    outstream << line << endl;
	    line = sep;
	    for ( i = 0; i < fieldcount; ++i )
		line += QString().rightJustify( 15, '-' ) + sep;
	    outstream << line << endl;
	}
	rs->first();
	do {
	    line = sep;
	    localsql::Record& rec = rs->currentRecord();
	    for ( i = 0; i < fieldcount; ++i )
		line += rec[i].toString().rightJustify( 15 ).mid( 0, 15 ) + sep;
	    outstream << line << endl;
	} while( rs->next() );
	outstream << rs->size() << " record(s) processed" << endl;
    } else if ( env.affectedRows() > -1 ) {
	outstream << env.affectedRows() << " record(s) processed" << endl;
    }


    if ( outfile.isOpen() )
	outfile.close();
    if ( verbose )
	outstream << "done" << endl;
    return 0;
}
