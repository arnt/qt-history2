/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qclipboard_mac.cpp
**
** Implementation of QClipboard class for mac
**
** Created : 001019
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
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

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

// #define QCLIPBOARD_DEBUG

#include "qapplication.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qapplication_p.h"
#include "qt_mac.h"

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
    
    void addTransferredPixmap(QPixmap pm)
    { /* TODO: queue them */
	transferred[tindex] = pm;
	tindex=(tindex+1)%2;
    }
    void clearTransfers()
    {
	transferred[0] = QPixmap();
	transferred[1] = QPixmap();
    }

    void clear();

    // private:
    QMimeSource *src;

    QPixmap transferred[2];
    int tindex;
};

QClipboardData::QClipboardData()
{
    src = 0;
    tindex=0;
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

const char* QClipboardWatcher::format( int n ) const
{
    char *ret = NULL, *buffer = NULL;
    ScrapFlavorInfo *infos = NULL;
    Size flavorsize=0, typesize=0, realsize=sizeof(typesize);
    hasScrapChanged();

    UInt32 cnt = 0;
    if(GetScrapFlavorCount(scrap, &cnt) || !cnt) {
	return 0;
    }

    infos = (ScrapFlavorInfo *)calloc(cnt, sizeof(ScrapFlavorInfo));
    if(!infos || GetScrapFlavorInfoList(scrap, &cnt, infos) != noErr) {
	qDebug("Failure to collect ScrapFlavorInfoList..");
	goto format_end;
    }

    if(infos[n].flavorType == kScrapFlavorTypeText) {
	ret = "text/plain";
	goto format_end;
    }

    if(GetScrapFlavorSize(scrap, infos[n].flavorType, &flavorsize) != noErr || flavorsize < 4) {
	qDebug("Failure to get ScrapFlavorSize for %d", (int)infos[n].flavorType);
	goto format_end;
    }
    GetScrapFlavorData(scrap, infos[n].flavorType, &realsize, &typesize);

    buffer = (char *)malloc(typesize+realsize);
    GetScrapFlavorData(scrap, infos[n].flavorType, &typesize, buffer);
    memcpy(buffer, buffer+realsize, typesize);
    *(buffer + realsize) = '\0';
    ret = buffer;

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
    char *buffer = NULL;
    ScrapFlavorInfo *infos = NULL;
    Size flavorsize=0;
    hasScrapChanged();

    UInt32 cnt = 0;
    if(GetScrapFlavorCount(scrap, &cnt) != noErr || !cnt)
	return 0;

    //special case again..
    if(!strcmp(fmt, "text/plain")) {
	GetScrapFlavorSize(scrap, kScrapFlavorTypeText, &flavorsize);
	buffer = (char *)malloc(flavorsize);
	GetScrapFlavorData(scrap, kScrapFlavorTypeText, &flavorsize, buffer);
	ret.assign(buffer, flavorsize);
	return ret;
    }

    infos = (ScrapFlavorInfo *)calloc(cnt, sizeof(ScrapFlavorInfo));
    if(!infos || GetScrapFlavorInfoList(scrap, &cnt, infos) != noErr) {
	qDebug("Failure to collect ScrapFlavorInfoList..");
	goto encode_end;
    }

    for(UInt32 x = 0; x < cnt; x++) {
	GetScrapFlavorSize(scrap, infos[x].flavorType, &flavorsize);
	buffer = (char *)realloc(buffer, flavorsize);
	GetScrapFlavorData(scrap, infos[x].flavorType, &flavorsize, buffer);
	
	UInt32 mimesz;
	memcpy(&mimesz, buffer, sizeof(mimesz));
	if(!strncasecmp(buffer+sizeof(mimesz), fmt, mimesz))
	    ret.assign(buffer+mimesz+sizeof(mimesz), flavorsize-(mimesz+sizeof(mimesz)));
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

void QClipboard::clear()
{
    ClearCurrentScrap();
}


void QClipboard::ownerDestroyed()
{
    owner = NULL;
}


void QClipboard::connectNotify( const char * )
{
}


bool QClipboard::event( QEvent *e )
{
    if ( e->type() != QEvent::Clipboard )
	return QObject::event( e );

    if(hasScrapChanged()) {
	clipboardData()->clear();
	emit dataChanged();
    }
    return TRUE;
}


QMimeSource* QClipboard::data() const
{
    QClipboardData *d = clipboardData();
    if ( !d->source() )
	d->setSource(new QClipboardWatcher());
    return d->source();
}

void QClipboard::setData( QMimeSource *src )
{
    QByteArray ar;
    QClipboardData *d = clipboardData();
    d->setSource( src );

    ClearCurrentScrap();
    hasScrapChanged();

    //handle text/plain specially so other apps can get it
    if ( d->source()->provides("text/plain") ) {
	ar = d->source()->encodedData("text/plain");
	PutScrapFlavor(scrap, kScrapFlavorTypeText, 0, ar.size(), ar.data());
    }
	
    //now the other formats
    ScrapFlavorType mactype;
    const char *fmt;
    for(int i = 0; (fmt = d->source()->format(i)); i++) {
	if(!strcmp(fmt, "text/plain"))
	    continue; //already did that

	//encode it..
	ar = d->source()->encodedData(fmt);
	mactype = ('Q' << 24) | ('T' << 16) | (i & 0xFFFF);
	UInt32 mimelen = strlen(fmt);
	char *buffer = (char *)malloc(ar.size() + mimelen + sizeof(mimelen));
	memcpy(buffer, &mimelen, sizeof(mimelen));
	memcpy(buffer+sizeof(mimelen), fmt, mimelen);
	memcpy(buffer+sizeof(mimelen)+mimelen, ar.data(), ar.size());
	PutScrapFlavor(scrap, (ScrapFlavorType)mactype, 0, ar.size()+mimelen+sizeof(mimelen), buffer);
    }
}

void QClipboard::setSelectionMode(bool)
{
}


bool QClipboard::selectionModeEnabled() const
{
    return FALSE; //nei takk
}

bool QClipboard::supportsSelection() const
{
    return FALSE; //nei takk
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
