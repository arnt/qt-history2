#include <qfile.h>
#include <qdatastream.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qprocess.h>

#include <stdlib.h>
#include <windows.h>

#include <zlib/zlib.h>

QString makeCmd( "nmake" );

#define BUFFERSIZE 512 * 1024
/*
** Writes a file to the archive
**
** First, the length is written, then the file data is written
*/
void writeFile( QFileInfo* fi, QDataStream& outStream )
{
    QFile inFile( fi->absFilePath() );
    QByteArray inBuffer;
    QByteArray outBuffer( BUFFERSIZE );
    z_stream ztream;
    bool continueCompressing;

    if( inFile.open( IO_ReadOnly ) ) {
	if( inBuffer.resize( fi->size() ) ) {
	    printf( "Deflating %s... ", fi->absFilePath().latin1() );
	    ztream.next_in = (unsigned char*)inBuffer.data();
	    ztream.avail_in = inBuffer.size();
	    ztream.total_in = 0;
	    ztream.next_out = (unsigned char*)outBuffer.data();
	    ztream.avail_out = outBuffer.size();
	    ztream.total_out = 0;
	    ztream.msg = NULL;
	    ztream.zalloc = (alloc_func)NULL;
	    ztream.zfree = (free_func)NULL;
	    ztream.opaque = (voidpf)NULL;
	    ztream.data_type = Z_BINARY;
	    deflateInit( &ztream, 9 );
	    inFile.readBlock( inBuffer.data(), inBuffer.size() );

	    continueCompressing = true;
	    while( continueCompressing ) {
		continueCompressing = ( deflate( &ztream, Z_FINISH ) == Z_OK );
		if( !ztream.avail_out ) {
		    if( !outBuffer.resize( outBuffer.size() + BUFFERSIZE ) )
			qFatal( "Could not allocate compression buffer!" );
		    ztream.next_out = (unsigned char*)&outBuffer.data()[ ztream.total_out ];
		    ztream.avail_out = BUFFERSIZE;
		}
	    }

	    printf( "done. %d => %d (%.1f%%)\n", ztream.total_in, ztream.total_out, float( float( ztream.total_out ) / float( ztream.total_in ) * 100 ) );
	    deflateEnd( &ztream );
	    // Now write the compressed data to the output
	    outStream << ztream.total_out;
	    outStream.writeRawBytes( outBuffer.data(), ztream.total_out );
	}
	inFile.close();
    }
}

/*
** Writes the contents of a directory to the archive file.
** Recurses into subdirectories if needed.
**
** At current, the following entries are excluded:
**
** All entries beginning with a dot ('.')
** All "tmp" and "depot" entries
*/
void writeDir( QDir& srcDir, QDataStream& outStream, bool topLevel = false )
{
    if( !topLevel )
	outStream << QString( srcDir.dirName() + "\\" ).latin1();
    
    const QFileInfoList* dirEntries = srcDir.entryInfoList();
    QFileInfoListIterator dirIter( *dirEntries );
    QFileInfo* fi;

    dirIter.toLast();
    while( ( fi = dirIter.current() ) ) {
	if( ( fi->fileName()[0] != '.' ) && ( fi->fileName() != "tmp" ) && ( fi->fileName() != "depot" ) ){
	    if( fi->isDir() )
	    {
		QDir subDir( srcDir.absPath() + "\\" + fi->fileName() );
		writeDir( subDir, outStream );
	    }
	    else {
		outStream << fi->fileName().latin1();
		outStream << fi->lastModified();
		writeFile( fi, outStream );
	    }
	}
	--dirIter;
    }
    if( !topLevel )
	outStream << QString( "..\\" ).latin1();
}

void generateArchive( const QString arcName, QStringList dirNames )
{
    QFile outFile( arcName );
    QDataStream outStream;

    if( outFile.open( IO_WriteOnly ) ) {
	outStream.setDevice( &outFile );
        QString qtDir = getenv( "QTDIR" );
	for( QStringList::Iterator it = dirNames.begin(); it != dirNames.end(); ++it ) {
	    QString srcDirName = qtDir + "\\" + *it;
	    QDir srcDir( srcDirName );
	    if( srcDirName.right( 1 ) == "\\" )
		writeDir( srcDir, outStream, true );
	    else
		writeDir( srcDir, outStream, false );
	}
    }
}

void generateFileArchive( const QString arcName, QStringList fileNames )
{
    QFile outFile( arcName );
    QDataStream outStream;
    QFileInfo fi;

    if( outFile.open( IO_WriteOnly ) ) {
	outStream.setDevice( &outFile );
	for( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it ) {
	    QString srcName = *it;
	    fi.setFile( srcName );
	    outStream << fi.fileName().latin1();
	    outStream << fi.lastModified();
	    writeFile( &fi, outStream );
	}
    }
}

