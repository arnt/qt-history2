/****************************************************************************
** $Id$
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for Win32
**
** Created : 940630
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
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

// the miscrosoft platform SDK says that the Unicode versions of
// TextOut and GetTextExtentsPoint32 are supported on all platforms, so we use them
// exclusively to simplify code, save a lot of conversions into the local encoding
// and have generally better unicode support :)

#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qcomplextext_p.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"

#include "qwidget.h"
#include "qpainter.h"
#include "qdict.h"
#include "qcache.h"
#include <limits.h>
#include "qt_windows.h"
#include "qapplication_p.h"
#include "qpaintdevicemetrics.h"
#include <private/qunicodetables_p.h>
#include <qfontdatabase.h>


extern HDC   shared_dc;		// common dc for all fonts

static int max_font_count = 256;


// ### maybe move to qapplication_win
QFont qt_LOGFONTtoQFont(LOGFONT& lf, bool /*scale*/)
{
    QString family = QT_WA_INLINE( QString::fromUcs2((ushort*)lf.lfFaceName),
				   QString::fromLocal8Bit((char*)lf.lfFaceName) );
    QFont qf(family);
    if (lf.lfItalic)
	qf.setItalic( TRUE );
    if (lf.lfWeight != FW_DONTCARE)
	qf.setWeight(lf.lfWeight*99/900);
    int lfh = QABS( lf.lfHeight );
    Q_ASSERT(shared_dc);
    qf.setPointSizeFloat( lfh * 72.0 / GetDeviceCaps(shared_dc,LOGPIXELSY) );
    return qf;
}


static inline float pixelSize( const QFontDef &request, QPaintDevice *paintdevice,
			       int )
{
    float pSize;
    if ( request.pointSize != -1 ) {
	if ( paintdevice )
	    pSize = request.pointSize *
		    QPaintDeviceMetrics( paintdevice ).logicalDpiY() / 720.;
	else
	    pSize = (request.pointSize*GetDeviceCaps(shared_dc,LOGPIXELSY) + 360) / 720;
    } else {
	pSize = request.pixelSize;
    }
    return pSize;
}

static inline float pointSize( const QFontDef &fd, QPaintDevice *paintdevice,
			       int )
{
    float pSize;
    if ( fd.pointSize == -1 ) {
	if ( paintdevice )
	    pSize = fd.pixelSize * 720. /
		    QPaintDeviceMetrics( paintdevice ).logicalDpiY();
	else
	    pSize = fd.pixelSize * 72.0 / GetDeviceCaps(shared_dc,LOGPIXELSY);
    } else {
	pSize = fd.pointSize;
    }
    return pSize;
}

/*****************************************************************************
  QFont member functions
 *****************************************************************************/

QFont::Script QFontPrivate::defaultScript = QFont::UnknownScript;

void QFont::initialize()
{
    if ( QFontCache::instance )
	return;
    shared_dc = CreateCompatibleDC( qt_display_dc() );
#if defined(QT_CHECK_RANGE)
    if ( !shared_dc )
	qSystemWarning( "QFont::initialize() (qfont_win.cpp, 163): couldn't create device context" );
#endif
    new QFontCache();

    // #########
    QFontPrivate::defaultScript = QFont::Latin;
}

void QFont::cleanup()
{
    delete QFontCache::instance;
    DeleteDC( shared_dc );
    shared_dc = 0;
}


