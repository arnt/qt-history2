/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qnetworkprotocol.h#8 $
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
#include <qdict.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qobject.h>

class QNetworkProtocol;

class QNetworkProtocolFactoryBase
{
public:
   virtual QNetworkProtocol *createObject() = 0;

};

template< class Protocol >
class QNetworkProtocolFactory : public QNetworkProtocolFactoryBase
{
public:
    QNetworkProtocol *createObject() {
	return new Protocol;
    }

};

typedef QDict< QNetworkProtocolFactoryBase > QNetworkProtocolDict;
extern Q_EXPORT QNetworkProtocolDict *qNetworkProtocolRegister;


class QNetworkProtocol : public QObject
{
    Q_OBJECT

public:
    enum ConnectionState {
	ConHostFound = 0,
	ConConnected,
	ConClosed,
	ConDataHostFound,
	ConDataConnected,
	ConDataClosed,
	ConError
    };

    enum Operations {
	OpListEntries = 1,
	OpMkdir = 2,
	OpRemove = 4,
	OpRename = 8,
	OpCopy = 16,
	OpPut = 32,
	OpUrlIsDir = 64
    };

    QNetworkProtocol();
    virtual ~QNetworkProtocol();

    virtual void openConnection( QUrl *u );
    virtual bool isOpen();
    virtual void close();
    virtual void setUrl( QUrl *u );

    virtual void listEntries();
    virtual void mkdir( const QString &dirname );
    virtual void remove( const QString &filename );
    virtual void rename( const QString &oldname, const QString &newname );
    virtual void copy( const QStringList &files, const QString &dest, bool move );
    virtual void put( const QCString &data );

    virtual bool isUrlDir();

    static void registerNetworkProtocol( const QString &protocol,
					 QNetworkProtocolFactoryBase *protocolFactory );
    static QNetworkProtocol *getNetworkProtocol( const QString &protocol );

    virtual int supportedOperations() const;

signals:
    void error( int ecode, const QString &msg );
    void data( const QCString & );
    void putSuccessful( const QCString & );
    void connectionStateChanged( int state, const QString &data );
    void finished( int );
    void start( int );

    void entry( const QUrlInfo & );
    void createdDirectory( const QUrlInfo & );
    void removed( const QString & );
    void itemChanged( const QString &oldname, const QString &newname );
    void copyProgress( const QString &, const QString &,
		       int step, int total );

protected:
    QUrl *url;

};


#endif
