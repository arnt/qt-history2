/****************************************************************************
**
** Implementation of QDns class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include <qapplication.h>

// POSIX Large File Support redefines open -> open64
#if defined(open)
# undef open
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

// Solaris redefines connect -> __xnet_connect with _XOPEN_SOURCE_EXTENDED.
#if defined(connect)
# undef connect
#endif

// UnixWare 7 redefines socket -> _socket
#if defined(socket)
# undef socket
#endif

#include "qdns.h"

#ifndef QT_NO_DNS

#include "qdatetime.h"
#include "qhash.h"
#include "qvector.h"
#include "qstring.h"
#include "qtimer.h"
#include "qcoreapplication.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qsocketdevice.h"
#include "qcleanuphandler.h"
#include <limits.h>

//#define QDNS_DEBUG

static Q_UINT16 id; // ### seeded started by now()


static QDateTime * originOfTime = 0;

static QCleanupHandler<QDateTime> qdns_cleanup_time;

static Q_UINT32 now()
{
    if ( originOfTime )
	return originOfTime->secsTo( QDateTime::currentDateTime() );

    originOfTime = new QDateTime( QDateTime::currentDateTime() );
    ::id = originOfTime->time().msec() * 60 + originOfTime->time().second();
    qdns_cleanup_time.add( &originOfTime );
    return 0;
}


static QList<QHostAddress*> *ns = 0;
static QList<QByteArray> *domains = 0;
static bool ipv6support = FALSE;


class QDnsPrivate {
public:
    QDnsPrivate() : queryTimer( 0 ), noNames(FALSE)
    {
#if defined(Q_DNS_SYNCHRONOUS)
#if defined(Q_OS_UNIX)
	noEventLoop = QCoreApplication::instance()==0 || QCoreApplication::instance()->loopLevel()==0;
#else
	noEventLoop = FALSE;
#endif
#endif
    }
    ~QDnsPrivate()
    {
	delete queryTimer;
    }
private:
    QTimer * queryTimer;
    bool noNames;
#if defined(Q_DNS_SYNCHRONOUS)
    bool noEventLoop;
#endif

    friend class QDns;
    friend class QDnsAnswer;
};


class QDnsRR;
class QDnsDomain;



// QDnsRR is the class used to store a single RR.  QDnsRR can store
// all of the supported RR types.  a QDnsRR is always cached.

// QDnsRR is mostly constructed from the outside.  a but hacky, but
// permissible since the entire class is internal.

class QDnsRR {
public:
    QDnsRR( const QString & label );
    ~QDnsRR();

public:
    QDnsDomain * domain;
    QDns::RecordType t;
    bool nxdomain;
    bool current;
    Q_UINT32 expireTime;
    Q_UINT32 deleteTime;
    // somewhat space-wasting per-type data
    // a / aaaa
    QHostAddress address;
    // cname / mx / srv / ptr
    QString target;
    // mx / srv
    Q_UINT16 priority;
    // srv
    Q_UINT16 weight;
    Q_UINT16 port;
    // txt
    QString text; // could be overloaded into target...
private:

};


class QDnsDomain {
public:
    QDnsDomain( const QString & label );
    ~QDnsDomain();

    static void add( const QString & label, QDnsRR * );
    static QList<QDnsRR *> *cached(const QDns *);

    void take( QDnsRR * );

    void sweep( Q_UINT32 thisSweep );

    bool isEmpty() const { return rrs == 0 || rrs->isEmpty(); }

    QString name() const { return l; }

public:
    QString l;
    QList<QDnsRR *> *rrs;
};


class QDnsQuery: public QTimer { // this inheritance is a very evil hack
public:
    QDnsQuery():
	id( 0 ), t( QDns::None ), step(0), started(0),
	dns(new QHash<void *, void *>) { dns->reserve(17); }
    ~QDnsQuery() { delete dns; }
    Q_UINT16 id;
    QDns::RecordType t;
    QString l;

    uint step;
    Q_UINT32 started;

    QHash<void *, void *> *dns;
};



class QDnsAnswer {
public:
    QDnsAnswer( QDnsQuery * );
    QDnsAnswer( const QByteArray &, QDnsQuery * );
    ~QDnsAnswer();

    void parse();
    void notify();

    bool ok;

private:
    QDnsQuery * query;

    Q_UINT8 * answer;
    int size;
    int pp;

    QList<QDnsRR *> *rrs;

    // convenience
    int next;
    int ttl;
    QString label;
    QDnsRR * rr;

    QString readString();
    void parseA();
    void parseAaaa();
    void parseMx();
    void parseSrv();
    void parseCname();
    void parsePtr();
    void parseTxt();
    void parseNs();
};


QDnsRR::QDnsRR( const QString & label )
    : domain( 0 ), t( QDns::None ),
      nxdomain( FALSE ), current( FALSE ),
      expireTime( 0 ), deleteTime( 0 ),
      priority( 0 ), weight( 0 ), port( 0 )
{
    QDnsDomain::add( label, this );
}


// not supposed to be deleted except by QDnsDomain
QDnsRR::~QDnsRR()
{
    // nothing is necessary
}


// this one just sticks in a NXDomain
QDnsAnswer::QDnsAnswer( QDnsQuery * query_ )
{
    ok = TRUE;

    answer = 0;
    size = 0;
    query = query_;
    pp = 0;
    rrs = new QList<QDnsRR *>;
    rrs->setAutoDelete( FALSE );
    next = size;
    ttl = 0;
    label = QString::null;
    rr = 0;

    QDnsRR * newrr = new QDnsRR( query->l );
    newrr->t = query->t;
    newrr->deleteTime = query->started + 10;
    newrr->expireTime = query->started + 10;
    newrr->nxdomain = TRUE;
    newrr->current = TRUE;
    rrs->append( newrr );
}


QDnsAnswer::QDnsAnswer( const QByteArray& answer_,
			QDnsQuery * query_ )
{
    ok = TRUE;

    answer = (Q_UINT8 *)(answer_.data());
    size = (int)answer_.size();
    query = query_;
    pp = 0;
    rrs = new QList<QDnsRR *>;
    rrs->setAutoDelete( FALSE );
    next = size;
    ttl = 0;
    label = QString::null;
    rr = 0;
}


QDnsAnswer::~QDnsAnswer()
{
    if ( !ok && rrs ) {
	for (int i = 0; i < rrs->count(); ++i)
	    rrs->at(i)->t = QDns::None; // will be deleted soonish
    }
    delete rrs;
}


QString QDnsAnswer::readString()
{
    int p = pp;
    QString r = QString::null;
    Q_UINT8 b;
    for( ;; ) {
	b = 128;
	if ( p >= 0 && p < size )
	    b = answer[p];

	switch( b >> 6 ) {
	case 0:
	    p++;
	    if ( b == 0 ) {
		if ( p > pp )
		    pp = p;
		return r.isNull() ? QString( "." ) : r;
	    }
	    if ( !r.isNull() )
		r += '.';
	    while( b-- > 0 )
		r += QChar( answer[p++] );
	    break;
	default:
	    goto not_ok;
	case 3:
	    int q = ( (answer[p] & 0x3f) << 8 ) + answer[p+1];
	    if ( q >= pp || q >= p )
		goto not_ok;
	    if ( p >= pp )
		pp = p + 2;
	    p = q;
	}
    }
not_ok:
    ok = FALSE;
    return QString::null;
}



void QDnsAnswer::parseA()
{
    if ( next != pp + 4 ) {
#if defined(QDNS_DEBUG)
	qDebug( "QDns: saw %d bytes long IN A for %s",
		next - pp, label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->t = QDns::A;
    rr->address = QHostAddress( ( answer[pp+0] << 24 ) +
				( answer[pp+1] << 16 ) +
				( answer[pp+2] <<  8 ) +
				( answer[pp+3] ) );
#if defined(QDNS_DEBUG)
    qDebug( "QDns: saw %s IN A %s (ttl %d)", label.ascii(),
	    rr->address.toString().ascii(), ttl );
#endif
}


void QDnsAnswer::parseAaaa()
{
    if ( next != pp + 16 ) {
#if defined(QDNS_DEBUG)
	qDebug( "QDns: saw %d bytes long IN Aaaa for %s",
		next - pp, label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->t = QDns::Aaaa;
    rr->address = QHostAddress( answer+pp );
#if defined(QDNS_DEBUG)
    qDebug( "QDns: saw %s IN Aaaa %s (ttl %d)", label.ascii(),
	    rr->address.toString().ascii(), ttl );
#endif
}



void QDnsAnswer::parseMx()
{
    if ( next < pp + 2 ) {
#if defined(QDNS_DEBUG)
	qDebug( "QDns: saw %d bytes long IN MX for %s",
		next - pp, label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->priority = (answer[pp] << 8) + answer[pp+1];
    pp += 2;
    rr->target = readString().toLower();
    if ( !ok ) {
#if defined(QDNS_DEBUG)
	qDebug( "QDns: saw bad string in MX for %s", label.ascii() );
#endif
	return;
    }
    rr->t = QDns::Mx;
#if defined(QDNS_DEBUG)
    qDebug( "QDns: saw %s IN MX %d %s (ttl %d)", label.ascii(),
	    rr->priority, rr->target.ascii(), ttl );
#endif
}


void QDnsAnswer::parseSrv()
{
    if ( next < pp + 6 ) {
#if defined(QDNS_DEBUG)
	qDebug( "QDns: saw %d bytes long IN SRV for %s",
		next - pp, label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->priority = (answer[pp] << 8) + answer[pp+1];
    rr->weight = (answer[pp+2] << 8) + answer[pp+3];
    rr->port = (answer[pp+4] << 8) + answer[pp+5];
    pp += 6;
    rr->target = readString().toLower();
    if ( !ok ) {
#if defined(QDNS_DEBUG)
	qDebug( "QDns: saw bad string in SRV for %s", label.ascii() );
#endif
	return;
    }
    rr->t = QDns::Srv;
#if defined(QDNS_DEBUG)
    qDebug( "QDns: saw %s IN SRV %d %d %d %s (ttl %d)", label.ascii(),
	    rr->priority, rr->weight, rr->port, rr->target.ascii(), ttl );
#endif
}


void QDnsAnswer::parseCname()
{
    QString target = readString().toLower();
    if ( !ok ) {
#if defined(QDNS_DEBUG)
	qDebug( "QDns: saw bad cname for for %s", label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->t = QDns::Cname;
    rr->target = target;
#if defined(QDNS_DEBUG)
    qDebug( "QDns: saw %s IN CNAME %s (ttl %d)", label.ascii(),
	    rr->target.ascii(), ttl );
#endif
}


void QDnsAnswer::parseNs()
{
#if defined(QDNS_DEBUG)
    QString target =
#endif
	readString().toLower();
    if ( !ok ) {
#if defined(QDNS_DEBUG)
	qDebug( "QDns: saw bad cname for for %s", label.ascii() );
#endif
	return;
    }

    // parse, but ignore

#if defined(QDNS_DEBUG)
    qDebug( "QDns: saw %s IN NS %s (ttl %d)", label.ascii(),
	    target.ascii(), ttl );
#endif
}


void QDnsAnswer::parsePtr()
{
    QString target = readString().toLower();
    if ( !ok ) {
#if defined(QDNS_DEBUG)
	qDebug( "QDns: saw bad PTR for for %s", label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->t = QDns::Ptr;
    rr->target = target;
#if defined(QDNS_DEBUG)
    qDebug( "QDns: saw %s IN PTR %s (ttl %d)", label.ascii(),
	    rr->target.ascii(), ttl );
#endif
}


void QDnsAnswer::parseTxt()
{
    QString text = readString();
    if ( !ok ) {
#if defined(QDNS_DEBUG)
	qDebug( "QDns: saw bad TXT for for %s", label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->t = QDns::Txt;
    rr->text = text;
#if defined(QDNS_DEBUG)
    qDebug( "QDns: saw %s IN TXT \"%s\" (ttl %d)", label.ascii(),
	    rr->text.ascii(), ttl );
#endif
}


void QDnsAnswer::parse()
{
    // okay, do the work...
    if ( (answer[2] & 0x78) != 0 ) {
#if defined(QDNS_DEBUG)
	qDebug( "DNS Manager: answer to wrong query type (%d)", answer[1] );
#endif
	ok = FALSE;
	return;
    }

    // AA
    bool aa = (answer[2] & 4) != 0;

    // TC
    if ( (answer[2] & 2) != 0 ) {
#if defined(QDNS_DEBUG)
	qDebug( "DNS Manager: truncated answer; pressing on" );
#endif
    }

    // RD
    bool rd = (answer[2] & 1) != 0;

    // we don't test RA
    // we don't test the MBZ fields

    if ( (answer[3] & 0x0f) == 3 ) {
#if defined(QDNS_DEBUG)
	qDebug( "DNS Manager: saw NXDomain for %s", query->l.ascii() );
#endif
	// NXDomain.  cache that for one minute.
	rr = new QDnsRR( query->l );
	rr->t = query->t;
	rr->deleteTime = query->started + 60;
	rr->expireTime = query->started + 60;
	rr->nxdomain = TRUE;
	rr->current = TRUE;
	rrs->append( rr );
	return;
    }

    if ( (answer[3] & 0x0f) != 0 ) {
#if defined(QDNS_DEBUG)
	qDebug( "DNS Manager: error code %d", answer[3] & 0x0f );
#endif
	ok = FALSE;
	return;
    }

    int qdcount = ( answer[4] << 8 ) + answer[5];
    int ancount = ( answer[6] << 8 ) + answer[7];
    int nscount = ( answer[8] << 8 ) + answer[9];
    int adcount = (answer[10] << 8 ) +answer[11];

    pp = 12;

    // read query
    while( qdcount > 0 && pp < size ) {
	// should I compare the string against query->l?
	(void)readString();
	if ( !ok )
	    return;
	pp += 4;
	qdcount--;
    }

    // answers and stuff
    int rrno = 0;
    // if we parse the answer completely, but there are no answers,
    // ignore the entire thing.
    int answers = 0;
    while( ( rrno < ancount ||
	     ( ok && answers >0 && rrno < ancount + nscount + adcount ) ) &&
	   pp < size ) {
	label = readString().toLower();
	if ( !ok )
	    return;
	int rdlength = 0;
	if ( pp + 10 <= size )
	    rdlength = ( answer[pp+8] << 8 ) + answer[pp+9];
	if ( pp + 10 + rdlength > size ) {
#if defined(QDNS_DEBUG)
	    qDebug( "DNS Manager: ran out of stuff to parse (%d+%d>%d (%d)",
		    pp, rdlength, size, rrno < ancount );
#endif
	    // if we're still in the AN section, we should go back and
	    // at least down the TTLs.  probably best to invalidate
	    // the results.
	    // the rrs list is good for this
	    ok = ( rrno < ancount );
	    return;
	}
	uint type, clas;
	type = ( answer[pp+0] << 8 ) + answer[pp+1];
	clas = ( answer[pp+2] << 8 ) + answer[pp+3];
	ttl = ( answer[pp+4] << 24 ) + ( answer[pp+5] << 16 ) +
	      ( answer[pp+6] <<  8 ) + answer[pp+7];
	pp = pp + 10;
	if ( clas != 1 ) {
#if defined(QDNS_DEBUG)
	    qDebug( "DNS Manager: class %d (not internet) for %s",
		    clas, label.isNull() ? "." : label.ascii() );
#endif
	} else {
	    next = pp + rdlength;
	    rr = 0;
	    switch( type ) {
	    case 1:
		parseA();
		break;
	    case 28:
		parseAaaa();
		break;
	    case 15:
		parseMx();
		break;
	    case 33:
		parseSrv();
		break;
	    case 5:
		parseCname();
		break;
	    case 12:
		parsePtr();
		break;
	    case 16:
		parseTxt();
		break;
	    case 2:
		parseNs();
		break;
	    default:
		// something we don't know
#if defined(QDNS_DEBUG)
		qDebug( "DNS Manager: type %d for %s", type,
			label.isNull() ? "." : label.ascii() );
#endif
		break;
	    }
	    if ( rr ) {
		rr->deleteTime = 0;
		if ( ttl > 0 )
		    rr->expireTime = query->started + ttl;
		else
		    rr->expireTime = query->started + 20;
		if ( rrno < ancount ) {
		    answers++;
		    rr->deleteTime = rr->expireTime;
		}
		rr->current = TRUE;
		rrs->append( rr );
	    }
	}
	if ( !ok )
	    return;
	pp = next;
	next = size;
	rrno++;
    }
    if ( answers == 0 ) {
#if defined(QDNS_DEBUG)
	qDebug( "DNS Manager: answer contained no answers" );
#endif
	ok = ( aa && rd );
    }

    // now go through the list and mark all the As that are referenced
    // by something we care about.  we want to cache such As.
    QHash<QString, void *> used; used.reserve(17);
    used.setAutoDelete( FALSE );

    for (int i = 0; i < rrs->count(); ++i) {
	rr = rrs->at(i);
	if ( rr->target.length() && rr->deleteTime > 0 && rr->current )
	    used.insert( rr->target, (void*)42 );
	if ( ( rr->t == QDns::A || rr->t == QDns::Aaaa ) &&
	     used.find( rr->domain->name() ) != 0 )
	    rr->deleteTime = rr->expireTime;
    }

    // next, for each RR, delete any older RRs that are equal to it
    for (int i = 0; i < rrs->count(); ++i) {
	rr = rrs->at(i);
	if ( rr && rr->domain && rr->domain->rrs ) {
	    for (int j = 0; j < rr->domain->rrs->count(); ++j) {
		QDnsRR *older = rr->domain->rrs->at(j);
		if ( older != rr &&
		     older->t == rr->t &&
		     older->nxdomain == rr->nxdomain &&
		     older->address == rr->address &&
		     older->target == rr->target &&
		     older->priority == rr->priority &&
		     older->weight == rr->weight &&
		     older->port == rr->port &&
		     older->text == rr->text ) {
		    // well, it's equal, but it's not the same. so we kill it,
		    // but use its expiry time.
#if defined(QDNS_DEBUG)
		    qDebug( "killing off old %d for %s, expire was %d",
			   older->t, older->domain->name().latin1(),
			   rr->expireTime );
#endif
		    older->t = QDns::None;
		    rr->expireTime = qMax( older->expireTime, rr->expireTime );
		    rr->deleteTime = qMax( older->deleteTime, rr->deleteTime );
		    older->deleteTime = 0;
#if defined(QDNS_DEBUG)
		    qDebug( "    adjusted expire is %d", rr->expireTime );
#endif
		}
	    }
	}
    }

#if defined(QDNS_DEBUG)
    //qDebug( "DNS Manager: ()" );
#endif
}


class QDnsUgleHack: public QDns {
public:
    void ugle( bool emitAnyway=FALSE );
};


void QDnsAnswer::notify()
{
    if ( !rrs || !ok || !query || !query->dns )
	return;

    QHash<void *, void *> notified;
    notified.setAutoDelete( FALSE );

    QHash<void *, void *>::Iterator it = query->dns->begin();
    QDns * dns;
    while (it != query->dns->end()) {
	dns = static_cast<QDns *>(*it);
	++it;
	if (notified.find((void *)dns) == notified.end()) {
	    notified.insert( (void*)dns, (void*)42 );
	    if ( rrs->count() == 0 ) {
#if defined(QDNS_DEBUG)
		qDebug( "DNS Manager: found no answers!" );
#endif
		dns->d->noNames = TRUE;
		((QDnsUgleHack*)dns)->ugle( TRUE );
	    } else {
		QStringList n = dns->qualifiedNames();
		if ( n.contains(query->l) )
		    ((QDnsUgleHack*)dns)->ugle();
#if defined(QDNS_DEBUG)
		else
		    qDebug( "DNS Manager: DNS thing %s not notified for %s",
			    dns->label().ascii(), query->l.ascii() );
#endif
	    }
	}
    }
}


//
//
// QDnsManager
//
//


class QDnsManager: public QDnsSocket {
private:
public: // just to silence the moronic g++.
    QDnsManager();
    ~QDnsManager();
public:
    static QDnsManager * manager();

    QDnsDomain * domain( const QString & );

    void transmitQuery( QDnsQuery * );
    void transmitQuery( int );

    // reimplementation of the slots
    void cleanCache();
    void retransmit();
    void answer();

public:
    QList<QDnsQuery *> queries;
    QHash<QString, QDnsDomain *> cache;
    QSocketDevice * ipv4Socket;
#if !defined (QT_NO_IPV6)
    QSocketDevice * ipv6Socket;
#endif
};



static QDnsManager * globalManager = 0;

static void cleanupDns()
{
    delete globalManager;
    globalManager = 0;
}

QDnsManager * QDnsManager::manager()
{
    if ( !globalManager ) {
        qAddPostRoutine(cleanupDns);
	new QDnsManager();
    }
    return globalManager;
}


void QDnsUgleHack::ugle( bool emitAnyway)
{
    if ( emitAnyway || !isWorking() ) {
#if defined(QDNS_DEBUG)
	qDebug( "DNS Manager: status change for %s (type %d)",
		label().ascii(), recordType() );
#endif
	emit resultsReady();
    }
}


QDnsManager::QDnsManager()
    : QDnsSocket( qApp, "Internal DNS manager" ),
      queries(QList<QDnsQuery *>() ),
      cache(QHash<QString, QDnsDomain *>() ),
      ipv4Socket( new QSocketDevice( QSocketDevice::Datagram, QSocketDevice::IPv4, 0 ) )
#if !defined (QT_NO_IPV6)
      , ipv6Socket( new QSocketDevice( QSocketDevice::Datagram, QSocketDevice::IPv6, 0 ) )
#endif
{
    cache.setAutoDelete( TRUE );
    globalManager = this;

    QTimer * sweepTimer = new QTimer( this );
    sweepTimer->start( 1000 * 60 * 3 );
    connect( sweepTimer, SIGNAL(timeout()),
	     this, SLOT(cleanCache()) );

    QSocketNotifier * rn4 = new QSocketNotifier( ipv4Socket->socket(),
						 QSocketNotifier::Read,
						 this, "dns IPv4 socket watcher" );
    ipv4Socket->setAddressReusable( FALSE );
    ipv4Socket->setBlocking( FALSE );
    connect( rn4, SIGNAL(activated(int)), SLOT(answer()) );

#if !defined (QT_NO_IPV6)
    // Don't connect the IPv6 socket notifier if the host does not
    // support IPv6.
    if ( ipv6Socket->socket() != -1 ) {
	QSocketNotifier * rn6 = new QSocketNotifier( ipv6Socket->socket(),
						     QSocketNotifier::Read,
						     this, "dns IPv6 socket watcher" );

	ipv6support = TRUE;
	ipv6Socket->setAddressReusable( FALSE );
	ipv6Socket->setBlocking( FALSE );
	connect( rn6, SIGNAL(activated(int)), SLOT(answer()) );
    }
#endif

    if ( !ns )
	QDns::doResInit();

    // O(n*n) stuff here.  but for 3 and 6, O(n*n) with a low k should
    // be perfect.  the point is to eliminate any duplicates that
    // might be hidden in the lists.
    QList<QHostAddress *> *ns = new QList<QHostAddress *>;

    for (int i = 0; i < ::ns->count(); ++i) {
	QHostAddress *h = ::ns->at(i);

	if (!ns->contains(h)) {
	    ns->append( new QHostAddress(*h) );
#if defined(QDNS_DEBUG)
	    qDebug( "using name server %s", h->toString().latin1() );
	} else {
	    qDebug( "skipping address %s", h->toString().latin1() );
#endif
	}
    }

    delete ::ns;
    ::ns = ns;
    ::ns->setAutoDelete( TRUE );

    QList<QByteArray> *new_domains = new QList<QByteArray>;
    for(QList<QByteArray>::Iterator it = domains->begin(); it != domains->end(); ++it) {
	bool found = false;
	for(QList<QByteArray>::Iterator it2 = new_domains->begin(); it2 != new_domains->end(); ++it2) {
	    if((*it) == (*it2)) {
		found = true;
		break;
	    }
	}
	if(!found) {
	    new_domains->append((*it));
#if defined(QDNS_DEBUG)
	    qDebug( "searching domain %s", s );
	} else {
	    qDebug( "skipping domain %s", s );
#endif
	}
    }

    delete domains;
    domains = new_domains;
}


QDnsManager::~QDnsManager()
{
    if ( globalManager )
	globalManager = 0;
    queries.setAutoDelete( TRUE );
    cache.setAutoDelete( TRUE );
    delete ipv4Socket;
#if !defined (QT_NO_IPV6)
    delete ipv6Socket;
#endif
}

static Q_UINT32 lastSweep = 0;

void QDnsManager::cleanCache()
{
    bool again = FALSE;
    QHash<QString, QDnsDomain *>::Iterator it = cache.begin();
    QDnsDomain * d;
    Q_UINT32 thisSweep = now();
#if defined(QDNS_DEBUG)
    qDebug( "QDnsManager::cleanCache(: Called, time is %u, last was %u",
	   thisSweep, lastSweep );
#endif

    while (it != cache.end()) {
	d = *it;
	++it;
	d->sweep( thisSweep ); // after this, d may be empty
	if ( !again )
	    again = !d->isEmpty();
    }
    if ( !again )
	delete this;
    lastSweep = thisSweep;
}


void QDnsManager::retransmit()
{
    const QObject * o = sender();
    if ( o == 0 || globalManager == 0 || this != globalManager )
	return;
    uint q = 0;
    while( q < (uint) queries.size() && queries[q] != o )
	q++;
    if ( q < (uint) queries.size() )
	transmitQuery( q );
}


void QDnsManager::answer()
{
    QByteArray a;
    a.resize( 16383 ); // large enough for anything, one suspects

    int r;
#if defined (QT_NO_IPV6)
    r = ipv4Socket->readBlock(a.data(), a.size());
#else
    if (((QSocketNotifier *)sender())->socket() == ipv4Socket->socket())
        r = ipv4Socket->readBlock(a.data(), a.size());
    else
        r = ipv6Socket->readBlock(a.data(), a.size());
#endif
#if defined(QDNS_DEBUG)
#if !defined (QT_NO_IPV6)
    qDebug("DNS Manager: answer arrived: %d bytes from %s:%d", r,
	   useIpv4Socket ? ipv4Socket->peerAddress().toString().ascii()
	   : ipv6Socket->peerAddress().toString().ascii(),
	   useIpv4Socket ? ipv4Socket->peerPort() : ipv6Socket->peerPort() );
#else
    qDebug("DNS Manager: answer arrived: %d bytes from %s:%d", r,
           ipv4Socket->peerAddress().toString().ascii(), ipv4Socket->peerPort());;
#endif
#endif
    if ( r < 12 )
	return;
    // maybe we should check that the answer comes from port 53 on one
    // of our name servers...
    a.resize( r );

    Q_UINT16 aid = (((Q_UINT8)a[0]) << 8) + ((Q_UINT8)a[1]);
    uint i = 0;
    while( i < (uint) queries.size() &&
	   !( queries[i] && queries[i]->id == aid ) )
	i++;
    if ( i == (uint) queries.size() ) {
#if defined(QDNS_DEBUG)
	qDebug( "DNS Manager: bad id (0x%04x) %d", aid, i );
#endif
	return;
    }

    // at this point queries[i] is whatever we asked for.

    if ( ( (Q_UINT8)(a[2]) & 0x80 ) == 0 ) {
#if defined(QDNS_DEBUG)
	qDebug( "DNS Manager: received a query" );
#endif
	return;
    }

    QDnsQuery * q = queries[i];
    QDnsAnswer answer( a, q );
    answer.parse();
    if ( answer.ok ) {
	queries.takeAt( i );
	answer.notify();
	delete q;
    }
}


void QDnsManager::transmitQuery( QDnsQuery * query_ )
{
    if ( !query_ )
	return;

    uint i = 0;
    while( i < (uint) queries.size() && queries[i] != 0 )
	i++;

    queries.insert( i, query_ );
    transmitQuery( i );
}


void QDnsManager::transmitQuery( int i )
{
    if ( i < 0 || i >= queries.size() )
	return;
    QDnsQuery * q = queries[i];

    if ( q && q->step > 8 ) {
	// okay, we've run out of retransmissions. we fake an NXDomain
	// with a very short life time...
	QDnsAnswer answer( q );
	answer.notify();
	// and then get rid of the query
	queries.takeAt( i );
#if defined(QDNS_DEBUG)
	qDebug( "DNS Manager: giving up on query 0x%04x", q->id );
#endif
	delete q;
	QTimer::singleShot( 1000*10, QDnsManager::manager(), SLOT(cleanCache()) );
	// and don't process anything more
	return;
    }

    if ( q && !q->dns || q->dns->isEmpty() )
	// noone currently wants the answer, so there's no point in
	// retransmitting the query. we keep it, though. an answer may
	// arrive for an earlier query transmission, and if it does we
	// may benefit from caching the result.
	return;

    QByteArray p;
    p.resize( 12 + q->l.length() + 2 + 4 );
    if ( p.size() > 500 )
	return; // way over the limit, so don't even try

    // header
    // id
    p[0] = (q->id & 0xff00) >> 8;
    p[1] =  q->id & 0x00ff;
    p[2] = 1; // recursion desired, rest is 0
    p[3] = 0; // all is 0
    // one query
    p[4] = 0;
    p[5] = 1;
    // no answers, name servers or additional data
    p[6] = p[7] = p[8] = p[9] = p[10] = p[11] = 0;

    // the name is composed of several components.  each needs to be
    // written by itself... so we write...
    // oh, and we assume that there's no funky characters in there.
    int pp = 12;
    int lp = 0;
    while( lp < q->l.length() ) {
	int le = q->l.indexOf( '.', lp );
	if ( le < 0 )
	    le = q->l.length();
	QString component = q->l.mid( lp, le-lp );
	p[pp++] = component.length();
	int cp;
	for( cp=0; cp < (int)component.length(); cp++ )
	    p[pp++] = component[cp].latin1();
	lp = le + 1;
    }
    // final null
    p[pp++] = 0;
    // query type
    p[pp++] = 0;
    switch( q->t ) {
    case QDns::A:
	p[pp++] = 1;
	break;
    case QDns::Aaaa:
	p[pp++] = 28;
	break;
    case QDns::Mx:
	p[pp++] = 15;
	break;
    case QDns::Srv:
	p[pp++] = 33;
	break;
    case QDns::Cname:
	p[pp++] = 5;
	break;
    case QDns::Ptr:
	p[pp++] = 12;
	break;
    case QDns::Txt:
	p[pp++] = 16;
	break;
    default:
	p[pp++] = (char)255; // any
	break;
    }
    // query class (always internet)
    p[pp++] = 0;
    p[pp++] = 1;

    if ( !ns || ns->isEmpty() ) {
	// we don't find any name servers. We fake an NXDomain
	// with a very short life time...
	QDnsAnswer answer( q );
	answer.notify();
	// and then get rid of the query
	queries.takeAt( i );
#if defined(QDNS_DEBUG)
	qDebug( "DNS Manager: no DNS server found on query 0x%04x", q->id );
#endif
	delete q;
	QTimer::singleShot( 1000*10, QDnsManager::manager(), SLOT(cleanCache()) );
	// and don't process anything more
	return;
    }

    int nsindex = q->step & ns->count();
    QHostAddress receiver = *ns->at(nsindex);
    if (receiver.isIPv4Address())
	ipv4Socket->writeBlock(p.data(), pp, receiver, 53);
#if !defined (QT_NO_IPV6)
    else
	ipv6Socket->writeBlock(p.data(), pp, receiver, 53);
#endif
#if defined(QDNS_DEBUG)
    qDebug( "issuing query 0x%04x (%d) about %s type %d to %s",
	    q->id, q->step, q->l.ascii(), q->t,
	    ns->at( q->step % ns->count() )->toString().ascii() );
#endif
    if ( ns->count() > 1 && q->step == 0 && queries.count() == 1 ) {
	// if it's the first time, and we don't have any other
	// outstanding queries, send nonrecursive queries to the other
	// name servers too.
	p[2] = 0;
	++nsindex;
	for (; nsindex < ns->count(); ++nsindex) {
	    QHostAddress receiver = *ns->at(nsindex);
            if (receiver.isIPv4Address())
                ipv4Socket->writeBlock( p, pp, receiver, 53 );
#if !defined (QT_NO_IPV6)
            else
                ipv6Socket->writeBlock( p, pp, receiver, 53 );
#endif
        }
#if defined(QDNS_DEBUG)
	    qDebug( "copying query to %s", server->toString().ascii() );
#endif
    }
    q->step++;
    // some testing indicates that normal dns queries take up to 0.6
    // seconds.  the graph becomes steep around that point, and the
    // number of errors rises... so it seems good to retry at that
    // point.
    q->start(q->step < (uint) ns->count() ? 800 : 1500, true);
}


QDnsDomain * QDnsManager::domain( const QString & label )
{
    QDnsDomain *d = cache.value(label.toLower());
    if ( !d ) {
	d = new QDnsDomain( label );
	cache.insert(label.toLower(), d);
    }
    return d;
}


//
//
// the QDnsDomain class looks after and coordinates queries for QDnsRRs for
// each domain, and the cached QDnsRRs.  (A domain, in DNS terminology, is
// a node in the DNS.  "no", "trolltech.com" and "lupinella.troll.no" are
// all domains.)
//
//


// this is ONLY to be called by QDnsManager::domain().  noone else.
QDnsDomain::QDnsDomain( const QString & label )
{
    l = label;
    rrs = 0;
}


QDnsDomain::~QDnsDomain()
{
    delete rrs;
    rrs = 0;
}


void QDnsDomain::add( const QString & label, QDnsRR * rr )
{
    QDnsDomain * d = QDnsManager::manager()->domain( label );
    if ( !d->rrs ) {
	d->rrs = new QList<QDnsRR *>;
	d->rrs->setAutoDelete( TRUE );
    }
    d->rrs->append( rr );
    rr->domain = d;
}


QList<QDnsRR *> *QDnsDomain::cached(const QDns *r)
{
    QList<QDnsRR *> *l = new QList<QDnsRR *>;

    // test at first if you have to start a query at all
    if ( r->recordType() == QDns::A ) {
	if ( r->label().toLower() == "localhost" ) {
	    // undocumented hack. ipv4-specific. also, may be a memory
	    // leak? not sure. would be better to do this in doResInit(),
	    // anyway.
	    QDnsRR *rrTmp = new QDnsRR( r->label() );
	    rrTmp->t = QDns::A;
	    rrTmp->address = QHostAddress( 0x7f000001 );
	    rrTmp->current = TRUE;
	    l->append( rrTmp );
	    return l;
	}
	QHostAddress tmp;
	if ( tmp.setAddress( r->label() ) ) {
	    QDnsRR *rrTmp = new QDnsRR( r->label() );
	    if ( tmp.isIPv4Address() )
		rrTmp->t = QDns::A;
	    else
		rrTmp->t = QDns::Aaaa;
	    rrTmp->address = tmp;
	    rrTmp->current = TRUE;
	    l->append( rrTmp );
	    return l;
	}
    }
    if ( r->recordType() == QDns::Aaaa ) {
	QHostAddress tmp;
	if ( tmp.setAddress(r->label()) && !tmp.isIp4Addr() ) {
	    QDnsRR *rrTmp = new QDnsRR( r->label() );
	    rrTmp->t = QDns::Aaaa;
	    rrTmp->address = tmp;
	    rrTmp->current = TRUE;
	    l->append( rrTmp );
	    return l;
	}
    }

    // if you reach this point, you have to do the query
    QDnsManager * m = QDnsManager::manager();
    QStringList n = r->qualifiedNames();
    QList<QString>::Iterator it = n.begin();
    QList<QString>::Iterator end = n.end();
    bool nxdomain;
    int cnamecount = 0;
    while( it != end ) {
	QString s = *it++;
	nxdomain = FALSE;
#if defined(QDNS_DEBUG)
	qDebug( "looking at cache for %s (%s %d)",
		s.ascii(), r->label().ascii(), r->recordType() );
#endif
	QDnsDomain * d = m->domain( s );
#if defined(QDNS_DEBUG)
	qDebug( " - found %d RRs", d && d->rrs ? d->rrs->count() : 0 );
#endif
	bool answer = FALSE;
	int i = 0;
	while (d->rrs && i < d->rrs->count()) {
	    QDnsRR *rr = d->rrs->at(i);
	    ++i;

	    if ( rr->t == QDns::Cname
		 && r->recordType() != QDns::Cname
		 && !rr->nxdomain && cnamecount < 16 ) {
		// cname.  if the code is ugly, that may just
		// possibly be because the concept is.
#if defined(QDNS_DEBUG)
		qDebug( "found cname from %s to %s",
			r->label().ascii(), rr->target.ascii() );
#endif
		s = rr->target;
		d = m->domain( s );
		i = 0;
		it = end;
		// we've elegantly moved over to whatever the cname
		// pointed to.  well, not elegantly.  let's remember
		// that we've done something, anyway, so we can't be
		// fooled into an infinte loop as well.
		cnamecount++;
	    } else {
		if ( rr->t == r->recordType() ) {
		    if ( rr->nxdomain )
			nxdomain = TRUE;
		    else
			answer = TRUE;
		    l->append( rr );
		    if ( rr->deleteTime <= lastSweep ) {
			// we're returning something that'll be
			// deleted soon.  we assume that if the client
			// wanted it twice, it'll want it again, so we
			// ask the name server again right now.
			QDnsQuery * query = new QDnsQuery;
			query->started = now();
			query->id = ++::id;
			query->t = rr->t;
			query->l = rr->domain->name();
			// note that here, we don't bother about
			// notification. but we do bother about
			// timeouts: we make sure to use high timeouts
			// and few tramsissions.
			query->step = ns->count();
			QObject::connect( query, SIGNAL(timeout()),
					  QDnsManager::manager(),
					  SLOT(retransmit()) );
			QDnsManager::manager()->transmitQuery( query );
		    }
		}
	    }
	}
	// if we found a positive result, return quickly
	if ( answer && l->count() ) {
#if defined(QDNS_DEBUG)
	    qDebug( "found %d records for %s",
		    l->count(), r->label().ascii() );

	    for (int i = 0; i < l->count(); ++i) {
		QDnsRR *lcur = l->at(i);
		qDebug( "  type %d target %s address %s",
		       lcur->t,
		       lcur->target.latin1(),
		       lcur->address.toString().latin1() );
		++li;
	    }
#endif

	    return l;
	}

#if defined(QDNS_DEBUG)
	if ( nxdomain )
	    qDebug( "found NXDomain %s", s.ascii() );
#endif

	if ( !nxdomain ) {
	    // if we didn't, and not a negative result either, perhaps
	    // we need to transmit a query.
	    uint q = 0;
	    while ( q < (uint) m->queries.size() &&
		    ( m->queries[q] == 0 ||
		      m->queries[q]->t != r->recordType() ||
		      m->queries[q]->l != s ) )
		q++;
	    // we haven't done it before, so maybe we should.  but
	    // wait - if it's an unqualified name, only ask when all
	    // the other alternatives are exhausted.
	    if ( q == (uint) m->queries.size() && ( s.indexOf( '.' ) >= 0 ||
					     (int)l->count() >= n.count()-1 ) ) {
		QDnsQuery * query = new QDnsQuery;
		query->started = now();
		query->id = ++::id;
		query->t = r->recordType();
		query->l = s;
		query->dns->insert((void *)r, (void *)r);
		QObject::connect( query, SIGNAL(timeout()),
				  QDnsManager::manager(), SLOT(retransmit()) );
		QDnsManager::manager()->transmitQuery( query );
	    } else if ( q < (uint) m->queries.size() ) {
		// if we've found an earlier query for the same
		// domain/type, subscribe to its answer
		m->queries[q]->dns->insert((void *)r, (void *)r);
	    }
	}
    }

    return l;
}


void QDnsDomain::sweep( Q_UINT32 thisSweep )
{
    if ( !rrs )
	return;

    QDnsRR * rr;
    for (int i = 0; i < rrs->count(); ++i) {
	rr = rrs->at(i);
	if ( !rr->deleteTime )
	    rr->deleteTime = thisSweep; // will hit next time around

#if defined(QDNS_DEBUG)
	qDebug( "QDns::sweep: %s type %d expires %u %u - %s / %s",
	       rr->domain->name().latin1(), rr->t,
	       rr->expireTime, rr->deleteTime,
	       rr->target.latin1(), rr->address.toString().latin1());
#endif
	if ( rr->current == FALSE ||
	     rr->t == QDns::None ||
	     rr->deleteTime <= thisSweep ||
	     rr->expireTime <= thisSweep )
	    rrs->removeAt(i);
    }

    if ( rrs->isEmpty() ) {
	delete rrs;
	rrs = 0;
    }
}




// the itsy-bitsy little socket class I don't really need except for
// so I can subclass and reimplement the slots.


QDnsSocket::QDnsSocket( QObject * parent, const char * name )
    : QObject( parent, name )
{
    // nothing
}


QDnsSocket::~QDnsSocket()
{
    // nothing
}


void QDnsSocket::cleanCache()
{
    // nothing
}


void QDnsSocket::retransmit()
{
    // nothing
}


void QDnsSocket::answer()
{
    // nothing
}


/*!
    \class QDns qdns.h
    \brief The QDns class provides asynchronous DNS lookups.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module network
    \ingroup io

    Both Windows and Unix provide synchronous DNS lookups; Windows
    provides some asynchronous support too. At the time of writing
    neither operating system provides asynchronous support for
    anything other than hostname-to-address mapping.

    QDns rectifies this shortcoming, by providing asynchronous caching
    lookups for the record types that we expect modern GUI
    applications to need in the near future.

    The class is \e not straightforward to use (although it is much
    simpler than the native APIs); QSocket provides much easier to use
    TCP connection facilities. The aim of QDns is to provide a correct
    and small API to the DNS and nothing more. (We use "correctness"
    to mean that the DNS information is correctly cached, and
    correctly timed out.)

    The API comprises a constructor, functions to set the DNS node
    (the domain in DNS terminology) and record type (setLabel() and
    setRecordType()), the corresponding get functions, an isWorking()
    function to determine whether QDns is working or reading, a
    resultsReady() signal and query functions for the result.

    There is one query function for each RecordType, namely
    addresses(), mailServers(), servers(), hostNames() and texts().
    There are also two generic query functions: canonicalName()
    returns the name you'll presumably end up using (the exact meaning
    of this depends on the record type) and qualifiedNames() returns a
    list of the fully qualified names label() maps to.

    \sa QSocket
*/

