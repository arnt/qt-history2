/****************************************************************************
** $Id: $
**
** Implementation of QSound class and QAuServer internal class
**
** Created : 000117
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

#define QT_CLEAN_NAMESPACE

#include "qsound.h"

#ifndef QT_NO_SOUND

#include "qptrdict.h"
#include "qsocketnotifier.h"
#include "qapplication.h"


#ifdef QT_NAS_SUPPORT

#include <audio/audiolib.h>
#include <audio/soundlib.h>

static AuServer *nas=0;

class QAuBucketNAS : public QAuBucket {
public:
    QAuBucketNAS(int i) : id(i) {}

    ~QAuBucketNAS()
    {
	if ( nas )
	    AuDestroyBucket( nas, (AuBucketID)id, NULL );
    }

    AuBucketID id;
};

class QAuServerNAS : public QAuServer {
    Q_OBJECT

    QSocketNotifier* sn;

public:
    QAuServerNAS(QObject* parent);
    ~QAuServerNAS();

    void init(QSound*);
    void play(const QString& filename);
    void play(QSound*);
    void stop(QSound*);
    bool okay();

public slots:
    void dataReceived();

private:
    QAuBucketNAS* bucket(QSound* s)
    {
	return (QAuBucketNAS*)QAuServer::bucket(s);
    }
};

QAuServerNAS::QAuServerNAS(QObject* parent) :
    QAuServer(parent,"Network Audio System")
{
    nas = AuOpenServer(NULL, 0, NULL, 0, NULL, NULL);
    if (nas) {
	// Ask Qt for async messages...
	sn=new QSocketNotifier(AuServerConnectionNumber(nas),
		QSocketNotifier::Read);
	QObject::connect(sn, SIGNAL(activated(int)),
		this, SLOT(dataReceived()));
    } else {
	sn = 0;
    }
}

QAuServerNAS::~QAuServerNAS()
{
    if ( nas )
	AuCloseServer( nas );
    delete sn;
    nas = 0;
}

void QAuServerNAS::play(const QString& filename)
{
    if (nas) {
	int iv=100;
	AuFixedPoint volume=AuFixedPointFromFraction(iv,100);
	AuSoundPlayFromFile(nas, filename, AuNone, volume, NULL, NULL, NULL, NULL, NULL, NULL);
	AuFlush(nas);
	dataReceived();
	AuFlush(nas);
	qApp->flushX();
    }
}

static QPtrDict<void> *inprogress=0;

static void callback( AuServer*, AuEventHandlerRec*, AuEvent* e, AuPointer p)
{
qDebug("Event %d",e->type);
    if ( inprogress->find(p) && e ) {
	if (e->type==AuEventTypeElementNotify &&
		    e->auelementnotify.kind==AuElementNotifyKindState) {
	    if ( e->auelementnotify.cur_state == AuStateStop )
		qDebug("DONE");
	}
    }
}

void QAuServerNAS::play(QSound* s)
{
    if (nas) {
	if ( !inprogress )
	    inprogress = new QPtrDict<void>;
	inprogress->insert(s,(void*)1);
	int iv=100;
	AuFixedPoint volume=AuFixedPointFromFraction(iv,100);
	AuSoundPlayFromBucket(nas, bucket(s)->id, AuNone, volume,
		callback, s, 0, NULL, NULL, NULL, NULL);
	AuFlush(nas);
	dataReceived();
	AuFlush(nas);
	qApp->flushX();
    }
}

void QAuServerNAS::stop(QSound* s)
{
    if (nas) {
	inprogress->remove(s);

	// #######
    }
}

void QAuServerNAS::init(QSound* s)
{
    if ( nas )
	setBucket(s,
	    new QAuBucketNAS(AuSoundCreateBucketFromFile(nas, s->fileName(),
		0 /*AuAccessAllMasks*/, NULL, NULL)));
}

bool QAuServerNAS::okay()
{
    return !!nas;
}

void QAuServerNAS::dataReceived()
{
    AuHandleEvents(nas);
}

#include "qsound_x11.moc"

#endif


class QAuServerNull : public QAuServer {
public:
    QAuServerNull(QObject* parent);

    void play(const QString&) { }
    void play(QSound*) { }
    void stop(QSound*) { }
    bool okay() { return FALSE; }
};

QAuServerNull::QAuServerNull(QObject* parent) :
    QAuServer(parent,"Null Audio Server")
{
}


QAuServer* qt_new_audio_server()
{
#ifdef QT_NAS_SUPPORT
    QAuServer* s=new QAuServerNAS(qApp);
    if (s->okay()) {
	return s;
    } else {
	delete s;
    }
#endif
    return new QAuServerNull(qApp);
}

#endif // QT_NO_SOUND
