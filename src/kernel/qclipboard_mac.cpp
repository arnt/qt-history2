/****************************************************************************
** $Id$
**
** Implementation of QClipboard class for mac
**
** Created : 001019
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD
#include "qapplication.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qapplication_p.h"
#include "qt_mac.h"
#include <stdlib.h>
#include <string.h>

static ScrapRef scrap = NULL;
static QWidget * owner = 0;

bool hasScrapChanged()
{
    ScrapRef nref;
    GetCurrentScrap(&nref);
    if(nref != scrap) {
	scrap = nref;
	return TRUE;
    }
    return FALSE;
}

static void cleanup()
{
    delete owner;
    owner = 0;
}

static
void setupOwner()
{
    if(owner)
	return;
    owner = new QWidget(0, "internal clipboard owner");
    qAddPostRoutine(cleanup);
}

class QClipboardWatcher : public QMimeSource {
public:
    QClipboardWatcher();
    const char* format(int n) const;
    QByteArray encodedData(const char* fmt) const;
};



class QClipboardData
{
public:
    QClipboardData();
    ~QClipboardData();

    void setSource(QMimeSource* s)
    {
	QMimeSource *s2 = src;
	src = s;
	delete s2;
    }

    QMimeSource *source() const { return src; }
    void clear();

    // private:
    QMimeSource *src;
};

QClipboardData::QClipboardData()
{
    src = 0;
}

QClipboardData::~QClipboardData()
{
    clear();
}

void QClipboardData::clear()
{
    QMimeSource *s2 = src;
    src = NULL;
    delete s2;
}


static QClipboardData *internalCbData = 0;
static void cleanupClipboardData()
{
    delete internalCbData;
    internalCbData = 0;
}

static QClipboardData *clipboardData()
{
    if(internalCbData == 0) {
	internalCbData = new QClipboardData;
	Q_CHECK_PTR(internalCbData);
	qAddPostRoutine(cleanupClipboardData);
    }
    return internalCbData;
}

QClipboardWatcher::QClipboardWatcher()
{
    setupOwner();
}

const char* QClipboardWatcher::format(int n) const
{
    const char* mime = NULL;
    if(n >= 0) {
	UInt32 cnt = 0;
	if(GetScrapFlavorCount(scrap, &cnt) || !cnt)
	    return NULL;
	
	ScrapFlavorInfo *infos = (ScrapFlavorInfo *)calloc(cnt, sizeof(ScrapFlavorInfo));
	if(!infos || GetScrapFlavorInfoList(scrap, &cnt, infos) != noErr) {
	    qDebug("Qt: internal: Failure to collect ScrapFlavorInfoList..");
	} else {
	    QMacMime::QMacMimeType qmt = QMacMime::MIME_CLIP;
	    {
		Size sz;
		extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
		if(GetScrapFlavorSize(scrap, qt_mac_mime_type, &sz) == noErr)
		    qmt = QMacMime::MIME_QT_CONVERTOR;
	    }

	    bool sawSBText = FALSE;
	    for(int i = 0; i < (int)cnt; i++) {
		if(infos[i].flavorType == kScrapFlavorTypeText) {
		    sawSBText = TRUE;
		} else if(const char *m = QMacMime::flavorToMime(qmt, infos[i].flavorType)) {
		    if(!n) {
			mime = m;
			break;
		    }
		    n--;
		}
	    }
	    if(!mime && sawSBText && !n) 
		mime = QMacMime::flavorToMime(qmt, kScrapFlavorTypeText);
	    free(infos);
	}
    }
    return n ? NULL : mime;
}

QByteArray QClipboardWatcher::encodedData(const char* mime) const
{
    Size flavorsize=0;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_CLIP;
    extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
    if(GetScrapFlavorSize(scrap, qt_mac_mime_type, &flavorsize) == noErr)
	qmt = QMacMime::MIME_QT_CONVERTOR;
    QPtrList<QMacMime> all = QMacMime::all(qmt);
    for (QMacMime* c = all.first(); c; c = all.next()) {
	int flav = c->flavorFor(mime);
	if(flav) {
	    if(GetScrapFlavorSize(scrap, flav, &flavorsize) == noErr) {
		char *buffer = (char *)malloc(flavorsize);
		GetScrapFlavorData(scrap, flav, &flavorsize, buffer);
		QByteArray r, tr;
		r.setRawData(buffer, flavorsize);
		{
		    QList<QByteArray> lst;
		    lst.append(r);
		    tr = c->convertToMime(lst, mime, flav);
		}
		tr.detach();
		r.resetRawData(buffer, flavorsize);
		free(buffer);
		return tr;
	    }
	}
    }
    return QByteArray();
}


/*****************************************************************************
  QClipboard member functions for mac.
 *****************************************************************************/

