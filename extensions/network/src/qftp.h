/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qftp.h#8 $
**
** Implementation of Network Extension Library
**
** Created : 970521
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

#ifndef QFTP_H
#define QFTP_H

#include "qsocket.h"
#include "qapplication.h"
#include "qstring.h"
#include "qstringlist.h"

#include "qurlinfo.h"
#include "qnetworkprotocol.h"

class QFtp : public QNetworkProtocol
{
    Q_OBJECT

public:
    QFtp();
    virtual ~QFtp();

    virtual void openConnection( QUrl *url );
    virtual bool isOpen();
    virtual void close();
    virtual void listEntries( const QString &nameFilter, int filterSpec = QDir::DefaultFilter,
			      int sortSpec = QDir::DefaultSort );
    virtual void mkdir( const QString &dirname );
    virtual void remove( const QString &filename );
    virtual void rename( const QString &oldname, const QString &newname );
    virtual void copy( const QStringList &files, const QString &dest, bool move );
    virtual void isUrlDir();
    virtual void isUrlFile();

    virtual int supportedOperations() const;

protected:
    enum Command {
	List = 0,
	Mkdir,
	None = -1
    };

    void parseDir( const QString &buffer, QUrlInfo &info );

    QSocket *commandSocket, *dataSocket;
    QString extraData;
    Command command;
    bool connectionReady, passiveMode;
    QString tmp;

protected slots:
    void hostFound();
    void connected();
    void closed();
    void readyRead();
    void dataHostFound();
    void dataConnected();
    void dataClosed();
    void dataReadyRead();

};

#endif
