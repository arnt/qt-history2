/****************************************************************************
** $Id: //depot/qt/main/tests/url/qftp.h#2 $
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

#ifndef QFTP_H
#define QFTP_H

#include <qsocket.h>
#include <qsocketdevice.h>
#include <qapplication.h>
#include <qstring.h>
#include <sys/socket.h>
#include <qstringlist.h>
#include "qurlinfo.h"

class QFtp : public QObject
{
    Q_OBJECT

public:
    QFtp();
    ~QFtp();
    QFtp &operator=( const QFtp &ftp );
    enum Command {
	List = 0,
	Mkdir
    };
    void read();
    void open( const QString &host_, int port, const QString &path_ = "/",
	       const QString &username_ = "anonymous", const QString &passwd_ = "Qt is cool!",
	       Command cmd = List, const QString &extraData_ = QString::null );
    void close();

protected:
    void parseDir( const QString &buffer, QUrlInfo &info );

    QSocket *commandSocket, *dataSocket;
    QString host;
    QCString buffer;
    QString path, username, passwd;
    Command command;
    QString extraData;

protected slots:
    void hostFound();
    void connected();
    void closed();
    void readyRead();
    void dataHostFound();
    void dataConnected();
    void dataClosed();
    void dataReadyRead();

signals:
    void newEntry( const QUrlInfo & );
    void listFinished();

};

#endif