void QClipboard::clear(Mode mode)
{
    if(mode != Clipboard) return;
    ClearCurrentScrap();
}


void QClipboard::ownerDestroyed()
{
    owner = NULL;
    clipboardData()->clear();
    emit dataChanged();
}


static int clipWatcherId = -1;

void QClipboard::connectNotify(const char *signal)
{
    if(qstrcmp(signal,SIGNAL(dataChanged())) == 0 && clipWatcherId == -1)
	clipWatcherId = startTimer(100);
}


bool QClipboard::event(QEvent *e)
{
    QTimerEvent *te = 0;
    if(clipWatcherId != -1  && e->type() == QEvent::Timer) {
	te = (QTimerEvent *)e;
	if(te->timerId() != clipWatcherId)
	    te = 0;
	if(te && !receivers(SIGNAL(dataChanged()))) {
	    killTimer(clipWatcherId);
	    clipWatcherId = -1;
	    te = 0;
	}
    }
    if(e->type() != QEvent::Clipboard && !te)
	return QObject::event(e);

    if(hasScrapChanged()) {
	clipboardData()->clear();
	emit dataChanged();
    }
    return TRUE;
}


QMimeSource* QClipboard::data(Mode mode) const
{
    if(mode != Clipboard) 
	return 0;

    QClipboardData *d = clipboardData();
    if(!d->source())
	d->setSource(new QClipboardWatcher());
    return d->source();
}

void QClipboard::setData(QMimeSource *src, Mode mode)
{
    if(mode != Clipboard) 
	return;

    QClipboardData *d = clipboardData();
    d->setSource(src);
    ClearCurrentScrap();
    hasScrapChanged();

    QPtrList<QMacMime> all = QMacMime::all(QMacMime::MIME_CLIP);
    const char* mime;
    for (int i = 0; (mime = src->format(i)); i++) {
	for (QMacMime* c = all.first(); c; c = all.next()) {
	    if(c->flavorFor(mime)) {
		for (int j = 0; j < c->countFlavors(); j++) {
		    uint flav = c->flavor(j);
		    if(c->canConvert(mime, flav)) {
			QList<QByteArray> md = c->convertFromMime(src->encodedData(mime), mime, flav);
			if(md.count() > 1)
			    qWarning("QClipBoard: cannot handle multiple byte array conversions..");
			PutScrapFlavor(scrap, (ScrapFlavorType)flav, 0, md.first().size(), md.first().data());
		    }
		}
	    }
	}
    }

    extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
    PutScrapFlavor(scrap, qt_mac_mime_type, 0, 0, NULL);
    emit dataChanged();
}

bool QClipboard::supportsSelection() const
{
    return FALSE; //nei takk
}

bool QClipboard::ownsSelection() const
{
    return FALSE;
}


bool QClipboard::ownsClipboard() const
{
    qWarning("Qt: QClipboard::ownsClipboard: UNIMPLEMENTED!");
    return FALSE;
}

void QClipboard::loadScrap(bool)
{
    LoadScrap();
}

void QClipboard::saveScrap()
{
    UnloadScrap();
}


#endif // QT_NO_CLIPBOARD
