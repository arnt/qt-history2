/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlocalfs.cpp#3 $
**
** Implementation of QFileDialog class
**
** Created : 950429
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qlocalfs.h"
#include "qfileinfo.h"
#include "qfile.h"
#include "qurlinfo.h"
#include "qapplication.h"

// NOT REVISED

/*!
 */

QLocalFs::QLocalFs()
    : QNetworkProtocol()
{
}

/*!
 */

void QLocalFs::operationListChildren( QNetworkOperation *op )
{
    op->setState( StInProgress );
	
    dir = QDir( url()->path( FALSE ) );
    dir.setNameFilter( url()->nameFilter() );
    dir.setMatchAllDirs( TRUE );
    if ( !dir.isReadable() ) {
	QString msg = tr( "Could not read directory\n" + url()->path( FALSE ) );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrReadDir );
	emit finished( op );
	return;
    }
	
    const QFileInfoList *filist = dir.entryInfoList( QDir::All | QDir::Hidden );
    if ( !filist ) {
	QString msg = tr( "Could not read directory\n" + url()->path( FALSE ) );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrReadDir );
	emit finished( op );
	return;
    }
	
    emit start( op );
    QFileInfoListIterator it( *filist );
    QFileInfo *fi;
    while ( ( fi = it.current() ) != 0 ) {
	++it;
	QUrlInfo inf( fi->fileName(), 0/*permissions*/, fi->owner(), fi->group(),
		      fi->size(), fi->lastModified(), fi->lastRead(), fi->isDir(), fi->isFile(),
		      fi->isSymLink(), fi->isWritable(), fi->isReadable(), fi->isExecutable() );
	url()->emitNewChild( inf, op );
    }
    op->setState( StDone );
    emit finished( op );
}

/*!
 */

void QLocalFs::operationMkDir( QNetworkOperation *op )
{
    op->setState( StInProgress );
    QString dirname = op->arg1();

    dir = QDir( url()->path( FALSE ) );
    if ( dir.mkdir( dirname ) ) {
	QFileInfo fi( dir, dirname );
	QUrlInfo inf( fi.fileName(), 0/*permissions*/, fi.owner(), fi.group(),
		      fi.size(), fi.lastModified(), fi.lastRead(), fi.isDir(), fi.isFile(),
		      fi.isSymLink(), fi.isWritable(), fi.isReadable(), fi.isExecutable() );
	emit newChild( inf, op );
	op->setState( StDone );
	emit createdDirectory( inf, op );
	emit finished( op );
    } else {
	QString msg = tr( "Could not create directory\n" + dirname );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrCreateDir );
	emit finished( op );
    }
}

/*!
 */

void QLocalFs::operationRemove( QNetworkOperation *op )
{
    op->setState( StInProgress );
    QString name = op->arg1();

    dir = QDir( url()->path( FALSE ) );
    if ( dir.remove( name ) ) {
	op->setState( StDone );
	emit removed( op );
	emit finished( op );
    } else {
	QString msg = tr( "Could not remove file or directory\n" + name );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrRemove );
	emit finished( op );
    }
}

/*!
 */

void QLocalFs::operationRename( QNetworkOperation *op )
{
    op->setState( StInProgress );
    QString oldname = op->arg1();
    QString newname = op->arg2();

    dir = QDir( url()->path( FALSE ) );
    if ( dir.rename( oldname, newname ) ) {
	op->setState( StDone );
	emit itemChanged( op );
	emit finished( op );
    } else {
	QString msg = tr( "Could not rename\n%1\nto\n%2" ).arg( oldname ).arg( newname );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrRename );
	emit finished( op );
    }
}

/*!
 */

void QLocalFs::operationCopy( QNetworkOperation *op )
{
    QString from = QUrl( op->arg1() ).path();
    QString to = QUrl( op->arg2() ).path();

    QFile f( from );
    if ( !f.open( IO_ReadOnly ) ) {
	QString msg = tr( "Could not open\n%1" ).arg( from );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrCopy );
	emit finished( op );
	return;
    }

    QFileInfo fi( from );
    if ( fi.exists() ) {
	if ( !fi.isFile() ) {
	    f.close();
	    QString msg = tr( "Couldn´t write\n%1" ).arg( to );
	    op->setState( StFailed );
	    op->setProtocolDetail( msg );
	    op->setErrorCode( ErrCopy );
	    emit finished( op );
	    return;
	}
    }

    to += "/" + fi.fileName();
    QFile f2( to );
    if ( !f2.open( IO_WriteOnly ) ) {
	f.close();
	QString msg = tr( "Could not write to\n%1" ).arg( to );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrCopy );
	emit finished( op );
	return;
    }

    char *block = new char[ 1024 ];
    bool error = FALSE;
    int sum = 0;
    emit copyProgress( -1, 100, op );
    while ( !f.atEnd() ) {
	int len = f.readBlock( block, 100 );
	if ( len == -1 ) {
	    error = TRUE;
	    break;
	}
	sum += len;
	emit copyProgress( ( sum * 100 ) / f.size(), 100, op );
	f2.writeBlock( block, len );
	qApp->processEvents();
    }

    delete[] block;

    f.close();
    f2.close();

    emit finished( op );
}

int QLocalFs::supportedOperations() const
{
    return OpListChildren | OpMkdir | OpRemove | OpCopy | OpRename | OpMove;
}
