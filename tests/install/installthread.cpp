#include "installthread.h"
#include "setupwizardimpl.h"
#include "installevent.h"
#include <qfileinfo.h>
#include <qdatastream.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <zlib\zlib.h>
#include <windows.h>
#include <stdlib.h>

#define BUFFERSIZE 65536

InstallThread::InstallThread()
{
    GUI = NULL;
}

bool InstallThread::createDir( QString fullPath )
{
    QStringList hierarchy = QStringList::split( QString( "\\" ), fullPath );
    QString pathComponent, tmpPath;
    QDir dirTmp;
    bool success;

    for( QStringList::Iterator it = hierarchy.begin(); it != hierarchy.end(); ++it ) {
	pathComponent = *it + "\\";
	tmpPath += pathComponent;
	success = dirTmp.mkdir( tmpPath );
    }
    return success;
}

void InstallThread::readArchive( QString arcname, QString installPath )
{
    QDataStream inStream, outStream;
    QFile inFile, outFile;
    QDir outDir;
    QByteArray inBuffer;
    QByteArray outBuffer( BUFFERSIZE );
    z_stream ztream;
    int totalOut;
    bool continueDeCompressing;
    QString entryName, dirName;
    int entryLength;

    inFile.setName( arcname );
    outDir.setPath( QDir::convertSeparators( installPath ) );
    if( inFile.open( IO_ReadOnly ) ) {
	inStream.setDevice( &inFile );
	while( !inStream.atEnd() ) {
	    inStream >> entryLength;
	    totalRead += sizeof( entryLength );
	    inBuffer.resize( entryLength );
	    inStream.readRawBytes( inBuffer.data(), entryLength );
	    totalRead += entryLength;
	    entryName = inBuffer.data();
	    if( entryName.right( 1 ) == "\\" ) {
		if( entryName == "..\\" )
		    outDir.cdUp();
		else {
		    dirName = QDir::convertSeparators( outDir.absPath() + QString( "\\" ) + entryName.left( entryName.length() - 1 ) );
		    if( outDir.exists( dirName ) )
			outDir.cd( dirName );
		    else {
			if( createDir( dirName ) )
			    outDir.cd( dirName );
			else
			    break;
		    }
		}
	    }
	    else
	    {
		totalOut = 0;
		outFile.setName( outDir.absPath() + QString( "\\" ) + entryName );
		
		if( outFile.open( IO_WriteOnly ) ) {
		    if( GUI ) {
				postEvent( GUI, new InstallEvent( InstallEvent::updateProgress, outDir.absPath() + QString( "\\" ) + entryName, totalRead ) );
		    }
		    outStream.setDevice( &outFile );
		    inStream >> entryLength;
		    totalRead += sizeof( entryLength );
		    inBuffer.resize( entryLength );
		    inStream.readRawBytes( inBuffer.data(), entryLength );
		    totalRead += entryLength;
		    ztream.next_in = (unsigned char*)inBuffer.data();
		    ztream.avail_in = entryLength;
		    ztream.total_in = 0;
		    ztream.msg = NULL;
		    ztream.zalloc = (alloc_func)0;
		    ztream.zfree = (free_func)0;
		    ztream.opaque = (voidpf)0;
		    ztream.data_type = Z_BINARY;
		    inflateInit( &ztream );
		    continueDeCompressing = true;
		    while( continueDeCompressing ) {
			ztream.next_out = (unsigned char*)outBuffer.data();
			ztream.avail_out = outBuffer.size();
			ztream.total_out = 0;
			continueDeCompressing = ( inflate( &ztream, Z_NO_FLUSH ) == Z_OK );
			outStream.writeRawBytes( outBuffer.data(), ztream.total_out );
			totalOut += ztream.total_out;
		    }
		    inflateEnd( &ztream );
		    outFile.close();
		}
	    }
	}

	inFile.close();
    }
}

void InstallThread::run()
{
    QStringList args;
    QFileInfo fi;
    int totalSize( 0 );
    totalRead = 0;

    fi.setFile( "qt.arq" );
    if( fi.exists() ) {
	totalSize = fi.size();
    }

    if( GUI->installDocs->isChecked() ) {
	fi.setFile( "doc.arq" );
	if( fi.exists() ) {
	    totalSize += fi.size();
	}
    }

    if( GUI->installExamples->isChecked() ) {
	fi.setFile( "examples.arq" );
	if( fi.exists() ) {
	    totalSize += fi.size();
	}
    }

    if( GUI->installTutorials->isChecked() ) {
	fi.setFile( "tutorial.arq" );
	if( fi.exists() ) {
	    totalSize += fi.size();
	}
    }

    GUI->operationProgress->setTotalSteps( totalSize );

    readArchive( "qt.arq", GUI->installPath->text() );
    if( GUI->installDocs->isChecked() )
	readArchive( "doc.arq", GUI->installPath->text() );
    if( GUI->installExamples->isChecked() )
	readArchive( "examples.arq", GUI->installPath->text() );
    if( GUI->installTutorials->isChecked() )
	readArchive( "tutorial.arq", GUI->installPath->text() );

    postEvent( GUI, new InstallEvent( InstallEvent::moveNext ) );
}
