/* The qdb command line processor */

#include <qdb.h>
#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <xdb/xbase.h>


#include <op.h> //## remove

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
	qWarning( " -i <table>     Dump table index information to stdout and exit" );
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
    bool index = FALSE;
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
	} else if ( arg == "-i" ) {
	    index = TRUE;
	    if ( i+1 > app.argc()-1 )
		die( "-i requires table name", TRUE );
	    tablename = app.argv()[++i];
	} else if ( arg == "-o" ) {
	    if ( i+1 > app.argc()-1 )
		die( "-o requires filename", TRUE );
	    outfilename = app.argv()[++i];
	} else if ( arg == "-r" ) {
	    index = TRUE;
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
    if ( !QDir::setCurrent( dbdirname ) )
	die( "could not cd: " + dbdirname );

    /* index stuff */
    if ( index && tablename.length() ) {
	if ( verbose )
	    outstream << "index info for " + tablename << endl;
	char buf[XB_MAX_NDX_NODE_SIZE];
	xbShort rc;
	xbXBase x;
	xbDbf file( &x );
	if( ( rc =  file.OpenDatabase( tablename ) ) != 0 )
	    die( "could not open table " + tablename );
	xbNdx idx( &file );
	QFileInfo fi( tablename );
	QString basename = fi.baseName();
	QDir dir;
	QStringList indexnames = dir.entryList( basename + "*.ndx", QDir::Files );
	for ( uint i = 0; i < indexnames.count(); ++i ) {
	    if( ( rc = idx.OpenIndex( indexnames[i] )) != XB_NO_ERROR )
		die( "could not open index " + indexnames[i] );
	    idx.GetExpression( buf,XB_MAX_NDX_NODE_SIZE  );
	    QString output = indexnames[i] + ": " + buf;
	    if ( rebuildindexes ) {
		qDebug( "reindexing " + output );
		if ( idx.ReIndex() != XB_NO_ERROR )
		    output = "...FAILED";
		else
		    output = "...done";
	    }
	    qDebug( output );
	    idx.CloseIndex();
	}
	return 0;
    }

    /* table description */
    if ( tablename.length() ) {
	QDb env;
	env.addFileDriver( 0, tablename );
	qdb::FileDriver* driver = env.fileDriver( 0 );
	if ( !driver->open() )
	    die( "unable to open table:" + tablename );
	if ( !suppressheader ) {
	    outstream << "Table \"" << tablename << "\"" << endl;
	    outstream << sep << "   Attribute   " << sep << "     Type      " << sep
		      << "    Size       " << sep << "    Prec       " << sep << endl;
	    outstream << sep << "---------------" << sep << "---------------" << sep
		      << "---------------" << sep << "---------------" << sep << endl;
	}
	for ( uint i = 0; i < driver->count(); ++i ) {
	    QVariant v;
	    if ( !driver->fieldDescription( i, v ) )
		die( "unable to find field:" + QString::number( i ) );
	    qdb::List l = v.toList();
	    QString name( l[0].toString() );
	    v.cast( (QVariant::Type)l[1].toInt() );
	    outstream << sep << " " << name.leftJustify( 14 ) << sep
		      << " " << QString( v.typeName() ).leftJustify( 14 ) << sep
		      << " " << QString::number( l[2].toInt() ).leftJustify( 14 ) << sep
		      << " " << QString::number( l[3].toInt() ).leftJustify( 14 ) <<  sep << endl;
	}
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
    QDb env;
    env.setOutput( outstream );
    if ( env.parse( commands, echo ) ) {
	if ( analyse )
	    outstream << env.program()->listing().join( "\n" ) << endl;
	else if ( !env.execute( verbose ) )
	    die( env.lastError() );
    } else
	die( env.lastError() );

    /* output results */
    qdb::ResultSet* rs = env.resultSet( 0 ); //## what about more than one result set?
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
	    qdb::Record& rec = rs->currentRecord();
	    for ( i = 0; i < fieldcount; ++i )
		line += rec[i].toString().rightJustify( 15 ) + sep;
	    outstream << line << endl;
	} while( rs->next() );
	if ( verbose )
	    outstream << "(" << rs->size() << " row" << (rs->size()==1?")":"s)") << endl;
    }

    if ( outfile.isOpen() )
	outfile.close();
    if ( verbose )
	outstream << "done" << endl;
    return 0;
}
