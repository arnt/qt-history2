/****************************************************************************
** $Id: //depot/qt/extensions/network/src/qdns.cpp#1 $
**
** Implementation of QDns class.
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
*****************************************************************************/

#include "qdns.h"

#ifndef QT_NO_DNS

#include "qdatetime.h"
#include "qdict.h"
#include "qlist.h"
#include "qstring.h"
#include "qtimer.h"
#include "qapplication.h"
#include "qvector.h"
#include "qstrlist.h"
#include "qptrdict.h"


//#define DEBUG_QDNS


static Q_UINT16 id; // ### start somewhere random


static QDateTime * originOfTime = 0;


static void cleanup()
{
    delete originOfTime;
    originOfTime = 0;
}

static Q_UINT32 now()
{
    if ( originOfTime )
	return originOfTime->secsTo( QDateTime::currentDateTime() );

    originOfTime = new QDateTime( QDateTime::currentDateTime() );
    qAddPostRoutine( cleanup );
    return 0;
}


static QList<QHostAddress> * ns = 0;
static QStrList * domains = 0;

static void doResInit();


class QDnsPrivate {
public:
    QDnsPrivate() : startQueryTimer(FALSE) {}
    ~QDnsPrivate() {}
private:
    bool startQueryTimer;

    friend class QDns;
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
    static QList<QDnsRR> * cached( const QDns * );

    void take( QDnsRR * );

    void sweep();

    bool isEmpty() const { return rrs == 0 || rrs->isEmpty(); }

    QString name() const { return l; }

public:
    QString l;
    QList<QDnsRR> * rrs;
};


class QDnsQuery: public QTimer { // this inheritance is a very evil hack
public:
    QDnsQuery():
	id( 0 ), t( QDns::None ), step(0), started(0),
	dns( new QPtrDict<void>(17) ) {}
    Q_UINT16 id;
    QDns::RecordType t;
    QString l;

    uint step;
    Q_UINT32 started;

    QPtrDict<void> * dns;
};



class QDnsAnswer {
public:
    QDnsAnswer( const QByteArray &, QDnsQuery * );
    ~QDnsAnswer();

    void parse();
    void notify();

    bool ok;

private:
    QDnsQuery * q;

    Q_UINT8 * answer;
    int size;
    int pp;

    QList<QDnsRR> * rrs;

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


QDnsAnswer::QDnsAnswer( const QByteArray& answer_,
			QDnsQuery * query_ )
{
    ok = TRUE;

    answer = (Q_UINT8 *)(answer_.data());
    size = (int)answer_.size();
    q = query_;
    pp = 0;
    rrs = new QList<QDnsRR>;
    rrs->setAutoDelete( FALSE );
    next = size;
    ttl = 0;
    label = QString::null;
    rr = 0;
};


QDnsAnswer::~QDnsAnswer()
{
    if ( !ok && rrs ) {
	QListIterator<QDnsRR> it( *rrs );
	QDnsRR * rr;
	while( (rr=it.current()) != 0 ) {
	    ++it;
	    rr->t = QDns::None; // will be deleted soonish
	}
    }
}


QString QDnsAnswer::readString()
{
    int p = pp;
    QString r = QString::null;
    Q_UINT8 b;
    while( TRUE ) {
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
	    ok = FALSE;
	    return QString::null;
	case 3:
	    int q = ( (answer[p] & 0x3f) << 8 ) + answer[p+1];
	    if ( q >= pp || q >= p ) {
		ok = FALSE;
		return QString::null;
	    }
	    if ( p >= pp )
		pp = p + 2;
	    p = q;
	    break;
	}
    }
}



