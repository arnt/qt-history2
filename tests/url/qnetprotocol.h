/****************************************************************************
** $Id: //depot/qt/main/tests/url/qnetprotocol.h#3 $
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
void qRegisterNetworkProtocol( const QString &protocol, QNetworkProtocol *nprotocol );
QNetworkProtocol *qGetNetworkProtocol( const QString &protocol );


class QNetworkProtocol : public QObject
{
    Q_OBJECT

public:
    QNetworkProtocol();
    virtual ~QNetworkProtocol();

    virtual void openConnection( QUrl *u );
    virtual bool isOpen();
    virtual void close();
    virtual void setUrl( QUrl *u );

    virtual void listEntries( const QString &nameFilter, int filterSpec = QDir::DefaultFilter,
			      int sortSpec = QDir::DefaultSort );
    virtual void mkdir( const QString &dirname );
    virtual void remove( const QString &filename );
    virtual void rename( const QString &oldname, const QString &newname );
    virtual void copy( const QStringList &files, const QString &dest, bool move );
    virtual void get();
    virtual void isDir();
    virtual void isFile();
    
    virtual QNetworkProtocol *copy() const;
    virtual QString toString() const;

protected:
    QUrl *url;

};

#endif