/*!
    Constructs a DNS query object with invalid settings for both the
    label and the search type.
*/

QDns::QDns()
{
    d = new QDnsPrivate;
    t = None;
}




/*!
    Constructs a DNS query object that will return record type \a rr
    information about \a label.

    The DNS lookup is started the next time the application enters the
    event loop. When the result is found the signal resultsReady() is
    emitted.

    \a rr defaults to \c A, IPv4 addresses.
*/

QDns::QDns( const QString & label, RecordType rr )
{
    d = new QDnsPrivate;
    t = rr;
    setLabel( label );
    setStartQueryTimer(); // start query the next time we enter event loop
}



/*!
    Constructs a DNS query object that will return record type \a rr
    information about host address \a address. The label is set to the
    IN-ADDR.ARPA domain name. This is useful in combination with the
    \c Ptr record type (e.g. if you want to look up a hostname for a
    given address).

    The DNS lookup is started the next time the application enters the
    event loop. When the result is found the signal resultsReady() is
    emitted.

    \a rr defaults to \c Ptr, that maps addresses to hostnames.
*/

QDns::QDns( const QHostAddress & address, RecordType rr )
{
    d = new QDnsPrivate;
    t = rr;
    setLabel( address );
    setStartQueryTimer(); // start query the next time we enter event loop
}




