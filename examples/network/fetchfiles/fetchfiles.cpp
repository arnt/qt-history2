/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "fetchfiles.h"

FetchFiles::FetchFiles( QObject *parent, const char *name ) :
    QObject( parent, name )
{
    currFile = files.begin();
    connect( &urlOp, SIGNAL(start(QNetworkOperation*)), this, SLOT(opStart(QNetworkOperation*)) );
    connect( &urlOp, SIGNAL(finished(QNetworkOperation*)), this, SLOT(opFinished(QNetworkOperation*)) );
    connect( &urlOp, SIGNAL(newChildren(const QValueList<QUrlInfo>&, QNetworkOperation*)), this, SLOT(opNewChildren(const QValueList<QUrlInfo>&, QNetworkOperation*)) );
}

void FetchFiles::fetch( const QString& path, const QString& dest )
{
    urlOp = path;
    destFolder = dest;
    if ( !urlOp.isDir() ) {
	emit error();
	emit finished();
	return;
    }
    urlOp.listChildren();
}

void FetchFiles::stop()
{
    urlOp.stop();
}

void FetchFiles::opStart( QNetworkOperation* op )
{
    if ( op->operation() == QNetworkProtocol::OpListChildren ) {
	files.clear();
	emit start();
    }
}

void FetchFiles::opFinished( QNetworkOperation* op )
{
    if ( op->state() == QNetworkProtocol::StStopped ) {
	files.clear();
	currFile = files.begin();
	emit finished();
    } else if ( op->state() != QNetworkProtocol::StDone ) {
	files.clear();
	currFile = files.begin();
	emit error();
	emit finished();
    } else { //StDone
	if ( op->operation() == QNetworkProtocol::OpListChildren ) {
	    currFile = files.begin();
	    if ( currFile != files.end() ) {
		emit startFile( *currFile );
		urlOp.copy( *currFile, destFolder );
	    } else {
		emit finished();
	    }	
	} else if ( op->operation() == QNetworkProtocol::OpPut ) {
	    emit finishedFile( *currFile );
	    if ( ++currFile != files.end() ) {
		emit startFile( *currFile );
		urlOp.copy( *currFile, destFolder );
	    } else
		emit finished();
	}
    }
}

void FetchFiles::opNewChildren( const QValueList<QUrlInfo>& i, QNetworkOperation* op )
{
    for ( QValueList<QUrlInfo>::const_iterator it = i.begin(); it != i.end(); ++it ) {
	if ( (*it).isValid() && (*it).isFile() && (*it).isReadable() ) {
	    files.append( (*it).name() );
	}
    }
}


