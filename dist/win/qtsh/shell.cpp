#include "shell.h"
#include <qapplication.h>
#include <stdio.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <stdlib.h>

Shell::Shell()
{
    parse();
}

void Shell::parse()
{
    if ( qApp->argc() > 1 ) {
	QString cmd = qApp->argv()[ 1 ];
	if ( cmd == "mv" )
	    parseMove();
	else if ( cmd == "cp" )
	    parseCopy();
	else if ( cmd == "rm" )
	    parseRemove();
	else
	    error( UnknownCommand, cmd );
    } else {
	error( NoCommand );
    }
}

void Shell::parseMove()
{
    if ( qApp->argc() < 3 ) {
	error( MvTooFewArguments );
	return;
    }	

    if ( qApp->argc() > 4 ) {
	error( MvTooMuchArguments );
	return;
    }	

    QString src = qApp->argv()[ 2 ];
    QString dest = qApp->argv()[ 3 ];

    if ( !QFile::exists( src ) ) {
	error( MvFileDoesNotExist, src );
	return;
    }

    QFileInfo di( dest );
    QFileInfo si( src );
    if ( di.isDir() ) {
	dest += "/" + si.fileName();
	di = QFileInfo( dest );
    }
    QDir d( si.dirPath() );
    QDir d2( di.dirPath() );
    d.rename( si.fileName(), d2.canonicalPath() + "/" + di.fileName() );
    ::exit( 0 );
}

void Shell::parseCopy()
{
    if ( qApp->argc() < 3 ) {
	error( CpTooFewArguments );
	return;
    }	

    if ( qApp->argc() > 4 ) {
	error( CpTooMuchArguments );
	return;
    }	

    QString src = qApp->argv()[ 2 ];
    QString dest = qApp->argv()[ 3 ];

    if ( !QFile::exists( src ) ) {
	error( CpFileDoesNotExist, src );
	return;
    }

    QFileInfo di( dest );
    QFileInfo si( src );
    if ( di.isDir() ) {
	dest += "/" + si.fileName();
	di = QFileInfo( dest );
    }
    QDir d( si.dirPath() );
    QDir d2( di.dirPath() );

    QFile f1( d.canonicalPath() + "/" + si.fileName() );
    f1.open( IO_ReadOnly );
    QByteArray ba;
    ba.resize( f1.size() );
    f1.readBlock( ba.data(), f1.size() );
    f1.close();

    QFile f2( d2.canonicalPath() + "/" + di.fileName() );
    f2.open( IO_WriteOnly );
    f2.writeBlock( ba, ba.size() );
    f2.close();
    ::exit( 0 );
}

void Shell::parseRemove()
{
    if ( qApp->argc() < 3 ) {
	error( RmTooFewArguments );
	return;
    }	
	
    for ( int i = 2; i < qApp->argc(); ++i ) {
	QString a = qApp->argv()[ i ];
	if ( a[ 0 ] == '@' )
	    removeFiles( a.mid( 1 ) );
	else
	    removeFile( a );
    }
    ::exit( 0 );
}

void Shell::removeFiles( const QString &fn )
{
    QFile f( fn );
    if ( !f.open( IO_ReadOnly ) || !f.exists() ) {
	warning( RmCannotOpenFile, fn );
	return;
    }
    QTextStream ts( &f );
    QString tmp = ts.read();
    tmp = tmp.simplifyWhiteSpace();
    QStringList files = QStringList::split( ' ', tmp );
    for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
	removeFile( *it );
}

void Shell::removeFile( const QString &f )
{
    if ( !QFile::exists( f ) ) {
	warning( RmFileDoesNotExist, f );
	return;
    }
    QString fn = QDir::cleanDirPath( f );
    QFileInfo fi( fn );
    QDir d( fi.dirPath() );
    d.remove( fi.fileName() );
}

void Shell::error( Error e, const QString &msg )
{
    switch ( e ) {
    case NoCommand:
	fprintf( stderr, "Error: No command given (possible are mv, cp, and rm)\n" );
	::exit( -1 );
	break;
    case RmTooFewArguments:
	fprintf( stderr, "Error: rm: too few arguments\n" );
	::exit( -1 );
	break;
    case MvTooFewArguments:
	fprintf( stderr, "Error: mv: too few arguments\n" );
	::exit( -1 );
	break;
    case MvTooMuchArguments:
	fprintf( stderr, "Error: mv: too much arguments\n" );
	::exit( -1 );
	break;
    case MvFileDoesNotExist:
	fprintf( stderr, "Error: mv: file %s does not exist\n", msg.latin1() );
	::exit( -1 );
	break;
    case CpTooFewArguments:
	fprintf( stderr, "Error: cp: too few arguments\n" );
	::exit( -1 );
	break;
    case CpTooMuchArguments:
	fprintf( stderr, "Error: cp: too much arguments\n" );
	::exit( -1 );
	break;
    case CpFileDoesNotExist:
	fprintf( stderr, "Error: cp: file %s does not exist\n", msg.latin1() );
	::exit( -1 );
	break;
    case UnknownCommand:
	fprintf( stderr, "Error: unknwon command: %s (possible are mv, cp and rm)\n", msg.latin1() );
	::exit( -1 );
	break;
    }
}

void Shell::warning( Warning w, const QString &msg )
{
    switch ( w ) {
    case RmCannotOpenFile:
	fprintf( stderr, "Warning: rm: Cannot open file %s\n", msg.latin1() );
	break;
    case RmFileDoesNotExist:
	fprintf( stderr, "Warning: rm: File %s does not exist - cannot remove it\n", msg.latin1() );
	break;
    }
}