void QFontPrivate::load( QFont::Script script )
{
//     qDebug("trying to load font for script %d", script );
#ifdef QT_CHECK_STATE
    // sanity checks
    Q_ASSERT( QFontCache::instance != 0);
    Q_ASSERT( script >= 0 && script < QFont::LastPrivateScript );
#endif // QT_CHECK_STATE

    int px = int( pixelSize( request, paintdevice, screen ) + .5 );
    QFontDef req = request;
    req.pixelSize = px;
    req.pointSize = 0;
    req.underline = req.strikeOut = 0;
    req.mask = 0;

    if ( ! engineData ) {
	QFontCache::Key key( req, QFont::NoScript, screen );

	// look for the requested font in the engine data cache
	engineData = QFontCache::instance->findEngineData( key );

	if ( ! engineData ) {
	    // create a new one
	    engineData = new QFontEngineData;
	    QFontCache::instance->insertEngineData( key, engineData );
	} else {
	    engineData->ref();
	}
    }

    // load the font
    QFontEngine *engine = 0;
    double scale = 1.0; // ### TODO: fix the scale calculations
    Q_UNUSED( scale );

    // list of families to try
    QStringList family_list = QStringList::split( ',', request.family );

    // append the substitute list for each family in family_list
    QStringList subs_list;
    QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
    for ( ; it != end; ++it )
	subs_list += QFont::substitutes( *it );
    family_list += subs_list;

    // append the default fallback font for the specified script
    // family_list << ... ;

    // null family means find the first font matching the specified script
    family_list << QString::null;

    it = family_list.begin(), end = family_list.end();
    for ( ; ! engine && it != end; ++it ) {
	req.family = *it;

#if defined( Q_WS_X11 ) || defined (Q_WS_WIN )
	QFontCache::Key key( req, script, screen );
#else
	QFontCache::Key key( req, QFont::NoScript, screen );
#endif // Q_WS_X11

	// first, look in the font cache for a font...
	engine = QFontCache::instance->findEngine( key );
	if ( engine ) {
	    if ( engine->type() != QFontEngine::Box ) {
		// found a real font engine... stop
		break;
	    }

	    if ( ! req.family.isEmpty() ) {
		// don't accept a box font engine... retry with the
		// next family in the list (if any)
		engine = 0;
	    }
	    continue;
	}

	// not found in cache, try to load it...
	engine = QFontDatabase::findFont( script, req, screen );

	if ( ! engine ) {
	    // couldn't load it, put a box font engine in the cache to
	    // prevent multiple calls to QFontDatabase::findFont
	    engine = new QFontEngineBox( px );
	    QFontCache::instance->insertEngine( key, engine );

	    if ( ! req.family.isEmpty() ) {
		// don't accept a box font engine... retry with the
		// next family in the list (if any)
		engine = 0;
	    }
	} else {
	    // insert the new font engine into the font cache
	    QFontCache::instance->insertEngine( key, engine );
	}
    }

    engine->ref();
    engineData->engines[script] = engine;
    // initFontInfo( script, scale );
    // request.dirty = FALSE;
}

HFONT QFont::handle() const
{
    QFontEngine *engine = d->engineForScript( QFont::NoScript );
    return engine->hfont;
}

QString QFont::rawName() const
{
    return family();
}

void QFont::setRawName( const QString &name )
{
    setFamily( name );
}


bool QFont::dirty() const
{
    return !d->engineData;
}


QString QFont::defaultFamily() const
{
    switch( d->request.styleHint ) {
	case QFont::Times:
	    return QString::fromLatin1("Times New Roman");
	case QFont::Courier:
	    return QString::fromLatin1("Courier New");
	case QFont::Decorative:
	    return QString::fromLatin1("Bookman Old Style");
	case QFont::Helvetica:
	    return QString::fromLatin1("Arial");
	case QFont::System:
	default:
	    return QString::fromLatin1("MS Sans Serif");
    }
}

QString QFont::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}

QString QFont::lastResortFont() const
{
    return QString::fromLatin1("arial");
}



/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

#define IS_TRUETYPE (QT_WA_INLINE( engine->tm.w.tmPitchAndFamily, engine->tm.a.tmPitchAndFamily ) & TMPF_TRUETYPE)
#define TMX engine->tm.w
#define TMW engine->tm.w
#define TMA engine->tm.a

int QFontMetrics::ascent() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return engine->ascent();
}

int QFontMetrics::descent() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return engine->descent();
}

bool QFontMetrics::inFont(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );

    QFontEngine *engine = d->engineForScript( script );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    if ( engine->type() == QFontEngine::Box ) return FALSE;
    return engine->canRender( &ch, 1 );
}

int QFontMetrics::leftBearing(QChar ch) const
{
#ifdef Q_OS_TEMP
    return 0;
#else
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    if ( IS_TRUETYPE ) {
	ABC abc;
	QT_WA( {
	    uint ch16 = ch.unicode();
	    GetCharABCWidths(engine->dc(),ch16,ch16,&abc);
	} , {
	    uint ch8;
	    if ( ch.row() || ch.cell() > 125 ) {
		QCString w = QString(ch).local8Bit();
		if ( w.length() != 1 )
		    return 0;
		ch8 = w[0];
	    } else {
		ch8 = ch.cell();
	    }
	    GetCharABCWidthsA(engine->dc(),ch8,ch8,&abc);
	} );
	return abc.abcA;
    } else {
	QT_WA( {
	    uint ch16 = ch.unicode();
	    ABCFLOAT abc;
	    GetCharABCWidthsFloat(engine->dc(),ch16,ch16,&abc);
	    return int(abc.abcfA);
	} , {
	    return 0;
	} );
    }
    return 0;
#endif
}


