/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont_qws.cpp#1 $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for FB
**
** Created : 991026
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qwidget.h"
#include "qpainter.h"
#include "qfontdata_p.h"
#include "qcomplextext_p.h"
#include "qfontdatabase.h"
#include "qstrlist.h"
#include "qcache.h"
#include "qdict.h"
#include "qtextcodec.h"
#include "qapplication.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qmap.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <qshared.h>
#include "qfontmanager_qws.h"
#include "qmemorymanager_qws.h"


/*****************************************************************************
  QFontInternal contains FB font data

  Two global dictionaries and a cache hold QFontInternal objects, which
  are shared between all QFonts.
 *****************************************************************************/

class QFontStruct : public QShared
{
public:
    QFontStruct( const QFontDef& );
   ~QFontStruct();
    const QFontDef *spec()  const;
    void	    reset();
    bool	    dirty() const;
    QMemoryManager::FontID handle() const { return id; }

    int ascent() const { return memorymanager->fontAscent(id); }
    int descent() const { return memorymanager->fontDescent(id); }
    int minLeftBearing() const { return memorymanager->fontMinLeftBearing(id); }
    int minRightBearing() const { return memorymanager->fontMinRightBearing(id); }
    int leading() const { return memorymanager->fontLeading(id); }
    int maxWidth() const { return memorymanager->fontMaxWidth(id); }

    QFontDef s;
    QMemoryManager::FontID id;
    int cache_cost;
};


extern bool qws_smoothfonts; //in qapplication_qws.cpp

inline QFontStruct::QFontStruct( const QFontDef& d )
{
    s = d;
    id = memorymanager->findFont(d);
}

bool QFontStruct::dirty() const
{
    return FALSE;
}

inline const QFontDef *QFontStruct::spec() const
{
    return &s;
}

inline void QFontStruct::reset()
{
}

inline QFontStruct::~QFontStruct()
{
    reset();
}

QFontPrivate::~QFontPrivate()
{
    if ( fin ) fin->deref();
}

// ###### FIXME: merge with code in qfont.cpp. currently not possible because
// of circular dependencies in the headers

// **********************************************************************
// QFontCache
// **********************************************************************

static const int qtFontCacheMin = 2*1024*1024;
static const int qtFontCacheSize = 61;
static const int qtFontCacheFastTimeout =  30000;
static const int qtFontCacheSlowTimeout = 300000;


QFontCache *QFontPrivate::fontCache = 0;


QFontCache::QFontCache() :
    QObject(0, "global font cache"),
    QCache<QFontStruct>(qtFontCacheMin, qtFontCacheSize),
    timer_id(0), fast(FALSE)
{
    setAutoDelete(TRUE);
}


QFontCache::~QFontCache()
{
    // remove negative cache items
    QFontCacheIterator it(*this);
    QString key;
    QFontStruct *qfs;

    while ((qfs = it.current())) {
	key = it.currentKey();
	++it;

	if (qfs == (QFontStruct *) -1) {
	    take(key);
	}
    }
}


bool QFontCache::insert(const QString &key, const QFontStruct *qfs, int cost)
{

#ifdef QFONTCACHE_DEBUG
    qDebug("QFC::insert: inserting %p w/ cost %d", qfs, cost);
#endif // QFONTCACHE_DEBUG

    if (totalCost() + cost > maxCost()) {

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::insert: adjusting max cost to %d (%d %d)",
	       totalCost() + cost, totalCost(), maxCost());
#endif // QFONTCACHE_DEBUG

	setMaxCost(totalCost() + cost);
    }

    bool ret = QCache<QFontStruct>::insert(key, qfs, cost);

    if (ret && (! timer_id || ! fast)) {
	if (timer_id) {

#ifdef QFONTCACHE_DEBUG
	    qDebug("QFC::insert: killing old timer");
#endif // QFONTCACHE_DEBUG

	    killTimer(timer_id);
	}

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::insert: starting timer");
#endif // QFONTCACHE_DEBUG

	timer_id = startTimer(qtFontCacheFastTimeout);
	fast = TRUE;
    }

    return ret;
}


