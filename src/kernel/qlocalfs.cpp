/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlocalfs.cpp#15 $
**
** Implementation of QLocalFs class
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
#include "qurloperator.h"

// NOT REVISED

/*!
  \class QLocalFs qlocalfs.h
  \brief Implementation of a QNetworkProtocol which works
  on the local filesystem.

  This class is a subclass of QNetworkProtocol and works
  on the local filesystem. If you want to write a network
  transparent application using QNetworkProtocol,
  QUrlOperator, etc. this class is used for accessing
  the local filesystem by QUrlOperator.

  \sa QUrlOperator, QNetworkProtocol
*/

/*!
  Constructor.
*/

QLocalFs::QLocalFs()
    : QNetworkProtocol()
{
}

/*!
  \reimp
*/

void QLocalFs::operationListChildren( QNetworkOperation *op )
{
    op->setState( StInProgress );
	
    dir = QDir( url()->path() );
    dir.setNameFilter( url()->nameFilter() );
    dir.setMatchAllDirs( TRUE );
    if ( !dir.isReadable() ) {
	QString msg = tr( "Could not read directory\n" + url()->path() );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrListChlidren );
	emit finished( op );
	return;
    }
	
    const QFileInfoList *filist = dir.entryInfoList( QDir::All | QDir::Hidden );
    if ( !filist ) {
	QString msg = tr( "Could not read directory\n" + url()->path() );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrListChlidren );
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
  \reimp
*/

void QLocalFs::operationMkDir( QNetworkOperation *op )
{
    op->setState( StInProgress );
    QString dirname = op->arg1();

    dir = QDir( url()->path() );
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
	op->setErrorCode( ErrMkdir );
	emit finished( op );
    }
}

/*!
  \reimp
*/

void QLocalFs::operationRemove( QNetworkOperation *op )
{
    op->setState( StInProgress );
    QString name = QUrl( op->arg1() ).path();

    dir = QDir( url()->path() );
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
  \reimp
*/

void QLocalFs::operationRename( QNetworkOperation *op )
{
    op->setState( StInProgress );
    QString oldname = op->arg1();
    QString newname = op->arg2();

    dir = QDir( url()->path() );
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
  \reimp
*/

void QLocalFs::operationGet( QNetworkOperation *op )
{
    op->setState( StInProgress );
    QString from = QUrl( op->arg1() ).path();

    QFile f( from );
    if ( !f.open( IO_ReadOnly ) ) {
	QString msg = tr( "Could not open\n%1" ).arg( from );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrGet );
	emit finished( op );
	return;
    }

    QByteArray s;
    s.resize( f.size() );
    f.readBlock( s.data(), f.size() );
    // #### todo progress
    emit dataTransferProgress( f.size(), f.size(), op );
    emit data( s, op );
    op->setState( StDone );
    f.close();
    emit finished( op );
}

/*!
  \reimp
*/

void QLocalFs::operationPut( QNetworkOperation *op )
{
    op->setState( StInProgress );
    QString to = QUrl( op->arg1() ).path();

    QFile f( to );
    if ( !f.open( IO_WriteOnly ) ) {
	QString msg = tr( "Could not write\n%1" ).arg( to );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrPut );
	emit finished( op );
	return;
    }

    f.writeBlock( op->rawArg2(), op->rawArg2().size() );
    // #### todo progress
    emit dataTransferProgress( op->rawArg2().size(), op->rawArg2().size(), op );
    op->setState( StDone );
    f.close();
    emit finished( op );
}

/*!
  \reimp
*/

int QLocalFs::supportedOperations() const
{
    return OpListChildren | OpMkdir | OpRemove | OpRename | OpGet | OpPut;
}