void copyFile( QString src, QString dst )
{
    QFile inFile( src );
    QFile outFile( dst );
    char* buffer;
    int bytesRead;

    if( inFile.open( IO_ReadOnly ) ) {
	if( outFile.open( IO_WriteOnly ) ) {
	    if( buffer = (char*)malloc( BUFFERSIZE ) ) {
    		while( ( bytesRead = inFile.readBlock( buffer, BUFFERSIZE ) ) > 0 )
		    outFile.writeBlock( buffer, bytesRead );
		free( buffer );
	    }
	    outFile.close();
	}
	inFile.close();
    }
}

void buildInstaller( QString distname )
{
/*
    QStringList args;
    QProcess extproc;

    args += QString( getenv( "QTDIR" ) ) + "\\bin\\qmake";
    args += "CONFIG+=windows release";
    args += QString( "DEFINES+=DISTVER=\\\\\\\"" ) + distname + "\\\\\\\"";
    args += QString( getenv( "QTDIR" ) ) + "\\tests\\install\\setup.pro";
    args += QString( "-o" );
    args += QString( getenv( "QTDIR" ) ) + "\\tests\\install\\Makefile";
    extproc.setWorkingDirectory( QString( getenv( "QTDIR" ) ) + "\\tests\\install" );
    extproc.setArguments( args );
    extproc.start();
    while( extproc.isRunning() )
	Sleep( 100 );
    qDebug( "qmake is done" );
    args.clear();
    args += makeCmd;
    args += QString( "clean" );
    extproc.setArguments( args );
    extproc.start();
    while( extproc.isRunning() )
	Sleep( 100 );
    qDebug( "make clean is done" );
    args.clear();
    args += makeCmd;
    extproc.setArguments( args );
    extproc.start();
    while( extproc.isRunning() )
	Sleep( 100 );
    qDebug( "make is done" );
*/
    copyFile( QString( getenv( "QTDIR" ) ) + "\\bin\\setup.exe", QString( getenv( "QTDIR" ) ) + QString( "\\tests\\install\\generator\\dist" ) + distname + "\\setup.exe" );
    qDebug( "Copied setup program" );
}

int main( int argc, char** argv )
{
    QDir distdir;
    QStringList dirList;
    QStringList fileList;
    QString distname;

    if( ( argc < 2 ) || ( argc > 3 ) ) {
	printf( "Usage: %s <dist> [makecommand]\n\n", argv[ 0 ] );
	return 1;
    }
    if( argc == 3 )
    {
	makeCmd = QString( argv[ 2 ] );
    }
    distname = QString( "dist" ) + argv[ 1 ];

    distdir.mkdir( distname );

    fileList << QString( getenv( "QTDIR" ) ) + "\\dist\\commercial\\LICENSE";
    fileList << QString( getenv( "QTDIR" ) ) + "\\tests\\install\\INSTALL_DONE.TXT";
    fileList << QString( getenv( "QTDIR" ) ) + "\\tests\\install\\integrate_msvc.bat";
    fileList << QString( getenv( "QTDIR" ) ) + "\\tests\\install\\integrate_borland.bat";
    fileList << QString( getenv( "QTDIR" ) ) + "\\tests\\install\\integrate_gcc.bat";
    generateFileArchive( distname + "\\sys.arq", fileList );

    fileList.clear();
    fileList << QString( getenv( "QTDIR" ) ) + "\\Makefile";
    fileList << QString( getenv( "QTDIR" ) ) + "\\configure.bat";
    generateFileArchive( distname + "\\build.arq", fileList );

    fileList.clear();
    fileList << QString( getenv( "QTDIR" ) ) + "\\bin\\quninstall.exe";
    generateFileArchive( distname + "\\uninstall.arq", fileList );

    dirList << "dist\\win\\" << "src" << "include" << "mkspecs" << "plugins" << "qmake" << "tmake" << "tools";
    generateArchive( distname + "\\qt.arq", dirList );

    dirList.clear();
    dirList << "doc" << "gif";
    generateArchive( distname + "\\doc.arq", dirList );

    dirList.clear();
    dirList << "tutorial";
    generateArchive( distname + "\\tutorial.arq", dirList );

    dirList.clear();
    dirList << "examples";
    generateArchive( distname + "\\examples.arq", dirList );

    buildInstaller( argv[ 1 ] );

    return 0;
}