/*!
    Destroys the DNS query object and frees its allocated resources.
*/

QDns::~QDns()
{
    if ( globalManager ) {
	uint q = 0;
	QDnsManager * m = globalManager;
	while( q < (uint) m->queries.size() ) {
	    QDnsQuery * query=m->queries[q];
	    if ( query && query->dns )
		    (void)query->dns->take( (void*) this );
		q++;
	}

    }

    delete d;
    d = 0;
}




/*!
    Sets this DNS query object to query for information about \a
    label.

    This does not change the recordType(), but its isWorking() status
    will probably change as a result.

    The DNS lookup is started the next time the application enters the
    event loop. When the result is found the signal resultsReady() is
    emitted.
*/

void QDns::setLabel( const QString & label )
{
    l = label;
    d->noNames = FALSE;

    // construct a list of qualified names
    n.clear();
    if ( l.length() > 1 && l[(int)l.length()-1] == '.' ) {
	n.append( l.left( l.length()-1 ).toLower() );
    } else {
	int i = l.length();
	int dots = 0;
	const int maxDots = 2;
	while( i && dots < maxDots ) {
	    if ( l[--i] == '.' )
		dots++;
	}
	if ( dots < maxDots ) {
	    (void)QDnsManager::manager(); // create a QDnsManager, if it is not already there
	    for(QList<QByteArray>::Iterator it = domains->begin(); it != domains->end(); ++it)
		n.append( l.toLower() + "." + (*it).data() );
	}
	n.append( l.toLower() );
    }

#if defined(Q_DNS_SYNCHRONOUS)
    if ( d->noEventLoop ) {
	doSynchronousLookup();
    } else {
	setStartQueryTimer(); // start query the next time we enter event loop
    }
#else
    setStartQueryTimer(); // start query the next time we enter event loop
#endif
#if defined(QDNS_DEBUG)
    qDebug( "QDns::setLabel: %d address(es) for %s", n.count(), l.ascii() );
    int i = 0;
    for( i = 0; i < (int)n.count(); i++ )
	qDebug( "QDns::setLabel: %d: %s", i, n[i].ascii() );
#endif
}


