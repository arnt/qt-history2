/****************************************************************************
** $Id$
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for FB
**
** Created : 991026
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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
#include <private/qunicodetables_p.h>
#include "qfontdatabase.h"
#include "qstrlist.h"
#include "qcache.h"
#include "qdict.h"
#include "qtextcodec.h"
#include "qapplication.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qmap.h"
#include "qshared.h"
#include "qfontmanager_qws.h"
#include "qmemorymanager_qws.h"
#include "qgfx_qws.h"
#include "qtextengine_p.h"

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


/*****************************************************************************
  QFont member functions
 *****************************************************************************/

Qt::HANDLE QFont::handle() const
{
    if ( d->request.dirty )
	d->load(); // the REAL reason this is called
    return d->fin->handle();
}

QString QFont::rawName() const
{
    if ( d->request.dirty )
	d->load();
    return "unknown";
}

void QFont::setRawName( const QString & )
{
}


bool QFont::dirty() const
{
    return d->request.dirty;
}


QFontPrivate::~QFontPrivate()
{
    if ( fin ) fin->deref();
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
    QFontEngine* qfs = fontCache->find(k);
    if ( !qfs ) {
	qfs = new QFontEngine(request);
	// make larger fonts cost a little more
	fontCache->insert(k, qfs, 1+qfs->s.pointSize/80);
    }
    qfs->ref();
    if ( fin )
	fin->deref();
    fin = qfs;
    request.dirty = FALSE;
}


/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

int QFontMetrics::ascent() const
{
    return d->fin->ascent();
}

int QFontMetrics::descent() const
{
    return d->fin->descent();
}

bool QFontMetrics::inFont(QChar ch) const
{
    return memorymanager->inFont(((QFontMetrics*)this)->d->fin->handle(),ch);
}

int QFontMetrics::leftBearing(QChar ch) const
{
    return memorymanager->lockGlyphMetrics(((QFontMetrics*)this)->d->fin->handle(),ch)->bearingx;
}


int QFontMetrics::rightBearing(QChar ch) const
{
    QGlyphMetrics *metrics = memorymanager->lockGlyphMetrics(((QFontMetrics*)this)->d->fin->handle(),ch);
    return metrics->advance - metrics->width - metrics->bearingx;
}

int QFontMetrics::minLeftBearing() const
{
    return d->fin->minLeftBearing();
}

int QFontMetrics::minRightBearing() const
{
    return d->fin->minRightBearing();
}

int QFontMetrics::height() const
{
    return ascent()+descent()+1;
}

int QFontMetrics::leading() const
{
    return d->fin->leading();
}

int QFontMetrics::lineSpacing() const
{
    return leading() + height();
}

int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    QTextEngine layout( str,  d );
    layout.itemize( FALSE );
    int w = layout.width( pos, 1 );
}

int QFontMetrics::width( QChar ch ) const
{
    if ( ch.category() == QChar::Mark_NonSpacing ) return 0;

    return memorymanager->lockGlyphMetrics(((QFontMetrics*)this)->d->fin->handle(),ch)->advance;
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
    return d->fin->maxCharWidth();
}

int QFontMetrics::underlinePos() const
{
    return d->fin->underlinePos();
}

int QFontMetrics::strikeOutPos() const
{
    return ascent()/3; // XXX
}

int QFontMetrics::lineWidth() const
{
    return d->fin->lineWidth();
}


/*!
    Saves the glyphs in the font that have previously been accessed as
    a QPF file. If \a all is TRUE (the default), then before saving,
    all glyphs are marked as used.

    If the font is large and you are sure that only a subset of
    characters will ever be required on the target device, passing
    FALSE for the \a all parameter can save a significant amount of
    disk space.

    Note that this function is only applicable on Qt/Embedded.
*/
void QFont::qwsRenderToDisk(bool all)
{
#ifndef QT_NO_QWS_SAVEFONTS
    memorymanager->savePrerenderedFont(handle(), all);
#endif
}
