/* The qdb command line processor */

#include "sqlinterpreter.h"
#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>

static QString appname;

void usage( const QString& message = QString::null )
{
    if ( !message.isNull() )
	qWarning( appname + ": " + message );
    qWarning( "Usage: " + appname + " <options> [command]" );
    qWarning( " Options:" );
    qWarning( " -a             Analyse and quit" );
    qWarning( " -c <commands>  Execute <commands>" );
    qWarning( " -d <dir>       Specify db directory (default:current dir)" );
    qWarning( " -e             Echo commands" );
    qWarning( " -f <file>      Read commands from file" );
    qWarning( " -o <file>      Place output in file" );
    qWarning( " -v             Verbose" );
    qWarning( "\nExit status is 0 if command(s) successful, 1 if trouble." );
}

void die( const QString& message = QString::null )
{
    usage( message );
    exit(1);
}

int main( int argc, char** argv )
{
    QApplication app( argc, argv, QApplication::Tty ); /* console */
    QFileInfo fi( QString(qApp->argv()[0]) );
    appname = fi.baseName();
    if ( app.argc() == 1 )
	die();

    QString outfilename;
    QString infilename;
    QString commands;
    QString dbdirname;
    bool verbose = FALSE;
    bool echo = FALSE;
    bool analyse = FALSE;

    /* process all command line options, die if problem */
    for ( int i = 1; i < app.argc(); ++i ) {
	QString arg = app.argv()[i];
	if ( arg == "-a" ) {
	    analyse = TRUE;
	} else if ( arg == "-c" ) {
	    if ( i+1 > app.argc()-1 )
		die( "no command(s) specified" );
	    commands = app.argv()[++i];
	} else if ( arg == "-d" ) {
	    if ( i+1 > app.argc()-1 )
		die( "-d requires dirname" );
	    dbdirname = app.argv()[++i];
	} else if ( arg == "-e" ) {
	    echo = TRUE;
	} else if ( arg == "-f" ) {
	    if ( i+1 > app.argc()-1 )
		die( "-f requires filename" );
	    infilename = app.argv()[++i];
	} else if ( arg == "-o" ) {
	    if ( i+1 > app.argc()-1 )
		die( "-o requires filename" );
	    outfilename = app.argv()[++i];
	} else if ( arg == "-v" ) {
	    verbose = TRUE;
	} else if ( arg == "-help" || arg == "-h" || arg == "--help" ) {
	    usage();
	    return 0;
	} else
	    die( "invalid option: " + arg );
    }

    /* output file */
    QFile outfile;
    QTextStream outstream( stdout, IO_WriteOnly ); /* default to stdout */
    if ( !outfilename.isNull() ) {
	if ( verbose )
	    qWarning( "output to file:" + outfilename );
	outfile.setName( outfilename );
	if ( !outfile.open( IO_Truncate | IO_WriteOnly ) )
	    die( "could not open file:" + outfilename );
	outstream.setDevice( &outfile );
    }

    /* check db dir */
    if ( !dbdirname )
	dbdirname = QDir::currentDirPath();
    QDir dbdir ( dbdirname );
    if ( !dbdir.exists() )
	die( "directory does not exist: " + dbdirname );
    if ( verbose )
	qWarning( "using database in " + dbdirname );
    if ( !QDir::setCurrent( dbdirname ) )
	die( "could not cd: " + dbdirname );

    /* get commands */
    if ( !commands ) {
	if ( infilename ) {
	    if ( verbose )
		qWarning( "reading commands from:" + infilename );
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
    Environment env;
    env.setOutput( outstream );
    if ( env.parse( commands, echo ) ) {
	if ( analyse )
	    env.saveListing( outstream );
	else if ( !env.execute( verbose ) )
	    die( env.lastError() );
    } else
	die( env.lastError() );

    /* output results */
    ResultSet& rs = env.resultSet( 0 ); //## what about more than one result set? will this always be the last one?
    if ( rs.size() ) {
	rs.first();
	do {
	    //## todo
	} while( rs.next() );
    }
    if ( outfile.isOpen() )
	outfile.close();
    return 0;
}