/*!
    \overload

    Sets this DNS query object to query for information about the host
    address \a address. The label is set to the IN-ADDR.ARPA domain
    name. This is useful in combination with the \c Ptr record type
    (e.g. if you want to look up a hostname for a given address).
*/

void QDns::setLabel( const QHostAddress & address )
{
    setLabel( toInAddrArpaDomain( address ) );
}


/*!
    \fn QStringList QDns::qualifiedNames() const

    Returns a list of the fully qualified names label() maps to.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myDns.qualifiedNames();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

*/


/*!
    \fn QString QDns::label() const

    Returns the domain name for which this object returns information.

    \sa setLabel()
*/

/*!
    \enum QDns::RecordType

    This enum type defines the record types QDns can handle. The DNS
    provides many more; these are the ones we've judged to be in
    current use, useful for GUI programs and important enough to
    support right away:

    \value None  No information. This exists only so that QDns can
    have a default.

    \value A  IPv4 addresses. By far the most common type.

    \value Aaaa  IPv6 addresses. So far mostly unused.

    \value Mx  Mail eXchanger names. Used for mail delivery.

    \value Srv  SeRVer names. Generic record type for finding
    servers. So far mostly unused.

    \value Cname  Canonical names. Maps from nicknames to the true
    name (the canonical name) for a host.

    \value Ptr  name PoinTeRs. Maps from IPv4 or IPv6 addresses to hostnames.

    \value Txt  arbitrary TeXT for domains.

    We expect that some support for the
    \link http://www.dns.net/dnsrd/rfc/rfc2535.html RFC-2535 \endlink
    extensions will be added in future versions.
*/

