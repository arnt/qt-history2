/****************************************************************************
** $Id: .emacs,v 1.5 1999/05/06 19:35:46 agulbra Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
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

    // to do the query
    enum Status { Passive, Active, Done };
    void sendQuery() const;
    Status queryStatus() const;

    // to query for replies
    QValueList<QHostAddress> addresses() const;

    class MailServer {
    public:
	MailServer( const QString & n_, Q_UINT16 p_):n(n_),p(p_){}
	QString name() const { return n; }
	Q_UINT16 priority() const { return p; }
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
    private:
	QString n;
	Q_UINT16 p, w;
    };

    QValueList<Server> Servers() const;

    QStringList texts() const;

    QString canonicalName() const; // ### real-world but uncommon: QStringList

    QStringList names() const;

signals:
    void statusChanged();

private:
    QString l;
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

#endif