int QFontMetrics::rightBearing(QChar ch) const
{
#ifdef Q_OS_TEMP
	return 0;
#else
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    if ( IS_TRUETYPE ) {
	ABC abc;
	QT_WA( {
	    uint ch16 = ch.unicode();
	    GetCharABCWidths(engine->dc(),ch16,ch16,&abc);
	    return abc.abcC;
	} , {
	    uint ch8;
	    if ( ch.row() || ch.cell() > 125 ) {
		QCString w = QString(ch).local8Bit();
		if ( w.length() != 1 )
		    return 0;
		ch8 = w[0];
	    } else {
		ch8 = ch.cell();
	    }
	    GetCharABCWidthsA(engine->dc(),ch8,ch8,&abc);
	} );
	return abc.abcC;
    } else {
	QT_WA( {
	    uint ch16 = ch.unicode();
	    ABCFLOAT abc;
	    GetCharABCWidthsFloat(engine->dc(),ch16,ch16,&abc);
	    return int(abc.abcfC);
	} , {
	    return -TMW.tmOverhang;
	} );
    }

    return 0;
#endif
}


int QFontMetrics::minLeftBearing() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return engine->minLeftBearing();
}

int QFontMetrics::minRightBearing() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return engine->minRightBearing();
}


int QFontMetrics::height() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return engine->ascent() + engine->descent();
}


int QFontMetrics::leading() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return engine->leading();
}

int QFontMetrics::lineSpacing() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return engine->leading() + engine->ascent() + engine->descent();
}

int QFontMetrics::width( QChar ch ) const
{
    if ( ch.category() == QChar::Mark_NonSpacing )
	return 0;

    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    if ( qt_winver & Qt::WV_NT_based && painter )
	painter->nativeXForm( TRUE );

    SIZE s = {0,0};
    wchar_t tc = ch.unicode();
    BOOL res = GetTextExtentPoint32W( engine->dc(), &tc, 1, &s );

    if ( qt_winver & Qt::WV_NT_based && painter )
	painter->nativeXForm( FALSE );

#ifndef QT_NO_DEBUG
    if ( !res )
	qSystemWarning( "QFontMetrics::width: GetTextExtentPoint32 failed" );
#endif
    if ( (qt_winver & Qt::WV_NT_based) == 0 )
	s.cx -= TMX.tmOverhang;
    return s.cx;
}


int QFontMetrics::width( const QString &str, int len ) const
{
    if ( len < 0 )
	len = str.length();
    if ( len == 0 )
	return 0;

    QTextEngine layout( str, d );
    layout.itemize( FALSE );
    return layout.width( 0, len );
}

int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    if ( qt_winver & Qt::WV_NT_based && painter )
	painter->nativeXForm( TRUE );

    QTextEngine layout( str,  d );
    layout.itemize( FALSE );
    int w = layout.width( pos, 1 );

    if ( qt_winver & Qt::WV_NT_based && painter )
	painter->nativeXForm( FALSE );

    if ( (qt_winver & Qt::WV_NT_based) == 0 ) {
	QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
	Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE
	w -= TMX.tmOverhang;
    }
    return w;
}

QRect QFontMetrics::boundingRect( QChar ch ) const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    glyph_t glyphs[10];
    int nglyphs = 9;
    advance_t advances[10];
    engine->stringToCMap( &ch, 1, glyphs, advances, &nglyphs );
    glyph_metrics_t gi = engine->boundingBox( glyphs[0] );
    return QRect( gi.x, gi.y, gi.width, gi.height );
}


QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    if (len < 0)
	len = str.length();
    if (len == 0)
	return QRect();

    QTextEngine layout( str, d );
    layout.itemize( FALSE );
    glyph_metrics_t gm = layout.boundingBox( 0, len );
    return QRect( gm.x, gm.y, gm.width, gm.height );
}

int QFontMetrics::maxWidth() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return TMX.tmMaxCharWidth;
}

int QFontMetrics::underlinePos() const
{
    int pos = (lineWidth()*2 + 3)/6;
    return QMAX(pos,1);
}

int QFontMetrics::strikeOutPos() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    int pos = TMX.tmAscent/3;
    return QMAX(pos,1);
}

int QFontMetrics::lineWidth() const
{
    if ( !d->engineData )
	d->load( QFont::NoScript );

    return d->engineData->lineWidth;
}