/*!
    Sets this object to query for record type \a rr records.

    The DNS lookup is started the next time the application enters the
    event loop. When the result is found the signal resultsReady() is
    emitted.

    \sa RecordType
*/

void QDns::setRecordType( RecordType rr )
{
    t = rr;
    d->noNames = FALSE;
    setStartQueryTimer(); // start query the next time we enter event loop
}

/*!
  \internal

  Private slot for starting the query.
*/
void QDns::startQuery()
{
    // isWorking() starts the query (if necessary)
    if ( !isWorking() )
	emit resultsReady();
}

/*!
    The three functions QDns::QDns(QString, RecordType),
    QDns::setLabel() and QDns::setRecordType() may start a DNS lookup.
    This function handles setting up the single shot timer.
*/
void QDns::setStartQueryTimer()
{
#if defined(Q_DNS_SYNCHRONOUS)
    if ( !d->queryTimer && !d->noEventLoop )
#else
    if ( !d->queryTimer )
#endif
    {
	// start the query the next time we enter event loop
	d->queryTimer = new QTimer( this );
	connect( d->queryTimer, SIGNAL(timeout()),
		 this, SLOT(startQuery()) );
	d->queryTimer->start( 0, TRUE );
    }
}

/*
    Transforms the host address \a address to the IN-ADDR.ARPA domain
    name. Returns something indeterminate if you're sloppy or
    naughty. This function has an IPv4-specific name, but works for
    IPv6 too.
*/
QString QDns::toInAddrArpaDomain( const QHostAddress &address )
{
    QString s;
    if ( address.isNull() ) {
	// if the address isn't valid, neither of the other two make
	// cases make sense. better to just return.
    } else if ( address.isIp4Addr() ) {
	Q_UINT32 i = address.ip4Addr();
	s.sprintf( "%d.%d.%d.%d.IN-ADDR.ARPA",
		   i & 0xff, (i >> 8) & 0xff, (i>>16) & 0xff, (i>>24) & 0xff );
    } else {
	// RFC 3152. (1886 is deprecated, and clients no longer need to
	// support it, in practice).
	Q_IPV6ADDR i = address.toIPv6Address();
	s = "ip6.arpa";
	uint b = 0;
	while( b < 16 ) {
	    s = QString::number( i.c[b]%16, 16 ) + "." +
		QString::number( i.c[b]/16, 16 ) + "." + s;
	    b++;
	}
    }
    return s;
}