void QFontCache::deleteItem(Item d)
{
    QFontStruct *qfs = (QFontStruct *) d;

    // don't try to delete negative cache items
    if (qfs == (QFontStruct *) -1) {
	return;
    }

    if (qfs->count == 0) {

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::deleteItem: removing %s from cache", (const char *) qfs->name);
#endif // QFONTCACHE_DEBUG

    	delete qfs;
    }
}


void QFontCache::timerEvent(QTimerEvent *)
{
    if (maxCost() <= qtFontCacheMin) {

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::timerEvent: cache max cost is less than min, killing timer");
#endif // QFONTCACHE_DEBUG

	setMaxCost(qtFontCacheMin);

	killTimer(timer_id);
	timer_id = 0;
	fast = TRUE;

	return;
    }

    QFontCacheIterator it(*this);
    QString key;
    QFontStruct *qfs;
    int tqcost = maxCost() * 3 / 4;
    int nmcost = 0;

    while ((qfs = it.current())) {
	key = it.currentKey();
	++it;

	if (qfs != (QFontStruct *) -1) {
	    if (qfs->count > 0) {
		nmcost += qfs->cache_cost;
	    }
	} else {
	    // keep negative cache items in the cache
	    nmcost++;
	}
    }

    nmcost = QMAX(tqcost, nmcost);
    if (nmcost < qtFontCacheMin) nmcost = qtFontCacheMin;

    if (nmcost == totalCost()) {
	if (fast) {

#ifdef QFONTCACHE_DEBUG
	    qDebug("QFC::timerEvent: slowing timer");
#endif // QFONTCACHE_DEBUG

	    killTimer(timer_id);

	    timer_id = startTimer(qtFontCacheSlowTimeout);
	    fast = FALSE;
	}
    } else if (! fast) {
	// cache size is changing now, but we're still on the slow timer... time to
	// drop into passing gear

#ifdef QFONTCACHE_DEBUG
	qDebug("QFC::timerEvent: dropping into passing gear");
#endif // QFONTCACHE_DEBUG

	killTimer(timer_id);
	timer_id = startTimer(qtFontCacheFastTimeout);
	fast = TRUE;
    }

#ifdef QFONTCACHE_DEBUG
    qDebug("QFC::timerEvent: before cache cost adjustment: %d %d",
	   totalCost(), maxCost());
#endif // QFONTCACHE_DEBUG

    setMaxCost(nmcost);

#ifdef QFONTCACHE_DEBUG
    qDebug("QFC::timerEvent:  after cache cost adjustment: %d %d",
	   totalCost(), maxCost());
#endif // QFONTCACHE_DEBUG

}


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

/*****************************************************************************
  set_local_font() - tries to set a sensible default font char set
 *****************************************************************************/

void QFont::initialize()
{
    if ( QFontPrivate::fontCache ) return;
    QFontPrivate::fontCache = new QFontCache();
}

void QFont::cleanup()
{
    delete QFontPrivate::fontCache;
    QFontPrivate::fontCache = 0;
}


void QFont::cacheStatistics()
{
}

// If d->req.dirty is not TRUE the font must have been loaded
// and we can safely assume that d->fin is a valid pointer:

#define DIRTY_FONT (d->request.dirty || d->fin->dirty())


Qt::HANDLE QFont::handle() const
{
    d->load(); // the REAL reason this is called
    return d->fin->handle();
}

QString QFont::rawName() const
{
    if ( DIRTY_FONT )
	d->load();
    return "unknown";
}

void QFont::setRawName( const QString & )
{
}


bool QFont::dirty() const
{
    return DIRTY_FONT;
}


QString QFontPrivate::defaultFamily() const
{
    switch( request.styleHint ) {
	case QFont::Times:
	    return QString::fromLatin1("times");
	case QFont::Courier:
	    return QString::fromLatin1("courier");
	case QFont::Decorative:
	    return QString::fromLatin1("old english");
	case QFont::Helvetica:
	case QFont::System:
	default:
	    return QString::fromLatin1("helvetica");
    }
}

QString QFontPrivate::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}

QString QFontPrivate::lastResortFont() const
{
    qFatal( "QFont::lastResortFont: Cannot find any reasonable font" );
    // Shut compiler up
    return "Times";
}

void QFontPrivate::load()
{
    QString k = key();
    QFontStruct* qfs = fontCache->find(k);
    if ( !qfs ) {
	qfs = new QFontStruct(request);
	fontCache->insert( k, qfs, 1 );
    }
    qfs->ref();
    if ( fin )
	fin->deref();
    fin = qfs;
    request.dirty = FALSE;
}

