#include <qfile.h>
#include <qdatastream.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qprocess.h>

#include <stdlib.h>
#include <windows.h>

#include <zlib/zlib.h>

#include "filebuffer.h"

QString makeCmd( "nmake" );

#define BUFFERSIZE 65536
/*
** Writes a file to the archive
**
** First, the length is written, then the file data is written
*/
void writeFile( QFileInfo* fi, QDataStream& outStream )
{
    QList<FileBuffer> zBuffer;
    QFile inFile( fi->absFilePath() );
    unsigned char* inBuffer;
    unsigned char* outBuffer;
    z_stream ztream;
    int bytesRead,totalOut( 0 );
    FileBuffer* tmpOut;
    int deflateCode;
    bool continueCompressing;

    MEMORYSTATUS memstat1,memstat2;

    if( inFile.open( IO_ReadOnly ) ) {
	if( inBuffer = (unsigned char*)malloc( BUFFERSIZE ) ) {
	    if( outBuffer = (unsigned char*)malloc( BUFFERSIZE ) ) {
//	        printf( "Deflating %s... ", fi->absFilePath().latin1() );
		ztream.next_out = outBuffer;
		ztream.avail_out = BUFFERSIZE;
		ztream.total_out = 0;
		ztream.msg = NULL;
		ztream.zalloc = (alloc_func)0;
		ztream.zfree = (free_func)0;
		ztream.opaque = (voidpf)0;
		ztream.data_type = Z_BINARY;
		deflateInit( &ztream, 9 );
		while( ( bytesRead = inFile.readBlock( (char*)inBuffer, BUFFERSIZE ) ) > 0 ) {
		    ztream.next_in = inBuffer;	// Reset the input buffer
		    ztream.avail_in = bytesRead;
		    if( bytesRead < BUFFERSIZE ) { // This was the rest of the data, so treat it as a special case
			deflateCode = Z_OK;
			while( deflateCode == Z_OK ) {
			    deflateCode = deflate( &ztream, Z_FINISH );
			    if( tmpOut = new FileBuffer( (char*)outBuffer, ztream.total_out ) )
				zBuffer.append( tmpOut );
			    totalOut += ztream.total_out;
			    ztream.next_out = outBuffer;
			    ztream.avail_out = BUFFERSIZE;
			    ztream.total_out = 0;
			}
			totalOut += ztream.total_out;
		    }
		    else {
			while( ztream.avail_in ) {	// Compress until we have no more input data
			    continueCompressing = true;
			    while( continueCompressing ) {
				deflateCode = deflate( &ztream, Z_NO_FLUSH );
				continueCompressing = ( deflateCode == Z_OK ) && ( ztream.avail_out == 0 );

				if( !ztream.avail_out ) { // Output buffer full => flush output to disk
				    if( tmpOut = new FileBuffer( (char*)outBuffer, ztream.total_out ) )
					zBuffer.append( tmpOut );
				    totalOut += ztream.total_out;
				    ztream.next_out = outBuffer;    // Reset the output buffer
				    ztream.avail_out = BUFFERSIZE;
				    ztream.total_out = 0;
				}
			    }
			}	// No more input data, try to get some more.
		    }
		}

//		printf( "done. %d => %d (%.1f%%)\n", ztream.total_in, totalOut, float( float( totalOut ) / float( ztream.total_in ) * 100 ) );
		deflateEnd( &ztream );
		// Now write the compressed data to the output
		outStream << totalOut;
		while( !zBuffer.isEmpty() ) {
		    tmpOut = zBuffer.first();
		    outStream.writeRawBytes( tmpOut->data(), tmpOut->size() );
		    zBuffer.remove( tmpOut );
		    delete tmpOut;
		}
	    }
	    free( inBuffer );
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
    QStringList args;
    QProcess extproc;

    args += QString( getenv( "QTDIR" ) ) + "\\bin\\qmake";
    args += "CONFIG+=windows release";
    args += QString( "DEFINES+=DISTVER=\\\"" ) + distname + "\\\"";
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
    copyFile( QString( getenv( "QTDIR" ) ) + "\\tests\\install\\setup.exe", QString( getenv( "QTDIR" ) ) + QString( "\\tests\\install\\generator\\dist" ) + distname + "\\setup.exe" );
    qDebug( "Copied setup program" );
}

int main( int argc, char** argv )
{
    QDir distdir;
    QStringList dirList;
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

    dirList << "dist\\win\\" << "src" << "include" << "mkspecs" << "plugins" << "qmake" << "tmake" << "tools";
    generateArchive( distname + "\\qt.z", dirList );

    dirList.clear();
    dirList << "doc" << "gif";
    generateArchive( distname + "\\doc.z", dirList );

    dirList.clear();
    dirList << "tutorial";
    generateArchive( distname + "\\tutorial.z", dirList );

    dirList.clear();
    dirList << "examples";
    generateArchive( distname + "\\examples.z", dirList );

    buildInstaller( argv[ 1 ] );
    return 0;
}