/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qnetworkprotocol.h#1 $
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

#ifndef QNETWORKPROTOCOL_H
#define QNETWORKPROTOCOL_H

#include "qurl.h"
#include "qurlinfo.h"

#include <qstring.h>
#include <qmap.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qobject.h>

class QNetworkProtocol;

extern Q_EXPORT QMap< QString, QNetworkProtocol* > *qNetworkProtocolRegister;

class QNetworkProtocol : public QObject
{
    Q_OBJECT

public:
    enum ConnectionState {
	HostFound = 0,
	Connected,
	Closed,
	DataHostFound,
	DataConnected,
	DataClosed,
	Error
    };


    QNetworkProtocol();
    virtual ~QNetworkProtocol();

    virtual void openConnection( QUrl *u );
    virtual bool isOpen();
    virtual void close();
    virtual void setUrl( QUrl *u );

    virtual void put( const QCString &data );

    virtual QNetworkProtocol *copy() const;

    static void registerNetworkProtocol( const QString &protocol, QNetworkProtocol *nprotocol );
    static QNetworkProtocol *getNetworkProtocol( const QString &protocol );
    
signals:
    void error( int ecode, const QString &msg );
    void data( const QCString & );
    void putSuccessful( const QString & );
    void connectionStateChanged( int state, const QString &data );

protected:
    QUrl *url;

};

class QNetworkFileAccess : public QNetworkProtocol
{
    Q_OBJECT

public:
    QNetworkFileAccess();
    virtual ~QNetworkFileAccess();

    virtual void listEntries( const QString &nameFilter, int filterSpec = QDir::DefaultFilter,
			      int sortSpec = QDir::DefaultSort );
    virtual void mkdir( const QString &dirname );
    virtual void remove( const QString &filename );
    virtual void rename( const QString &oldname, const QString &newname );
    virtual void copy( const QStringList &files, const QString &dest, bool move );
    virtual void isUrlDir();
    virtual void isUrlFile();

    virtual QNetworkProtocol *copy() const;

signals:
    void entry( const QUrlInfo & );
    void finished( int );
    void start( int );
    void createdDirectory( const QUrlInfo & );
    void removed( const QString & );
    void itemChanged( const QString &oldname, const QString &newname );
    void urlIsDir();
    void urlIsFile();
    void copyProgress( const QString &, const QString &,
		       int step, int total );


};

#endif
