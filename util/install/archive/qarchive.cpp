#include "qarchive.h"
#include <qdatastream.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qapplication.h>
#include <zlib/zlib.h>

QArchive::QArchive( const QString& archivePath )
{
    setPath( archivePath );

    bufferSize = 512 * 1024;
}

QArchive::~QArchive()
{
}

void QArchive::setPath( const QString& archivePath )
{
    QString fullName = archivePath;

    if( fullName.right( 4 ) != ".arq" )
	fullName += ".arq";

    arcFile.setName( fullName );
}

bool QArchive::open( int mode )
{
    switch( mode ) {
    case IO_ReadOnly:
	// Fallthrough intentional
    case IO_WriteOnly:
	if( arcFile.open( mode ) ) 
	    return true;
	break;
    }
    return false;
}

void QArchive::close()
{
    if( arcFile.isOpen() ) 
	arcFile.close();
}

bool QArchive::writeFile( const QString& fileName, const QString& localPath )
{
    if( arcFile.isOpen() ) {
	QDataStream outStream( &arcFile );
	QFileInfo fi( fileName );

	QFile inFile( fi.absFilePath() );
	QByteArray inBuffer;
	QByteArray outBuffer( bufferSize );
	z_stream ztream;
	bool continueCompressing;

	if( inFile.open( IO_ReadOnly ) ) {
	    if( inBuffer.resize( fi.size() ) ) {
		outStream << fi.fileName().latin1();
		outStream << fi.lastModified();

	        if( verbosityMode & Source )
		    emit operationFeedback( "Deflating " + fi.absFilePath() + "... " );
		else if( verbosityMode & Destination )
		    emit operationFeedback( "Deflating " + localPath + "/" + fi.fileName() + "... " );
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
		    if(qApp)
			qApp->processEvents();
		    continueCompressing = ( deflate( &ztream, Z_FINISH ) == Z_OK );
		    if( !ztream.avail_out ) {
			if( !outBuffer.resize( outBuffer.size() + bufferSize ) )
			    qFatal( "Could not allocate compression buffer!" );
			ztream.next_out = (unsigned char*)&outBuffer.data()[ ztream.total_out ];
			ztream.avail_out = bufferSize;
		    }
		}

		emit operationFeedback( QString( "done. %1 => %2 (%3%)\n" ).arg( ztream.total_in ).arg( ztream.total_out ).arg( int( double( ztream.total_out ) / double( ztream.total_in ) * 100 ) ) );
		deflateEnd( &ztream );
		// Now write the compressed data to the output
		outStream << ztream.total_out;
		outStream.writeRawBytes( outBuffer.data(), ztream.total_out );
	    }
	    inFile.close();
	    return true;
	}
    }
    return false;
}

bool QArchive::setDirectory( const QString& dirName )
{
    if( arcFile.isOpen() ) {
	QString fullName = dirName;
	QDataStream outStream( &arcFile );

	if( fullName.right( 1 ) != "\\" )
	    fullName += "\\";

	outStream << fullName.latin1();

	return true;
    }
    return false;
}

bool QArchive::writeDir( const QString& dirName, bool includeLastComponent, QString localPath )
{
    if( arcFile.isOpen() ) {
	QFileInfo fi( dirName );

	if( includeLastComponent ) {
	    setDirectory( fi.fileName() );
	}
	QDir dir( dirName );
	const QFileInfoList* dirEntries = dir.entryInfoList();
	QFileInfoListIterator dirIter( *dirEntries );
	QDataStream outStream( &arcFile );
	QFileInfo* pFi;

	dirIter.toLast();
	while( ( pFi = dirIter.current() ) ) {
	    if( pFi->fileName()[0] != '.' ) {
		if( pFi->isDir() )
		    writeDir( pFi->absFilePath(), true, localPath + "/" + pFi->fileName() ); // Subdirs should always get its name in the archive.
		else
		    writeFile( pFi->absFilePath(), localPath );
	    }
	    --dirIter;
	}
	setDirectory( ".." );
	return true;
    }
    return false;
}

bool QArchive::writeFileList( const QStringList fileList )
{
    for( QStringList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it ) {
	if( !writeFile( (*it) ) )
	    return false;
    }
    return true;
}

bool QArchive::writeDirList( const QStringList dirList, bool includeLastComponent )
{
    for( QStringList::ConstIterator it = dirList.begin(); it != dirList.end(); ++it ) {
	QString lastComponent = (*it).mid( (*it).findRev( "/" ) + 1 );
	if( !writeDir( (*it), includeLastComponent, lastComponent ) )
	    return false;
    }
    return true;
}

void QArchive::setVerbosity( int verbosity )
{
    verbosityMode = verbosity;
}