/*!
    \fn QDns::RecordType QDns::recordType() const

    Returns the record type of this DNS query object.

    \sa setRecordType() RecordType
*/

/*!
    \fn void QDns::resultsReady()

    This signal is emitted when results are available for one of the
    qualifiedNames().
*/

/*!
    Returns TRUE if QDns is doing a lookup for this object (i.e. if it
    does not already have the necessary information); otherwise
    returns FALSE.

    QDns emits the resultsReady() signal when the status changes to FALSE.
*/

bool QDns::isWorking() const
{
#if defined(QDNS_DEBUG)
    qDebug( "QDns::isWorking (%s, %d)", l.ascii(), t );
#endif
    if ( t == None )
	return FALSE;

#if defined(Q_DNS_SYNCHRONOUS)
    if ( d->noEventLoop )
	return TRUE;
#endif

    QList<QDnsRR *> *ll = QDnsDomain::cached(this);
    Q_LONG queries = n.count();

    for (int i = 0; i < ll->count(); ++i) {
	if (ll->at(i)->nxdomain) {
	    queries--;
	} else {
	    delete ll;
	    return FALSE;
	}
    }
    delete ll;

    if ( queries <= 0 )
	return FALSE;
    if ( d->noNames )
	return FALSE;
    return TRUE;
}


/*!
    Returns a list of the addresses for this name if this QDns object
    has a recordType() of \c QDns::A or \c QDns::Aaaa and the answer
    is available; otherwise returns an empty list.

    As a special case, if label() is a valid numeric IP address, this
    function returns that address.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QList<QHostAddress> list = myDns.addresses();
    QList<QHostAddress>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

*/

QList<QHostAddress> QDns::addresses() const
{
#if defined(QDNS_DEBUG)
    qDebug( "QDns::addresses (%s)", l.ascii() );
#endif
    QList<QHostAddress> result;
    if ( t != A && t != Aaaa )
	return result;

    QList<QDnsRR *> *cached = QDnsDomain::cached(this);

    for (int i = 0; i < cached->count(); ++i) {
	QDnsRR *rr = cached->at(i);
	if (rr->current && !rr->nxdomain)
	    result.append( rr->address );
    }
    delete cached;
    return result;
}


/*!
    \class QDns::MailServer
    \brief The QDns::MailServer class is  described in QDns::mailServers().
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io

    \internal
*/

/*!
    Returns a list of mail servers if the record type is \c Mx. The
    class \c QDns::MailServer contains the following public variables:
    \list
    \i QString QDns::MailServer::name
    \i Q_UINT16 QDns::MailServer::priority
    \endlist

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QList<QDns::MailServer> list = myDns.mailServers();
    QList<QDns::MailServer>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

*/
QList<QDns::MailServer> QDns::mailServers() const
{
#if defined(QDNS_DEBUG)
    qDebug( "QDns::mailServers (%s)", l.ascii() );
#endif
    QList<QDns::MailServer> result;
    if ( t != Mx )
	return result;

    QList<QDnsRR *> *cached = QDnsDomain::cached(this);
    for (int i = 0; i < cached->count(); ++i) {
	QDnsRR *rr = cached->at(i);
	if (rr->current && !rr->nxdomain) {
	    MailServer ms( rr->target, rr->priority );
	    result.append( ms );
	}
    }
    delete cached;
    return result;
}


