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

#include "qobject.h"
#include "qvaluelist.h"
#include "qsocket.h"

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
	MailServer( const QString & n_, Q_UINT16 p_):n(n_),p(p_){}
	QString name() const { return n; }
	Q_UINT16 priority() const { return p; }
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
	bool operator== ( const MailServer& ) const;
	MailServer();
#endif
    private:
	QString n;
	Q_UINT16 p;
    };

    QValueList<MailServer> mailServers() const;

    class Server {
    public:
	Server(const QString&n_,Q_UINT16 p_,Q_UINT16 w_):n(n_),p(p_),w(w_){}
	QString name() const { return n; }
	Q_UINT16 priority() const { return p; }
	Q_UINT16 weight() const { return w; }
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
	bool operator== ( const Server& ) const;
	Server();
#endif
    private:
	QString n;
	Q_UINT16 p, w;
    };

    QValueList<Server> Servers() const;

    QStringList texts() const;

    QString canonicalName() const; // ### real-world but uncommon: QStringList

    QValueList<QString> qualifiedNames() const { return n; }

signals:
    void resultsReady();

private:
    QString l;
    QValueList<QString> n;
    RecordType t;
    QDnsPrivate * d;
};


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

#endif // QDNS_H
