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

/*****************************************************************************
  QClipboard debug facilities
 *****************************************************************************/
//#define DEBUG_MAPPINGS

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
    if ( owner )
	return;
    owner = new QWidget( 0, "internal clipboard owner" );
    qAddPostRoutine( cleanup );
}

class QClipboardWatcher : public QMimeSource {
public:
    QClipboardWatcher();
    const char* format( int n ) const;
    QByteArray encodedData( const char* fmt ) const;
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
    if ( internalCbData == 0 ) {
	internalCbData = new QClipboardData;
	Q_CHECK_PTR( internalCbData );
	qAddPostRoutine( cleanupClipboardData );
    }
    return internalCbData;
}

QClipboardWatcher::QClipboardWatcher()
{
    setupOwner();
}


#ifdef DEBUG_MAPPINGS
#  define MAP_FLAVOUR(x) x, #x
#else
#  define MAP_FLAVOUR(x) x
#endif
static struct {
    ScrapFlavorType mac_type;
#ifdef DEBUG_MAPPINGS
    const char *mac_type_name;
#endif
    const char *qt_type;
} scrap_map[] = {
    { MAP_FLAVOUR(kScrapFlavorTypeUnicode), "text/plain;charset=ISO-10646-UCS-2" }, //highest priority
    { MAP_FLAVOUR(kScrapFlavorTypeText), "text/plain" },
    { MAP_FLAVOUR(0), NULL }
};

const char* QClipboardWatcher::format( int n ) const
{
    int subtract = 0, i = 0;
    const char *ret = NULL;
    char *buffer = NULL;
    ScrapFlavorInfo *infos = NULL;
    Size flavorsize=0, typesize=0, realsize=sizeof(typesize);
    hasScrapChanged();

    UInt32 cnt = 0;
    if(GetScrapFlavorCount(scrap, &cnt) || !cnt)
	return 0;

    infos = (ScrapFlavorInfo *)calloc(cnt, sizeof(ScrapFlavorInfo));
    if(!infos || GetScrapFlavorInfoList(scrap, &cnt, infos) != noErr) {
	qDebug("Qt: internal: Failure to collect ScrapFlavorInfoList..");
	goto format_end;
    }

    for(int sm = 0; scrap_map[sm].qt_type; sm++) {
	if(n == (sm - subtract)) {
	    if(GetScrapFlavorSize(scrap, scrap_map[sm].mac_type, &flavorsize) == noErr) {
#ifdef DEBUG_MAPPINGS
		qDebug("QClipboard::format(%d): %s %s", n, scrap_map[sm].mac_type_name,
		       scrap_map[sm].qt_type);
#endif
		ret = scrap_map[sm].qt_type;
		goto format_end;
	    } else {
		subtract++;
	    }
	}
    }

    for(i = n; i < (int)cnt; i++) {
	if( ( infos[i].flavorType >> 16 ) == ( 'QTxx' >> 16 ) )
	    break;
	qDebug( "Qt: internal: %s:%d Unknown type %c%c%c%c (%d)", __FILE__, __LINE__,
		char(infos[i].flavorType >> 24), char((infos[i].flavorType >> 16) & 255),
		char((infos[i].flavorType >> 8) & 255), char(infos[i].flavorType & 255 ), i );
    }
    if(i >= (int)cnt)
	return 0;

    if(GetScrapFlavorSize(scrap, infos[i].flavorType, &flavorsize) != noErr || flavorsize < 4) {
	qDebug("Qt: internal: Failure to get ScrapFlavorSize for %d", (int)infos[i].flavorType);
	goto format_end;
    }
    GetScrapFlavorData(scrap, infos[i].flavorType, &realsize, &typesize);
    buffer = (char *)malloc(typesize+realsize);
    GetScrapFlavorData(scrap, infos[i].flavorType, &typesize, buffer);
    memcpy(buffer, buffer+realsize, typesize);
    *(buffer + realsize) = '\0';
    ret = buffer;
#ifdef DEBUG_MAPPINGS
    qDebug("QClipboard::format(%d): (internal) %s", n, ret);
#endif

 format_end:
    if(infos)
	free(infos);
    if(buffer && ret != buffer)
	free(buffer);
    return ret;
}