/*!
    \class QDns::Server
    \brief The QDns::Server class is described in QDns::servers().
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io

    \internal
*/

/*!
    Returns a list of servers if the record type is \c Srv. The class
    \c QDns::Server contains the following public variables:
    \list
    \i QString QDns::Server::name
    \i Q_UINT16 QDns::Server::priority
    \i Q_UINT16 QDns::Server::weight
    \i Q_UINT16 QDns::Server::port
    \endlist

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QList<QDns::Server> list = myDns.servers();
    QList<QDns::Server>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/
QList<QDns::Server> QDns::servers() const
{
#if defined(QDNS_DEBUG)
    qDebug( "QDns::servers (%s)", l.ascii() );
#endif
    QList<QDns::Server> result;
    if ( t != Srv )
	return result;

    QList<QDnsRR *> *cached = QDnsDomain::cached(this);
    for (int i = 0; i < cached->count(); ++i) {
	QDnsRR *rr = cached->at(i);
	if ( rr->current && !rr->nxdomain ) {
	    Server s( rr->target, rr->priority, rr->weight, rr->port );
	    result.append( s );
	}
    }
    delete cached;
    return result;
}


/*!
    Returns a list of host names if the record type is \c Ptr.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myDns.hostNames();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

*/
QStringList QDns::hostNames() const
{
#if defined(QDNS_DEBUG)
    qDebug( "QDns::hostNames (%s)", l.ascii() );
#endif
    QStringList result;
    if ( t != Ptr )
	return result;

    QList<QDnsRR *> *cached = QDnsDomain::cached(this);
    for (int i = 0; i < cached->count(); ++i) {
	QDnsRR *rr = cached->at(i);
	if ( rr->current && !rr->nxdomain ) {
	    QString str( rr->target );
	    result.append( str );
	}
    }
    delete cached;
    return result;
}


/*!
    Returns a list of texts if the record type is \c Txt.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myDns.texts();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/
QStringList QDns::texts() const
{
#if defined(QDNS_DEBUG)
    qDebug( "QDns::texts (%s)", l.ascii() );
#endif
    QStringList result;
    if ( t != Txt )
	return result;

    QList<QDnsRR *> *cached = QDnsDomain::cached(this);
    for (int i = 0; i < cached->count(); ++i) {
	QDnsRR *rr = cached->at(i);
	if ( rr->current && !rr->nxdomain ) {
	    QString str( rr->text );
	    result.append( str );
	}
    }
    delete cached;
    return result;
}


/*!
    Returns the canonical name for this DNS node. (This works
    regardless of what recordType() is set to.)

    If the canonical name isn't known, this function returns a null
    string.

    The canonical name of a DNS node is its full name, or the full
    name of the target of its CNAME. For example, if l.trolltech.com
    is a CNAME to lillian.troll.no, and the search path for QDns is
    "trolltech.com", then the canonical name for all of "lillian",
    "l", "lillian.troll.no." and "l.trolltech.com" is
    "lillian.troll.no.".
*/

QString QDns::canonicalName() const
{
    // the cname should work regardless of the recordType(), so set the record
    // type temporarily to cname when you look at the cache
    QDns *that = (QDns*) this; // mutable function
    RecordType oldType = t;
    that->t = Cname;
    QList<QDnsRR *> *cached = QDnsDomain::cached(that);

    that->t = oldType;

    for (int i = 0; i < cached->count(); ++i) {
	QDnsRR *rr = cached->at(i);
	if ( rr->current && !rr->nxdomain && rr->domain ) {
	    delete cached;
	    return rr->target;
	}
    }
    delete cached;
    return QString::null;
}

#if defined(Q_DNS_SYNCHRONOUS)
/*! \reimp
*/
void QDns::connectNotify( const char *signal )
{
    if ( d->noEventLoop && qstrcmp(signal,SIGNAL(resultsReady()) )==0 ) {
	doSynchronousLookup();
    }
}
#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_CYGWIN)

#if defined(Q_DNS_SYNCHRONOUS)
void QDns::doSynchronousLookup()
{
    // ### not implemented yet
}
#endif

// the following typedefs are needed for GetNetworkParams() API call
#ifndef IP_TYPES_INCLUDED
#define MAX_HOSTNAME_LEN    128
#define MAX_DOMAIN_NAME_LEN 128
#define MAX_SCOPE_ID_LEN    256
typedef struct {
    char String[4 * 4];
} IP_ADDRESS_STRING, *PIP_ADDRESS_STRING, IP_MASK_STRING, *PIP_MASK_STRING;
typedef struct _IP_ADDR_STRING {
    struct _IP_ADDR_STRING* Next;
    IP_ADDRESS_STRING IpAddress;
    IP_MASK_STRING IpMask;
    DWORD Context;
} IP_ADDR_STRING, *PIP_ADDR_STRING;
typedef struct {
    char HostName[MAX_HOSTNAME_LEN + 4] ;
    char DomainName[MAX_DOMAIN_NAME_LEN + 4];
    PIP_ADDR_STRING CurrentDnsServer;
    IP_ADDR_STRING DnsServerList;
    UINT NodeType;
    char ScopeId[MAX_SCOPE_ID_LEN + 4];
    UINT EnableRouting;
    UINT EnableProxy;
    UINT EnableDns;
} FIXED_INFO, *PFIXED_INFO;
#endif
typedef DWORD (WINAPI *GNP)( PFIXED_INFO, PULONG );

// ### FIXME: this code is duplicated in qfiledialog.cpp
static QString getWindowsRegString( HKEY key, const QString &subKey )
{
    QString s;
    QT_WA( {
	char buf[1024];
	DWORD bsz = sizeof(buf);
	int r = RegQueryValueEx( key, (TCHAR*)subKey.ucs2(), 0, 0, (LPBYTE)buf, &bsz );
	if ( r == ERROR_SUCCESS ) {
	    s = QString::fromUcs2( (unsigned short *)buf );
	} else if ( r == ERROR_MORE_DATA ) {
	    char *ptr = new char[bsz+1];
	    r = RegQueryValueEx( key, (TCHAR*)subKey.ucs2(), 0, 0, (LPBYTE)ptr, &bsz );
	    if ( r == ERROR_SUCCESS )
		s = ptr;
	    delete [] ptr;
	}
    } , {
	char buf[512];
	DWORD bsz = sizeof(buf);
	int r = RegQueryValueExA( key, subKey.local8Bit(), 0, 0, (LPBYTE)buf, &bsz );
	if ( r == ERROR_SUCCESS ) {
	    s = buf;
	} else if ( r == ERROR_MORE_DATA ) {
	    char *ptr = new char[bsz+1];
	    r = RegQueryValueExA( key, subKey.local8Bit(), 0, 0, (LPBYTE)ptr, &bsz );
	    if ( r == ERROR_SUCCESS )
		s = ptr;
	    delete [] ptr;
	}
    } );
    return s;
}

static bool getDnsParamsFromRegistry( const QString &path,
	QString *domainName, QString *nameServer, QString *searchList )
{
    HKEY k;
    int r;
    QT_WA( {
	r = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			  (TCHAR*)path.ucs2(),
			  0, KEY_READ, &k );
    } , {
	r = RegOpenKeyExA( HKEY_LOCAL_MACHINE,
			   path,
			   0, KEY_READ, &k );
    } );

    if ( r == ERROR_SUCCESS ) {
	*domainName = getWindowsRegString( k, "DhcpDomain" );
	if ( domainName->isEmpty() )
	    *domainName = getWindowsRegString( k, "Domain" );

	*nameServer = getWindowsRegString( k, "DhcpNameServer" );
	if ( nameServer->isEmpty() )
	    *nameServer = getWindowsRegString( k, "NameServer" );

	*searchList = getWindowsRegString( k, "SearchList" );
    }
    RegCloseKey( k );
    return r == ERROR_SUCCESS;
}