void QDnsAnswer::parseA()
{
    if ( next != pp + 4 ) {
#if defined(DEBUG_QDNS)
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
#if defined(DEBUG_QDNS)
    qDebug( "QDns: saw %s IN A %s (ttl %d)", label.ascii(),
	    rr->address.toString().ascii(), ttl );
#endif
}


void QDnsAnswer::parseAaaa()
{
    if ( next != pp + 16 ) {
#if defined(DEBUG_QDNS)
	qDebug( "QDns: saw %d bytes long IN Aaaa for %s",
		next - pp, label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->t = QDns::Aaaa;
    rr->address = QHostAddress( answer+pp );
#if defined(DEBUG_QDNS)
    qDebug( "QDns: saw %s IN Aaaa %s (ttl %d)", label.ascii(),
	    rr->address.toString().ascii(), ttl );
#endif
}



void QDnsAnswer::parseMx()
{
    if ( next < pp + 2 ) {
#if defined(DEBUG_QDNS)
	qDebug( "QDns: saw %d bytes long IN MX for %s",
		next - pp, label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->priority = (answer[pp] << 8) + answer[pp+1];
    pp += 2;
    rr->target = readString().lower();
    if ( !ok ) {
#if defined(DEBUG_QDNS)
	qDebug( "QDns: saw bad string in MX for %s", label.ascii() );
#endif
	return;
    }
    rr->t = QDns::Mx;
#if defined(DEBUG_QDNS)
    qDebug( "QDns: saw %s IN MX %d %s (ttl %d)", label.ascii(),
	    rr->priority, rr->target.ascii(), ttl );
#endif
}


void QDnsAnswer::parseSrv()
{
    if ( next < pp + 6 ) {
#if defined(DEBUG_QDNS)
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
    rr->target = readString().lower();
    if ( !ok ) {
#if defined(DEBUG_QDNS)
	qDebug( "QDns: saw bad string in SRV for %s", label.ascii() );
#endif
	return;
    }
    rr->t = QDns::Srv;
#if defined(DEBUG_QDNS)
    qDebug( "QDns: saw %s IN SRV %d %d %d %s (ttl %d)", label.ascii(),
	    rr->priority, rr->weight, rr->port, rr->target.ascii(), ttl );
#endif
}


void QDnsAnswer::parseCname()
{
    QString target = readString().lower();
    if ( !ok ) {
#if defined(DEBUG_QDNS)
	qDebug( "QDns: saw bad cname for for %s", label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->t = QDns::Cname;
    rr->target = target;
#if defined(DEBUG_QDNS)
    qDebug( "QDns: saw %s IN CNAME %s (ttl %d)", label.ascii(),
	    rr->target.ascii(), ttl );
#endif
}


void QDnsAnswer::parseNs()
{
    QString target = readString().lower();
    if ( !ok ) {
#if defined(DEBUG_QDNS)
	qDebug( "QDns: saw bad cname for for %s", label.ascii() );
#endif
	return;
    }

    // parse, but ignore

#if defined(DEBUG_QDNS)
    qDebug( "QDns: saw %s IN NS %s (ttl %d)", label.ascii(),
	    target.ascii(), ttl );
#endif
}


void QDnsAnswer::parsePtr()
{
    QString target = readString().lower();
    if ( !ok ) {
#if defined(DEBUG_QDNS)
	qDebug( "QDns: saw bad PTR for for %s", label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->t = QDns::Ptr;
    rr->target = target;
#if defined(DEBUG_QDNS)
    qDebug( "QDns: saw %s IN PTR %s (ttl %d)", label.ascii(),
	    rr->target.ascii(), ttl );
#endif
}


void QDnsAnswer::parseTxt()
{
    QString text = readString();
    if ( !ok ) {
#if defined(DEBUG_QDNS)
	qDebug( "QDns: saw bad TXT for for %s", label.ascii() );
#endif
	return;
    }

    rr = new QDnsRR( label );
    rr->t = QDns::Txt;
    rr->text = text;
#if defined(DEBUG_QDNS)
    qDebug( "QDns: saw %s IN TXT \"%s\" (ttl %d)", label.ascii(),
	    rr->text.ascii(), ttl );
#endif
}


void QDnsAnswer::parse()
{
    // okay, do the work...
    if ( (answer[2] & 0x78) != 0 ) {
#if defined(DEBUG_QDNS)
	qDebug( "DNS Manager: asnwer to wrong query type (%d)", answer[1] );
#endif
	ok = FALSE;
	return;
    }

    // AA
    bool AA = (answer[2] & 4) != 0;

    // TC
    if ( (answer[2] & 2) != 0 ) {
#if defined(DEBUG_QDNS)
	qDebug( "DNS Manager: truncated answer; pressing on" );
#endif
    }

    // RD
    bool RD = (answer[2] & 1) != 0;

    // we don't test RA
    // we don't test the MBZ fields

    if ( (answer[3] & 0x0f) == 3 ) {
#if defined(DEBUG_QDNS)
	qDebug( "DNS Manager: saw NXDomain for %s", q->l.ascii() );
#endif
	// NXDomain.  cache that for... how long?
	rr = new QDnsRR( q->l );
	rr->t = q->t;
	rr->deleteTime = q->started + 300;
	rr->expireTime = q->started + 300;
	rr->nxdomain = TRUE;
	rr->current = TRUE;
	rrs->append( rr );
	return;
    }

    if ( (answer[3] & 0x0f) != 0 ) {
#if defined(DEBUG_QDNS)
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
	// should I compare the string against q->l?
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
    while( rrno < ancount + nscount + adcount && pp < size ) {
	label = readString().lower();
	if ( !ok )
	    return;
	int rdlength = 0;
	if ( pp + 10 <= size )
	    rdlength = ( answer[pp+8] << 8 ) + answer[pp+9];
	if ( pp + 10 + rdlength > size ) {
#if defined(DEBUG_QDNS)
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
#if defined(DEBUG_QDNS)
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
#if defined(DEBUG_QDNS)
		qDebug( "DNS Manager: type %d for %s", type,
			label.isNull() ? "." : label.ascii() );
#endif
		break;
	    }
	    if ( rr ) {
		if ( rrno < ancount )
		    answers++;
		rr->expireTime = q->started + ttl;
		rr->deleteTime = ( rrno < ancount || ttl < 600)
				 ? q->started + ttl : 0;
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
#if defined(DEBUG_QDNS)
	qDebug( "DNS Manager: answer contained no answers" );
#endif
	if( AA && RD ) {
	    // there are really no answers
	    ok = TRUE;
	} else {
	    ok = FALSE;
	}
    }

#if defined(DEBUG_QDNS)
    //qDebug( "DNS Manager: ()" );
#endif
}


class QDnsUgleHack: public QDns {
public:
    void ugle( bool emitAnyway=FALSE );
};


void QDnsAnswer::notify()
{
    if ( !rrs || !ok || !q || !q->dns )
	return;

    QPtrDict<void> notified;
    notified.setAutoDelete( FALSE );

    QPtrDictIterator<void> it( *q->dns );
    QDns * dns;
    it.toFirst();
    while( (dns=(QDns*)(it.current())) != 0 ) {
	++it;
	if ( notified.find( (void*)dns ) == 0 && q->dns->find( (void*)dns ) != 0 ) {
	    notified.insert( (void*)dns, (void*)42 );
	    if( rrs->count() == 0 ) {
#if defined(DEBUG_QDNS)
		qDebug( "DNS Manager: found no answers!" );
#endif
		((QDnsUgleHack*)dns)->ugle( TRUE );
	    } else {
		QStringList n = dns->qualifiedNames();
		int i = n.count();
		bool found = FALSE;
		while( i-- > 0 && !found ) // ######## O(n*n)!! should use iterator!
		    if ( n[i] == q->l )
			found = TRUE;
		if ( found )
		    ((QDnsUgleHack*)dns)->ugle();
#if defined(DEBUG_QDNS)
		else
		    qDebug( "DNS Manager: DNS thing %s not notified for %s",
			    dns->label().ascii(), q->l.ascii() );
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
    QVector<QDnsQuery> queries;
    QDict<QDnsDomain> cache;
    QSocketDevice * socket;
};



static QDnsManager * globalManager;


QDnsManager * QDnsManager::manager()
{
    if ( !globalManager )
	new QDnsManager();
    return globalManager;
}


void QDnsUgleHack::ugle( bool emitAnyway)
{
    if ( emitAnyway || !isWorking() ) {
#if defined( DEBUG_QDNS )
	qDebug( "DNS Manager: status change for %s (type %d)",
		label().ascii(), recordType() );
#endif
	emit resultsReady();
    }
}


QDnsManager::QDnsManager()
    : QDnsSocket( qApp, "Internal DNS manager" ),
      queries( QVector<QDnsQuery>( 0 ) ),
      cache( QDict<QDnsDomain>( 83, FALSE ) ),
      socket( new QSocketDevice( QSocketDevice::Datagram ) )
{
    cache.setAutoDelete( TRUE );
    globalManager = this;

    QTimer * sweepTimer = new QTimer( this );
    sweepTimer->start( 1000 * 60 * 5 );
    connect( sweepTimer, SIGNAL(timeout()),
	     this, SLOT(cleanCache()) );

    QSocketNotifier * rn = new QSocketNotifier( socket->socket(),
						QSocketNotifier::Read,
						this, "dns socket watcher" );
    socket->setAddressReusable( FALSE );
    socket->setBlocking( FALSE );
    connect( rn, SIGNAL(activated(int)),
	     this, SLOT(answer()) );

    if ( !ns )
	doResInit();

    // O(n*n) stuff here.  but for 3 and 6, O(n*n) with a low k should
    // be perfect.  the point is to eliminate any duplicates that
    // might be hidden in the lists.
    QList<QHostAddress> * ns = new QList<QHostAddress>;

    ::ns->first();
    QHostAddress * h;
    while( (h=::ns->current()) != 0 ) {
	ns->first();
	while( ns->current() != 0 && !(*ns->current() == *h) )
	    ns->next();
	if ( !ns->current() ) {
	    ns->append( new QHostAddress(*h) );
#if defined(DEBUG_QDNS)
	    qDebug( "using name server %s", h->toString().latin1() );
	} else {
	    qDebug( "skipping address %s", h->toString().latin1() );
#endif
	}
	::ns->next();
    }

    delete ::ns;
    ::ns = ns;
    ::ns->setAutoDelete( TRUE );

    QStrList * domains = new QStrList( TRUE );

    ::domains->first();
    char * s;
    while( (s=::domains->current()) != 0 ) {
	domains->first();
	while( domains->current() != 0 && qstrcmp( domains->current(), s ) )
	    domains->next();
	if ( !domains->current() ) {
	    domains->append( s );
#if defined(DEBUG_QDNS)
	    qDebug( "searching domain %s", s );
	} else {
	    qDebug( "skipping domain %s", s );
#endif
	}
	::domains->next();
    }

    delete ::domains;
    ::domains = domains;
    ::domains->setAutoDelete( TRUE );
}


QDnsManager::~QDnsManager()
{
    if ( globalManager )
	globalManager = 0;
}


void QDnsManager::cleanCache()
{
    bool again = FALSE;
    QDictIterator<QDnsDomain> it( cache );
    QDnsDomain * d;
    while( (d=it.current()) != 0 ) {
	++it;
	d->sweep(); // after this, d may be empty
	if ( !again )
	    again = !d->isEmpty();
    }
    if ( !again )
	delete this;
}


void QDnsManager::retransmit()
{
    const QObject * o = sender();
    if ( o == 0 || globalManager == 0 || this != globalManager )
	return;
    uint q = 0;
    while( q < queries.size() && queries[q] != o )
	q++;
    if ( q < queries.size() )
	transmitQuery( q );
}


void QDnsManager::answer()
{
    QByteArray a( 16383 ); // large enough for anything, one suspects
    int r = socket->readBlock( a.data(), a.size() );
#if defined(DEBUG_QDNS)
    qDebug("DNS Manager: answer arrived: %d bytes from %s:%d", r,
	   socket->peerAddress().toString().ascii(), socket->peerPort() );
#endif
    if ( r < 12 )
	return;
    // maybe we should check that the answer comes from port 53 on one
    // of our name servers...
    a.resize( r );

    int id = (a[0] << 8) + a[1];
    uint i = 0;
    while( i < queries.size() &&
	   !( queries[i] && queries[i]->id == id ) )
	i++;
    if ( i == queries.size() ) {
#if defined(DEBUG_QDNS)
	qDebug( "DNS Manager: bad id (%d)", id );
#endif
	return;
    }

    // at this point queries[i] is whatever we asked for.

    if ( (Q_UINT8)(a[2]) & 0x80 == 0 ) {
#if defined(DEBUG_QDNS)
	qDebug( "DNS Manager: received a query" );
#endif
	return;
    }

    QDnsQuery * q = queries[i];
    queries.take( i );
    QDnsAnswer answer( a, q );
    answer.parse();
    answer.notify();
    if ( answer.ok )
	delete q;
    else
	queries.insert( i, q );
};


void QDnsManager::transmitQuery( QDnsQuery * query )
{
    uint i = 0;
    while( i < queries.size() && queries[i] != 0 )
	i++;
    if ( i == queries.size() )
	queries.resize( i+1 );
    queries.insert( i, query );
    transmitQuery( i );
}


void QDnsManager::transmitQuery( int i )
{
    if ( i < 0 || i >= (int)queries.size() )
	return;
    QDnsQuery * q = queries[i];

    QByteArray p( 12 + q->l.length() + 2 + 4 );
    if ( p.size() > 500 )
	return; // way over the limit, so don't even try

    // header
    // id
    p[0] = q->id >> 8;
    p[1] = q->id & 0xff;
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
    uint lp = 0;
    while( lp < q->l.length() ) {
	int le = q->l.find( '.', lp );
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

    if ( !ns || ns->isEmpty() )
	return;

    socket->writeBlock( p.data(), pp, *ns->at( q->step % ns->count() ), 53 );
#if defined(DEBUG_QDNS)
    qDebug( "issuing query %d about %s type %d to %s",
	    q->id, q->l.ascii(), q->t,
	    ns->at( q->step % ns->count() )->toString().ascii() );
#endif
    if ( ns->count() > 1 && q->step == 0 ) {
	// if it's the first time, send nonrecursive queries to the
	// other name servers too.
	p[2] = 0;
	QHostAddress * server;
	while( (server=ns->next()) != 0 ) {
	    socket->writeBlock( p.data(), pp, *server, 53 );
#if defined(DEBUG_QDNS)
	    qDebug( "copying query to %s", server->toString().ascii() );
#endif
	}
    }
    q->step++;
    // some testing indicates that normal dns queries take up to 0.6
    // seconds.  the graph becomes steep around that point, and the
    // number of errors rises... so it seems good to retry at that
    // point.
    q->start( 600, TRUE );
}


QDnsDomain * QDnsManager::domain( const QString & label )
{
    QDnsDomain * d = cache.find( label );
    if ( !d ) {
	d = new QDnsDomain( label );
	cache.insert( label, d );
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
	d->rrs = new QList<QDnsRR>;
	d->rrs->setAutoDelete( TRUE );
    }
    d->rrs->append( rr );
    rr->domain = d;
}


QList<QDnsRR> * QDnsDomain::cached( const QDns * r )
{
    QDnsManager * m = QDnsManager::manager();
    QStringList n = r->qualifiedNames();
    QValueListIterator<QString> it = n.begin();
    QValueListIterator<QString> end = n.end();
    QList<QDnsRR> * l = new QList<QDnsRR>;
    bool nxdomain;
    int cnamecount = 0;
    while( it != end ) {
	QString s = *it;
	it++;
	nxdomain = FALSE;
#if defined(DEBUG_QDNS)
	qDebug( "looking at cache for %s (%s %d)",
		s.ascii(), r->label().ascii(), r->recordType() );
#endif
	QDnsDomain * d = m->domain( s );
	// d->sweep(); // ### sweep?
	if ( d->rrs )
	    d->rrs->first();
	QDnsRR * rr;
	bool answer = FALSE;
	while( d->rrs && (rr=d->rrs->current()) != 0 ) {
	    if ( rr->t == QDns::Cname && r->recordType() != QDns::Cname &&
		 !rr->nxdomain && cnamecount < 16 ) {
		// cname.  if the code is ugly, that may just
		// possibly be because the concept is.
#if defined(DEBUG_QDNS)
		qDebug( "found cname from %s to %s",
			r->label().ascii(), rr->target.ascii() );
#endif
		s = rr->target;
		d = m->domain( s );
		if ( d->rrs )
		    d->rrs->first();
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
		}
		d->rrs->next();
	    }
	}
	// if we found a positive result, return quickly
	if ( answer && l->count() ) {
#if defined(DEBUG_QDNS)
	    qDebug( "found %d records for %s",
		    l->count(), r->label().ascii() );
#endif
	    l->first();
	    return l;
	}

#if defined(DEBUG_QDNS)
	if ( nxdomain )
	    qDebug( "found NXDomain %s", s.ascii() );
#endif

	if ( !nxdomain ) {
	    // if we didn't, and not a negative result either, perhaps
	    // we need to transmit a query.
	    uint q = 0;
	    while ( q < m->queries.size() &&
		    ( m->queries[q] == 0 ||
		      m->queries[q]->t != r->recordType() ||
		      m->queries[q]->l != s ) )
		q++;
	    // we haven't done it before, so maybe we should.  but
	    // wait - if it's an unqualified name, only ask when all
	    // the other alternatives are exhausted.
	    if ( q == m->queries.size() && ( s.find( '.' ) >= 0 ||
					     l->count() >= n.count()-1 ) ) {
		QDnsQuery * query = new QDnsQuery;
		query->id = ++::id;
		query->t = r->recordType();
		query->l = s;
		query->started = now();
		query->dns->replace( (void*)r, (void*)r );
		QObject::connect( query, SIGNAL(timeout()),
				  QDnsManager::manager(), SLOT(retransmit()) );
		QDnsManager::manager()->transmitQuery( query );
	    } else if ( q < m->queries.size() ) {
		// if we've found an earlier query for the same
		// domain/type, subscribe to its answer
		m->queries[q]->dns->replace( (void*)r, (void*)r );
	    }
	}
    }
    l->first();
    return l;
}


Q_UINT32 lastSweep;

void QDnsDomain::sweep()
{
    if ( !rrs )
	return;

    QDnsRR * rr;
    rrs->first();
    while( (rr=rrs->current()) != 0 ) {
	if ( !rr->deleteTime )
	    rr->deleteTime = lastSweep; // will hit next time around

	if ( rr->current == FALSE ||
		  rr->t == QDns::None ||
		  rr->deleteTime < lastSweep ||
		  rr->expireTime < lastSweep )
	    rrs->remove();
	else
	    rrs->next();
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


/*! \class QDns qdns.h

  \brief The QDns class provides asynchronous DNS lookups.

  \extension Network

  Both Windows and UNIX provides synchronous DNS lookups; Windows
  provides some asynchronous support too.  Neither OS provides
  asynchronous support for anything other than hostname-to-address
  mapping.

  QDns rectifies that, by providing asynchronous caching lookups for
  the record types that we expect modern GUI applications to need in
  the near future.

  The class is a bit hard to use (although much simpler than the
  native APIs); QSocket provides much simpler TCP connection
  facilities.  The aim of QDns is to provide a correct and small API
  to the DNS: Nothing more.  (Correctness implies that the DNS
  information is correctly cached, and correctly timed out.)

  The API is made up of a constructor, functions to set the DNS node
  (the domain in DNS terminology) and record type: setLabel() and
  setRecordType(), the corresponding getters, an isWorking() function
  to determine whether QDns is working or reading, a resultsReady()
  signal, and finally query functions for the result.

  There is one query function for each RecordType, namely addresses(),
  mailServers(), servers() and texts().  There are also two generic
  query functions: canonicalName() return the name you'll presumably
  end up using (the exact meaning of that depends on the record type)
  and qualifiedNames() returns a list of the fully qualified names
  label() maps to.
*/

/*!  Constructs a DNS query object with invalid settings both for the
  label and the search type.
*/

QDns::QDns()
{
    d = new QDnsPrivate;
    t = None;
}




/*!
  Constructs a DNS query object that will return \a rr
  information about \a label.

  The DNS lookup is started the next time the application goes into the event
  loop. When the result is found the signal resultsReady() is emmitted.

  \a rr defaults to \c A, IPv4 addresses.
*/

QDns::QDns( const QString & label, QDns::RecordType rr )
{
    d = new QDnsPrivate;
    t = rr;
    setLabel( label );
    setStartQueryTimer(); // start query the next time we enter event loop
}




/*! Destroys the query object and frees its allocated resources. */

QDns::~QDns()
{
    uint q = 0;
    QDnsQuery * query;
    QDnsManager * m = QDnsManager::manager();
    while( q < m->queries.size() && (query=m->queries[q]) != 0 ) {
	if ( query->dns )
	    (void)query->dns->take( (void*) this );
	q++;
    }

    delete d;
    d = 0;
}




/*!  Sets this query object to query for information about \a label.
  This does not change the recordType(), but its isWorking() most
  likely changes as a result.

  The DNS lookup is started the next time the application goes into the event
  loop. When the result is found the signal resultsReady() is emmitted.
*/

void QDns::setLabel( const QString & label )
{
    l = label;
    n.clear();

    if ( l.length() > 1 && l[(int)l.length()-1] == '.' ) {
	n.append( l.left( l.length()-1 ).lower() );
    } else {
	int i = l.length();
	int dots = 0;
	const int maxDots = 2;
	while( i && dots < maxDots ) {
	    if ( l[--i] == '.' )
		dots++;
	}
	if ( dots < maxDots ) {
	    (void)QDnsManager::manager();
	    QStrListIterator it( *domains );
	    const char * dom;
	    while( (dom=it.current()) != 0 ) {
		++it;
		n.append( l.lower() + "." + dom );
	    }
	}
	n.append( l.lower() );
    }
    setStartQueryTimer(); // start query the next time we enter event loop
#if defined(DEBUG_QDNS)
    qDebug( "QDns::setLabel: %d address(es) for %s", n.count(), l.ascii() );
    int i = 0;
    for( i = 0; i < (int)n.count(); i++ )
	qDebug( "QDns::setLabel: %d: %s", i, n[i].ascii() );
#endif
}

/*!
  \fn QStringList QDns::qualifiedNames() const

  Returns a list of the fully qualified names label() maps to.
*/


/*! \fn QString QDns::label() const

  Returns the domain name for which this object returns information.

  \sa setLabel()
*/

/*! \enum QDns::RecordType

  This enum type defines the record types QDns can handle.  The DNS
  provides many more; these are the ones we've judged to be in current
  use, useful for GUI programs and important enough to support right
  away: <ul>

  <li> \c None - no information.  This exists only so that QDns can
  have a default.

  <li> \c A - IPv4 addresses.  By far the most common type.

  <li> \c Aaaa - Ipv6 addresses.  So far mostly unused.

  <li> \c Mx - Mail eXchanger names.  Used for mail delivery.

  <li> \c Srv - SeRVer names.  Generic record type for finding
  servers.  So far mostly unused.

  <li> \c Cname - canonical name.  Maps from nicknames to the true
  name (the canonical name) for a host.

  <li> \c Ptr - name PoinTeR.  Maps from IPv4 or IPv6 addresses to hostnames.

  <li> \c Txt - arbitrary text for domains.

  </ul>

  We expect that some support for the
  <a href="http://www.dns.net/dnsrd/rfc/rfc2535.html">RFC-2535</a>
  extensions will be added in future versions.
*/

/*!
  Sets this object to query for \a rr records.

  The DNS lookup is started the next time the application goes into the event
  loop. When the result is found the signal resultsReady() is emmitted.

  \sa RecordType
*/

void QDns::setRecordType( RecordType rr )
{
    t = rr;
    setStartQueryTimer(); // start query the next time we enter event loop
}

/*!
  Private slot for starting the query.
*/
void QDns::startQuery()
{
//    QList<QDnsRR> *cached = QDnsDomain::cached( this );
//    delete cached;
    if ( !isWorking() ) {
	emit resultsReady();
    }
    d->startQueryTimer = FALSE;
}

/*!
  The three functions QDns::QDns( QString, RecordType ), QDns::setLabel()
  and QDns::setRecordType() may start a DNS lookup. This function handles
  setting up the single shot timer.
*/
void QDns::setStartQueryTimer()
{
    if ( !d->startQueryTimer ) {
	// start the query the next time we enter event loop
	QTimer::singleShot( 0, this, SLOT(startQuery()) );
	d->startQueryTimer = TRUE;
    }
}

/*! \fn QDns::RecordType QDns::recordType() const

  Returns the record type of this query object.

  \sa setRecordType() RecordType
*/

/*!
  \fn void QDns::resultsReady()

  This signal is emitted when results are available for one of
  the qualifiedNames().
*/

/*! Returns TRUE if QDns is doing a lookup for this object, and FALSE
if this object has the information it wants.

QDns emits the resultsReady() signal when the status changes to FALSE.
*/

bool QDns::isWorking() const
{
#if defined(DEBUG_QDNS)
    qDebug( "QDns::isWorking (%s, %d)", l.ascii(), t );
#endif
    if ( t == None )
	return FALSE;

    QList<QDnsRR> * l = QDnsDomain::cached( this );
    int queries = n.count();
    while( l->current() != 0 ) {
	if ( l->current()->nxdomain )
	    queries--;
	else
	    return FALSE;
	l->next();
    }

    if ( queries <= 0 )
	return FALSE;
    return TRUE;
}


/*!  Returns a list of the addresses for this name if this QDns object
  has a recordType() of \a QDns::A or \a QDns::Aaaa and the answer is
  available, or an empty list else.

  As a special case, if label() is a valid numeric IP address, this function
  returns that address.
*/

QValueList<QHostAddress> QDns::addresses() const
{
#if defined(DEBUG_QDNS)
    qDebug( "QDns::addresses (%s)", l.ascii() );
#endif
    QValueList<QHostAddress> result;
    if ( t != A && t != Aaaa )
	return result;

    if ( t == A ) {
	if ( l.lower() == "localhost" ) {
	    // undocumented hack:
	    result.append( QHostAddress( 0x7f000001 ) );
	    return result;
	}

	int maybeIP4 = 0;
	int bytes = 0;
	QString left = l.simplifyWhiteSpace();
	while( left.length() && bytes < 4 ) {
	    QString byteString;
	    int i = bytes < 3 ? left.find( '.' ) : left.length();
	    if ( i < 0 ) {
		left = "";
	    } else {
		QString byteString = left.left( i ).simplifyWhiteSpace();
		left = left.mid( i+1 );
		bool ok = FALSE;
		uint byteValue = byteString.toUInt( &ok );
		if ( ok && byteValue < 256 ) {
		    maybeIP4 = maybeIP4 * 256 + byteValue;
		    bytes++;
		    if ( bytes == 4 && !left.length() ) {
			result.append( QHostAddress( maybeIP4 ) );
			return result;
		    }
		} else {
		    left = "";
		}
	    }
	}
    }

    QList<QDnsRR> * cached = QDnsDomain::cached( this );

    QDnsRR * rr;
    while( (rr=cached->current()) != 0 ) {
	if ( rr->current && !rr->nxdomain )
	    result.append( rr->address );
	cached->next();
    }
    delete cached;
    return result;
}


// ### the \fn in the docu is not nice but qdoc wants it...
/*!
  \fn QValueList<MailServer> QDns::mailServers() const

  Returns a list of mail servers if the record type is \c Mx. The struct
  \c QDns::MailServer contains the following variables:
  <ul>
  <li> \c QString QDns::MailServer::name
  <li> \c Q_UINT16 QDns::MailServer::priority
  </ul>
*/
QValueList<QDns::MailServer> QDns::mailServers() const
{
#if defined(DEBUG_QDNS)
    qDebug( "QDns::mailServers (%s)", l.ascii() );
#endif
    QValueList<QDns::MailServer> result;
    if ( t != Mx )
	return result;

    QList<QDnsRR> * cached = QDnsDomain::cached( this );

    QDnsRR * rr;
    while( (rr=cached->current()) != 0 ) {
	if ( rr->current && !rr->nxdomain ) {
	    MailServer ms( rr->target, rr->priority );
	    result.append( ms );
	}
	cached->next();
    }
    delete cached;
    return result;
}


// ### the \fn in the docu is not nice but qdoc wants it...
/*!
  \fn QValueList<Server> QDns::servers() const

  Returns a list of servers if the record type is \c Srv. The struct \c
  QDns::Server contains the following variables:
  <ul>
  <li> \c QString QDns::Server::name
  <li> \c Q_UINT16 QDns::Server::priority
  <li> \c Q_UINT16 QDns::Server::weight
  <li> \c Q_UINT16 QDns::Server::port
  </ul>
*/
QValueList<QDns::Server> QDns::servers() const
{
#if defined(DEBUG_QDNS)
    qDebug( "QDns::servers (%s)", l.ascii() );
#endif
    QValueList<QDns::Server> result;
    if ( t != Srv )
	return result;

    QList<QDnsRR> * cached = QDnsDomain::cached( this );

    QDnsRR * rr;
    while( (rr=cached->current()) != 0 ) {
	if ( rr->current && !rr->nxdomain ) {
	    Server s( rr->target, rr->priority, rr->weight, rr->port );
	    result.append( s );
	}
	cached->next();
    }
    delete cached;
    return result;
}


/*!
  Returns a list of texts if the record type is \c Txt.
*/
QStringList QDns::texts() const
{
#if defined(DEBUG_QDNS)
    qDebug( "QDns::texts (%s)", l.ascii() );
#endif
    QStringList result;
    if ( t != Txt )
	return result;

    QList<QDnsRR> * cached = QDnsDomain::cached( this );

    QDnsRR * rr;
    while( (rr=cached->current()) != 0 ) {
	if ( rr->current && !rr->nxdomain ) {
	    QString t( rr->text );
	    result.append( t );
	}
	cached->next();
    }
    delete cached;
    return result;
}


/*!  Returns the canonical name for this DNS node.  (This works
regardless of what recordType() is set to.)

If the canonical name isn't known, this function returns a null
string.

The canonical name of a DNS node is its full name, or the full name of
the target of its CNAME.  For example, if l.trolltech.com is a CNAME to
lupinella.troll.no, and the search path for QDns is "trolltech.com", then
the canonical name for all of "lupinella", "l", "lupinella.troll.no."
and "l.trolltech.com" is "lupinella.troll.no.".  */

QString QDns::canonicalName() const
{
    QList<QDnsRR> * cached = QDnsDomain::cached( this );

    QDnsRR * rr;
    while( (rr=cached->current()) != 0 ) {
	if ( rr->current && !rr->nxdomain && rr->domain ) {
	    delete cached;
	    return rr->target;
	}
	cached->next();
    }
    delete cached;
    return QString::null;
}


#if defined(_OS_UNIX_)

// include this stuff late, so any defines won't hurt.  funkily,
// struct __res_state is part of the api.  normally not used, it says.
// but we use it, to get various information.

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

// if various defines aren't there, we'll set them safely.

#if !defined(MAXNS)
#define MAXNS 1
#endif

static void doResInit()
{
    if ( ns )
	return;
    ns = new QList<QHostAddress>;
    ns->setAutoDelete( TRUE );
    domains = new QStrList( TRUE );
    domains->setAutoDelete( TRUE );

    res_init();
    int i;
    // find the name servers to use
    for( i=0; i < MAXNS && i < _res.nscount; i++ ) {
	ns->append( new QHostAddress(
		             ntohl( _res.nsaddr_list[i].sin_addr.s_addr ) ) );
    }
#if defined(MAXDFLSRCH)
    for( i=0; i < MAXDFLSRCH; i++ )
	if ( _res.dnsrch[i] && *(_res.dnsrch[i]) )
	    domains->append( QString::fromLatin1( _res.dnsrch[i] ).lower() );
#endif
    if ( *_res.defdname )
	domains->append( QString::fromLatin1( _res.defdname ).lower() );
#if defined(SANE_OPERATING_SYSTEM) // never defined, but should be
    res_close();
#endif
}

#elif defined(_OS_WIN32_)

#include <windows.h>

//
// We need to get information about DNS etc. from the Windows
// registry.  We don't worry about Unicode strings here.
//

static QString getWindowsRegString( HKEY key, const char *subKey )
{
    QString s;
    char  buf[512];
    DWORD bsz = sizeof(buf);
    int r = RegQueryValueExA( key, subKey, 0, 0, (LPBYTE)buf, &bsz );
    if ( r == ERROR_SUCCESS ) {
	s = buf;
    } else if ( r == ERROR_MORE_DATA ) {
	char *ptr = new char[bsz+1];
	r = RegQueryValueExA( key, subKey, 0, 0, (LPBYTE)ptr, &bsz );
	if ( r == ERROR_SUCCESS )
	    s = ptr;
	delete [] ptr;
    }
    return s;
}


static void doResInit()
{
    if ( ns )
	return;
    ns = new QList<QHostAddress>;
    ns->setAutoDelete( TRUE );
    domains = new QStrList( TRUE );
    domains->setAutoDelete( TRUE );

    QString domainName, nameServer, searchList;
    HKEY k;
    int r = RegOpenKeyExA( HKEY_LOCAL_MACHINE,
			   "System\\CurrentControlSet\\Services\\Tcpip\\"
			   "Parameters",
			   0, KEY_READ, &k );
    if ( r == ERROR_SUCCESS ) {
	domainName = getWindowsRegString( k, "Domain" );
	nameServer = getWindowsRegString( k, "NameServer" );
	searchList = getWindowsRegString( k, "SearchList" );
    } else {
	// Could not access the TCP/IP parameters
	nameServer = "127.0.0.1";
    }
    RegCloseKey( k );

    nameServer = nameServer.simplifyWhiteSpace();
    int first, last;
    first = 0;
    do {
	last = nameServer.find( ' ', first );
	if ( last < 0 )
	    last = nameServer.length();
	QDns tmp( nameServer.mid( first, last-first ), QDns::A );
	QValueList<QHostAddress> address = tmp.addresses();
	int i = address.count();
	while( i )
	    ns->append( new QHostAddress(address[--i]) );
	first = last+1;
    } while( first < (int)nameServer.length() );

    searchList = searchList + " " + domainName;
    searchList = searchList.simplifyWhiteSpace().lower();
    first = 0;
    do {
	last = searchList.find( ' ', first );
	if ( last < 0 )
	    last = searchList.length();
	domains->append( qstrdup( searchList.mid( first, last-first ) ) );
	first = last+1;
    } while( first < (int)searchList.length() );
}

#endif

#endif
