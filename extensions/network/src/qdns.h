/****************************************************************************
** $Id: $
**
** Definition of QDns class.
**
** Created : 991122
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
****************************************************************************/

#ifndef QDNS_H
#define QDNS_H

#ifndef QT_H
#include "qobject.h"
#include "qvaluelist.h"
#include "qsocket.h"
#endif // QT_H

#ifndef QT_NO_DNS

class QDnsPrivate;

class QDns: public QObject {
    Q_OBJECT
public:
    enum RecordType {
	None,
	A, Aaaa,
	Mx, Srv,
	Cname,
	Ptr,
	Txt
    };

    QDns();
    QDns( const QString & label, RecordType rr = A );
    ~QDns();

    // to set/change the query
    virtual void setLabel( const QString & label );
    QString label() const { return l; }

    virtual void setRecordType( RecordType rr = A );
    RecordType recordType() const { return t; }

    // whether something is happening behind the curtains
    bool isWorking() const;

    // to query for replies
    QValueList<QHostAddress> addresses() const;

    class MailServer {
    public:
	MailServer( const QString & n=QString::null, Q_UINT16 p=0 )
	    :name(n), priority(p) {}
	QString name;
	Q_UINT16 priority;
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
	bool operator== ( const MailServer& ) const;
	MailServer();
#endif
    };
    QValueList<MailServer> mailServers() const;

    class Server {
    public:
	Server(const QString & n=QString::null, Q_UINT16 p=0, Q_UINT16 w=0 )
	    : name(n), priority(p), weight(w) {}
	QString name;
	Q_UINT16 priority;
	Q_UINT16 weight;
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
	bool operator== ( const Server& ) const;
	Server();
#endif
    };
    QValueList<Server> servers() const;

    QStringList texts() const;

    QString canonicalName() const; // ### real-world but uncommon: QStringList

    QStringList qualifiedNames() const { return n; }

signals:
    void resultsReady();

private:
    QString l;
    QStringList n;
    RecordType t;
    QDnsPrivate * d;
};


// QDnsSocket are sockets that are used for DNS lookup

class QDnsSocket: public QObject {
    Q_OBJECT
    // note: Private not public.  This class contains NO public API.
protected:
    QDnsSocket( QObject *, const char * );
    ~QDnsSocket();

private slots:
    virtual void cleanCache();
    virtual void retransmit();
    virtual void answer();
};

#endif // QT_NO_DNS

#endif // QDNS_H
