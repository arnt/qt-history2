/****************************************************************************
** $Id: $
**
** Definition of QHtpp and related classes.
**
** Created : 970521
**
** Copyright (C) 1997-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QHTTP_H
#define QHTTP_H

#ifndef QT_H
#include "qobject.h"
#include "qnetworkprotocol.h"
#endif // QT_H

#if !defined( QT_MODULE_NETWORK ) || defined( QT_LICENSE_PROFESSIONAL ) || defined( QT_INTERNAL_NETWORK )
#define QM_EXPORT_HTTP
#else
#define QM_EXPORT_HTTP Q_EXPORT
#endif

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

class QSocket;
class QTimerEvent;
class QTextStream;
class QIODevice;

class QHttpReplyHeader;
class QHttpClient;
class QHttpPrivate;

class QM_EXPORT_HTTP QHttp : public QNetworkProtocol
{
    Q_OBJECT

public:
    QHttp();
    virtual ~QHttp();

    int supportedOperations() const;

protected:
    void operationGet( QNetworkOperation *op );
    void operationPut( QNetworkOperation *op );

private slots:
    void reply( const QHttpReplyHeader & rep, const QByteArray & dataA );
    void requestFinished();
    void requestFailed( int );
    void connected();
    void closed();
    void hostFound();

private:
    QHttpPrivate *d;
    QHttpClient *client;
    int bytesRead;
};

#endif
#endif