QRect QFontPrivate::boundingRect( const QChar &ch )
{
    return QRect(0, 0, 1204, 768); //take that
}

int QFontPrivate::textWidth( const QString &str, int pos, int len )
{
    return 1000;
}

/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/


QFontStruct *QFontMetrics::internal()
{
    if (painter) {
        painter->cfont.d->load();
        return painter->cfont.d->fin;
    } else {
        return d->fin;
    }
}

#if 0
const QFontDef *QFontMetrics::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return d->fin->spec();
    }
}
#endif

// How to calculate metrics from ink and logical rectangles.
#define LBEARING(i,l) (i.x+l.x)
#define RBEARING(i,l) (i.width-l.width)
#define ASCENT(i,l) (-i.y)
#define DESCENT(i,l) (i.height+i.y-1)


int QFontMetrics::ascent() const
{
    int ret=((QFontMetrics*)this)->internal()->ascent();
    return ret;
}

int QFontMetrics::descent() const
{
    int ret=((QFontMetrics*)this)->internal()->descent();
    return ret;
}

bool QFontMetrics::inFont(QChar ch) const
{
    return memorymanager->inFont(((QFontMetrics*)this)->internal()->handle(),ch);
}

int QFontMetrics::leftBearing(QChar ch) const
{
    return memorymanager->lockGlyphMetrics(((QFontMetrics*)this)->internal()->handle(),ch)->bearingx;
}


int QFontMetrics::rightBearing(QChar ch) const
{
    QGlyphMetrics *metrics = memorymanager->lockGlyphMetrics(((QFontMetrics*)this)->internal()->handle(),ch);
    return metrics->advance - metrics->width - metrics->bearingx;
}

int QFontMetrics::minLeftBearing() const
{
    return ((QFontMetrics*)this)->internal()->minLeftBearing();
}

int QFontMetrics::minRightBearing() const
{
    return ((QFontMetrics*)this)->internal()->minRightBearing();
}

int QFontMetrics::height() const
{
    return ascent()+descent()+1;
}

int QFontMetrics::leading() const
{
    return 2;
    //return internal()->leading();
}

int QFontMetrics::lineSpacing() const
{
    return leading() + height();
}

int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    QChar ch = QComplexText::shapedCharacter( str, pos );
    return (ch != 0) ? width(ch) : 0; 
}

int QFontMetrics::width( QChar ch ) const
{
    qObsolete( "QFontMetrics", "width" );
    return memorymanager->lockGlyphMetrics(((QFontMetrics*)this)->internal()->handle(),ch)->advance;
}

int QFontMetrics::width( const QString &str, int len ) const
{
    if ( len < 0 )
	len = str.length();
    int ret=0;
    for (int i=0; i<len; i++)
	ret += width(str[i]);
    return ret;
}

QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    return QRect( 0,-(ascent()),width(str,len),height());
}

int QFontMetrics::maxWidth() const
{
    int ret=((QFontMetrics*)this)->internal()->maxWidth();
    return ret;
}

int QFontMetrics::underlinePos() const
{
    return 1; // XXX
}

int QFontMetrics::strikeOutPos() const
{
    return ascent()/3; // XXX
}

int QFontMetrics::lineWidth() const
{
    return 1; // XXX
}


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

#if 0
const QFontDef *QFontInfo::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return d->fin->spec();
    }
}
#endif

int QFont::pixelSize() const
{
    return d->request.pointSize/10;
}

void QFont::setPixelSizeFloat( float pixelSize )
{
    setPointSizeFloat( pixelSize );
}

/*!
  Saves the glyphs in the font that have previously been accessed
  as a QPF file. If \a all is TRUE (the default), then before saving,
  all glyphs are marked as used.

  If the font is large and you are sure that only a subset of characters
  will ever be required on the target device, passing FALSE for the
  \a all parameter can save significant disk space.

  \note Only applicable on Qt/Embedded.
*/
void QFont::qwsRenderToDisk(bool all)
{
#ifndef QT_NO_QWS_SAVEFONTS
    memorymanager->savePrerenderedFont(handle(), all);
#endif
}
