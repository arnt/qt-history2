/****************************************************************************
**
** Definition of Q3Dns class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3DNS_H
#define Q3DNS_H

#ifndef QT_H
#include "qobject.h"
#include "qhostaddress.h"
#include "qsocketnotifier.h"
#include "qstringlist.h"
#endif // QT_H

#if defined(QT_LICENSE_PROFESSIONAL) || defined(QT_INTERNAL_NETWORK)
#define QM_EXPORT_DNS
#else
#define QM_EXPORT_DNS Q_COMPAT_EXPORT
#endif

#ifndef QT_NO_DNS

//#define Q_DNS_SYNCHRONOUS

template<typename T> class QList;

class Q3DnsPrivate;

class QM_EXPORT_DNS Q3Dns: public QObject {
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

    Q3Dns();
    Q3Dns(const QString & label, RecordType rr = A);
    Q3Dns(const QHostAddress & address, RecordType rr = Ptr);
    virtual ~Q3Dns();

    // to set/change the query
    virtual void setLabel(const QString & label);
    virtual void setLabel(const QHostAddress & address);
    QString label() const { return l; }

    virtual void setRecordType(RecordType rr = A);
    RecordType recordType() const { return t; }

    // whether something is happening behind the scenes
    bool isWorking() const;

    // to query for replies
    QList<QHostAddress> addresses() const;

    class QM_EXPORT_DNS MailServer {
    public:
        MailServer(const QString & n=QString::null, Q_UINT16 p=0)
            :name(n), priority(p) {}
        QString name;
        Q_UINT16 priority;
        Q_DUMMY_COMPARISON_OPERATOR(MailServer)
    };
    QList<MailServer> mailServers() const;

    class QM_EXPORT_DNS Server {
    public:
        Server(const QString & n=QString::null, Q_UINT16 p=0, Q_UINT16 w=0, Q_UINT16 po=0)
            : name(n), priority(p), weight(w), port(po) {}
        QString name;
        Q_UINT16 priority;
        Q_UINT16 weight;
        Q_UINT16 port;
        Q_DUMMY_COMPARISON_OPERATOR(Server)
    };
    QList<Server> servers() const;

    QStringList hostNames() const;

    QStringList texts() const;

    QString canonicalName() const; // ### real-world but uncommon: QStringList

    QStringList qualifiedNames() const { return n; }

#if defined(Q_DNS_SYNCHRONOUS)
protected:
    void connectNotify(const char *signal);
#endif

signals:
    void resultsReady();

private slots:
    void startQuery();

private:
    static void doResInit();
    void setStartQueryTimer();
    static QString toInAddrArpaDomain(const QHostAddress &address);
#if defined(Q_DNS_SYNCHRONOUS)
    void doSynchronousLookup();
#endif

    QString l;
    QStringList n;
    RecordType t;
    Q3DnsPrivate * d;

    friend class Q3DnsAnswer;
    friend class Q3DnsManager;
};


// Q3DnsSocket are sockets that are used for DNS lookup

class Q3DnsSocket: public QObject {
    Q_OBJECT
    // note: Private not public.  This class contains NO public API.
protected:
    Q3DnsSocket(QObject *, const char *);
    virtual ~Q3DnsSocket();

private slots:
    virtual void cleanCache();
    virtual void retransmit();
    virtual void answer();
};

#endif // QT_NO_DNS

#endif // Q3DNS_H