QByteArray QClipboardWatcher::encodedData( const char* fmt ) const
{
    QByteArray ret;
    int buffersize = 0;
    char *buffer = NULL;
    ScrapFlavorInfo *infos = NULL;
    Size flavorsize=0;
    hasScrapChanged();

    UInt32 cnt = 0;
    if(GetScrapFlavorCount(scrap, &cnt) != noErr || !cnt)
	return 0;

    //special case again..
    for(int sm = 0; scrap_map[sm].qt_type; sm++) {
	if(!qstrcmp(fmt, scrap_map[sm].qt_type)) {
#ifdef DEBUG_MAPPINGS
	    qDebug("QClipboard::encodedData(%s): %s %s", fmt, scrap_map[sm].mac_type_name,
		   scrap_map[sm].qt_type);
#endif
	    GetScrapFlavorSize(scrap, scrap_map[sm].mac_type, &flavorsize);
	    buffer = (char *)malloc(buffersize = flavorsize);
	    GetScrapFlavorData(scrap, scrap_map[sm].mac_type, &flavorsize, buffer);
	    ret.duplicate(buffer, flavorsize);
	    free(buffer);
	    return ret;
	}
    }

    infos = (ScrapFlavorInfo *)calloc(cnt, sizeof(ScrapFlavorInfo));
    if(!infos || GetScrapFlavorInfoList(scrap, &cnt, infos) != noErr) {
	qDebug("Qt: internal: Failure to collect ScrapFlavorInfoList..");
	goto encode_end;
    }

    for(UInt32 x = 0; x < cnt; x++) {
	if( ( infos[x].flavorType >> 16 ) != ( 'QTxx' >> 16 ) ) {
	    qDebug( "Qt: internal: %s:%d Unknown type %c%c%c%c (%d)", __FILE__, __LINE__,
		    char(infos[x].flavorType >> 24), char((infos[x].flavorType >> 16) & 255),
		    char((infos[x].flavorType >> 8) & 255), char(infos[x].flavorType & 255 ), (int) x);
	    continue;
	}

	GetScrapFlavorSize(scrap, infos[x].flavorType, &flavorsize);
	if(buffersize < flavorsize)
	    buffer = (char *)realloc(buffer, buffersize = flavorsize);
	GetScrapFlavorData(scrap, infos[x].flavorType, &flavorsize, buffer);

	UInt32 mimesz;
	memcpy(&mimesz, buffer, sizeof(mimesz));
	if(!qstrnicmp(buffer+sizeof(mimesz), fmt, mimesz)) {
#ifdef DEBUG_MAPPINGS
	    qDebug("QClipboard::encodedData(%s): (internal)", fmt);
#endif
	    int len = flavorsize-(mimesz+sizeof(mimesz));
	    memcpy(buffer, buffer+(mimesz+sizeof(mimesz)), len);
	    ret.duplicate(buffer, len);
	    free(buffer);
	    break;
	}
    }

 encode_end:
    if(infos)
	free(infos);
    if(buffer && ret.data() != buffer)
	free(buffer);
    return ret;
}


/*****************************************************************************
  QClipboard member functions for mac.
 *****************************************************************************/

void QClipboard::clear( Mode mode )
{
    if ( mode != Clipboard ) return;
    ClearCurrentScrap();
}


void QClipboard::ownerDestroyed()
{
    owner = NULL;
    clipboardData()->clear();
    emit dataChanged();
}


static int clipWatcherId = -1;

void QClipboard::connectNotify( const char *signal )
{
    if ( qstrcmp(signal,SIGNAL(dataChanged())) == 0 && clipWatcherId == -1 )
	clipWatcherId = startTimer(100);
}


bool QClipboard::event( QEvent *e )
{
    QTimerEvent *te = 0;
    if (clipWatcherId != -1  && e->type() == QEvent::Timer) {
	te = (QTimerEvent *)e;
	if (te->timerId() != clipWatcherId)
	    te = 0;
	if (te && !receivers(SIGNAL(dataChanged()))) {
	    killTimer(clipWatcherId);
	    clipWatcherId = -1;
	    te = 0;
	}
    }
    if ( e->type() != QEvent::Clipboard && !te)
	return QObject::event( e );

    if(hasScrapChanged()) {
	clipboardData()->clear();
	emit dataChanged();
    }
    return TRUE;
}


QMimeSource* QClipboard::data( Mode mode ) const
{
    if ( mode != Clipboard ) return 0;

    QClipboardData *d = clipboardData();
    if ( !d->source() )
	d->setSource(new QClipboardWatcher());
    return d->source();
}

void QClipboard::setData( QMimeSource *src, Mode mode )
{
    if ( mode != Clipboard ) return;

    QByteArray ar;
    QClipboardData *d = clipboardData();
    d->setSource( src );
    ClearCurrentScrap();
    hasScrapChanged();

    ScrapFlavorType mactype;
    const char *fmt;
    for(int i = 0; (fmt = src->format(i)); i++) {
	bool found_flavour = FALSE;
	for(int sm = 0; scrap_map[sm].qt_type; sm++) {
	    if(!qstrcmp(fmt, scrap_map[sm].qt_type)) {
#ifdef DEBUG_MAPPINGS
		qDebug("QClipboard::setData(%s): %s", fmt, scrap_map[sm].mac_type_name);
#endif
		ar = src->encodedData(scrap_map[sm].qt_type);
		PutScrapFlavor(scrap, scrap_map[sm].mac_type, 0, ar.size(), ar.data());
		found_flavour = TRUE;
		break;
	    }
	}
	if(!found_flavour) { 	    //encode it..
#ifdef DEBUG_MAPPINGS
	    qDebug("QClipboard::setData(%s): (internal)", fmt);
#endif
	    ar = src->encodedData(fmt);
	    mactype = ('Q' << 24) | ('T' << 16) | (i & 0xFFFF);
	    UInt32 mimelen = strlen(fmt);
	    char *buffer = (char *)malloc(ar.size() + mimelen + sizeof(mimelen));
	    memcpy(buffer, &mimelen, sizeof(mimelen));
	    memcpy(buffer+sizeof(mimelen), fmt, mimelen);
	    memcpy(buffer+sizeof(mimelen)+mimelen, ar.data(), ar.size());
	    PutScrapFlavor(scrap, (ScrapFlavorType)mactype, 0,
			   ar.size()+mimelen+sizeof(mimelen), buffer);
	}
    }
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