void QDns::doResInit()
{
    char separator = 0;

    if ( ns )
	return;
    ns = new QList<QHostAddress *>;
    ns->setAutoDelete( TRUE );
    domains = new QList<QByteArray>;

    QString domainName, nameServer, searchList;

    bool gotNetworkParams = FALSE;
    // try the API call GetNetworkParams() first and use registry lookup only
    // as a fallback
#ifdef Q_OS_TEMP
    HINSTANCE hinstLib = LoadLibraryW( L"iphlpapi" );
#else
    HINSTANCE hinstLib = LoadLibraryA( "iphlpapi" );
#endif
    if ( hinstLib != 0 ) {
#ifdef Q_OS_TEMP
	GNP getNetworkParams = (GNP) GetProcAddressW( hinstLib, L"GetNetworkParams" );
#else
	GNP getNetworkParams = (GNP) GetProcAddress( hinstLib, "GetNetworkParams" );
#endif
	if ( getNetworkParams != 0 ) {
	    ULONG l = 0;
	    DWORD res;
	    res = getNetworkParams( 0, &l );
	    if ( res == ERROR_BUFFER_OVERFLOW ) {
		FIXED_INFO *finfo = (FIXED_INFO*)new char[l];
		res = getNetworkParams( finfo, &l );
		if ( res == ERROR_SUCCESS ) {
		    domainName = finfo->DomainName;
		    nameServer = "";
		    IP_ADDR_STRING *dnsServer = &finfo->DnsServerList;
		    while ( dnsServer != 0 ) {
			nameServer += dnsServer->IpAddress.String;
			dnsServer = dnsServer->Next;
			if ( dnsServer != 0 )
			    nameServer += " ";
		    }
		    searchList = "";
		    separator = ' ';
		    gotNetworkParams = TRUE;
		}
		delete[] finfo;
	    }
	}
	FreeLibrary( hinstLib );
    }
    if ( !gotNetworkParams ) {
	if ( getDnsParamsFromRegistry(
	    QString( "System\\CurrentControlSet\\Services\\Tcpip\\Parameters" ),
		    &domainName, &nameServer, &searchList )) {
	    // for NT
	    separator = ' ';
	} else if ( getDnsParamsFromRegistry(
	    QString( "System\\CurrentControlSet\\Services\\VxD\\MSTCP" ),
		    &domainName, &nameServer, &searchList )) {
	    // for 95/98
	    separator = ',';
	} else {
	    // Could not access the TCP/IP parameters
	    domainName = "";
	    nameServer = "127.0.0.1";
	    searchList = "";
	    separator = ' ';
	}
    }

    nameServer = nameServer.simplifyWhiteSpace();
    int first, last;
    if ( !nameServer.isEmpty() ) {
	first = 0;
	do {
	    last = nameServer.find( separator, first );
	    if ( last < 0 )
		last = nameServer.length();
	    QDns tmp( nameServer.mid( first, last-first ), QDns::A );
	    QList<QHostAddress> address = tmp.addresses();
	    Q_LONG i = address.count();
	    while( i )
		ns->append( new QHostAddress(address[--i]) );
	    first = last+1;
	} while( first < (int)nameServer.length() );
    }

    searchList = searchList + " " + domainName;
    searchList = searchList.simplifyWhiteSpace().toLower();
    first = 0;
    do {
	last = searchList.find( separator, first );
	if ( last < 0 )
	    last = searchList.length();
	domains->append( qstrdup( searchList.mid( first, last-first ) ) );
	first = last+1;
    } while( first < (int)searchList.length() );
}

#elif defined(Q_OS_UNIX)

#if defined(Q_DNS_SYNCHRONOUS)
void QDns::doSynchronousLookup()
{
    if ( t!=None && !l.isEmpty() ) {
	QListIterator<QString> it = n.begin();
	QListIterator<QString> end = n.end();
	int type;
	switch( t ) {
	    case QDns::A:
		type = 1;
		break;
	    case QDns::Aaaa:
		type = 28;
		break;
	    case QDns::Mx:
		type = 15;
		break;
	    case QDns::Srv:
		type = 33;
		break;
	    case QDns::Cname:
		type = 5;
		break;
	    case QDns::Ptr:
		type = 12;
		break;
	    case QDns::Txt:
		type = 16;
		break;
	    default:
		type = (char)255; // any
		break;
	}
	while( it != end ) {
	    QString s = *it;
	    it++;
	    QByteArray ba( 512 );
	    int len = res_search( s.latin1(), 1, type, (uchar*)ba.data(), ba.size() );
	    if ( len > 0 ) {
		ba.resize( len );

		QDnsQuery * query = new QDnsQuery;
		query->started = now();
		query->id = ++::id;
		query->t = t;
		query->l = s;
		QDnsAnswer a( ba, query );
		a.parse();
	    } else if ( len == -1 ) {
		// res_search error
	    }
	}
	emit resultsReady();
    }
}
#endif

#if defined(__GLIBC__) && ((__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 3)))
#define Q_MODERN_RES_API
#else
#endif

void QDns::doResInit()
{
    if ( ns )
	return;
    ns = new QList<QHostAddress *>;
    ns->setAutoDelete( TRUE );
    domains = new QList<QByteArray>;

    // read resolv.conf manually.
    QFile resolvConf("/etc/resolv.conf");
    if (resolvConf.open(IO_ReadOnly)) {
        QTextStream stream( &resolvConf );
	QString line;

	while ( !stream.atEnd() ) {
	    line = stream.readLine();
	    QStringList list = QStringList::split( " ", line );
	    const QString type = list[0].toLower();

	    if ( type == "nameserver" ) {
		QHostAddress *address = new QHostAddress();
		if ( address->setAddress( QString(line[1]) ) ) {
		    // only add ipv6 addresses from resolv.conf if
		    // this host supports ipv6.
		    if ( address->isIPv4Address() || ipv6support )
			ns->append( address );
		} else {
		    delete address;
		}
	    } else if ( type == "search" ) {
		QStringList srch = QStringList::split( " ", list[1] );
		for ( QStringList::Iterator i = srch.begin(); i != srch.end(); ++i )
		    domains->append( (*i).toLower().latin1() );

	    } else if ( type == "domain" ) {
		domains->append( list[1].toLower().latin1() );
	    }
	}
    }

    if (ns->isEmpty()) {
#if defined(Q_MODERN_RES_API)
	struct __res_state res;
	res_ninit( &res );
	int i;
	// find the name servers to use
	for( i=0; i < MAXNS && i < res.nscount; i++ )
	    ns->append( new QHostAddress( ntohl( res.nsaddr_list[i].sin_addr.s_addr ) ) );
#  if defined(MAXDFLSRCH)
	for( i=0; i < MAXDFLSRCH; i++ ) {
	    if ( res.dnsrch[i] && *(res.dnsrch[i]) )
		domains->append( QString::fromLatin1(res.dnsrch[i]).toLower().latin1() );
	    else
		break;
	}
#  endif
	if ( *res.defdname )
	    domains->append( QString::fromLatin1( res.defdname ).toLower().latin1() );
#else
	res_init();
	int i;
	// find the name servers to use
	for( i=0; i < MAXNS && i < _res.nscount; i++ )
	    ns->append( new QHostAddress( ntohl( _res.nsaddr_list[i].sin_addr.s_addr ) ) );
#  if defined(MAXDFLSRCH)
	for( i=0; i < MAXDFLSRCH; i++ ) {
	    if ( _res.dnsrch[i] && *(_res.dnsrch[i]) )
		domains->append( _res.dnsrch[i] );
	    else
		break;
	}
#  endif
	if ( *_res.defdname )
	    domains->append( _res.defdname );
#endif

	// the code above adds "0.0.0.0" as a name server at the slightest
	// hint of trouble. so remove those again.
/*
	QList<QHostAddress *> nsi = ns->begin();
	for (; nsi != ns->end(); ++nsi) {
	    if ( (*nsi)->isNull() )
		delete (*nsi)->take();
	    else
		ns->next();
	}
*/
    }

    QFile hosts( QString::fromLatin1( "/etc/hosts" ) );
    if ( hosts.open( IO_ReadOnly ) ) {
	// read the /etc/hosts file, creating long-life A and PTR RRs
	// for the things we find.
	QTextStream i( &hosts );
	QString line;
	while( !i.atEnd() ) {
	    line = i.readLine().simplified().toLower();
	    int n = 0;
	    while( n < line.length() && line[(int)n] != '#' )
		n++;
	    line.truncate( n );
	    n = 0;
	    while( n < line.length() && !line[(int)n].isSpace() )
		n++;
	    QString ip = line.left( n );
	    QHostAddress a;
	    a.setAddress( ip );
	    if ( ( a.isIPv4Address() || a.isIPv6Address() ) && !a.isNull() ) {
		bool first = TRUE;
		line = line.mid( n+1 );
		n = 0;
		while( n < line.length() && !line[(int)n].isSpace() )
		    n++;
		QString hostname = line.left( n );
		// ### in case of bad syntax, hostname is invalid. do we care?
		if ( n ) {
		    QDnsRR * rr = new QDnsRR( hostname );
		    if ( a.isIPv4Address() )
			rr->t = QDns::A;
		    else
			rr->t = QDns::Aaaa;
		    rr->address = a;
		    rr->deleteTime = UINT_MAX;
		    rr->expireTime = UINT_MAX;
		    rr->current = TRUE;
		    if ( first ) {
			first = FALSE;
			QDnsRR * ptr = new QDnsRR( QDns::toInAddrArpaDomain( a ) );
			ptr->t = QDns::Ptr;
			ptr->target = hostname;
			ptr->deleteTime = UINT_MAX;
			ptr->expireTime = UINT_MAX;
			ptr->current = TRUE;
		    }
		}
	    }
	}
    }
}

#endif

#endif // QT_NO_DNS
