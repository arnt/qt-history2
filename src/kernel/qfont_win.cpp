/****************************************************************************
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for Win32.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// the miscrosoft platform SDK says that the Unicode versions of
// TextOut and GetTextExtentsPoint32 are supported on all platforms, so we use them
// exclusively to simplify code, save a lot of conversions into the local encoding
// and have generally better unicode support :)

#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"

#include "qwidget.h"
#include "qpainter.h"
#include <limits.h>
#include "qt_windows.h"
#include "qapplication_p.h"
#include "qapplication.h"
#include "qpaintdevicemetrics.h"
#include <private/qunicodetables_p.h>
#include <qfontdatabase.h>


extern HDC   shared_dc;		// common dc for all fonts


// ### maybe move to qapplication_win
QFont qt_LOGFONTtoQFont(LOGFONT& lf, bool /*scale*/)
{
    QString family = QT_WA_INLINE( QString::fromUcs2((ushort*)lf.lfFaceName),
				   QString::fromLocal8Bit((char*)lf.lfFaceName) );
    QFont qf(family);
    qf.setItalic(lf.lfItalic);
    if (lf.lfWeight != FW_DONTCARE) {
	int weight;
	if ( lf.lfWeight < 400 )
	    weight = QFont::Light;
	else if ( lf.lfWeight < 600 )
	    weight = QFont::Normal;
	else if ( lf.lfWeight < 700 )
	    weight = QFont::DemiBold;
	else if ( lf.lfWeight < 800 )
	    weight = QFont::Bold;
	else
	    weight = QFont::Black;
	qf.setWeight(weight);
    }
    int lfh = QABS( lf.lfHeight );
    Q_ASSERT(shared_dc);
    qf.setPointSizeFloat( lfh * 72.0 / GetDeviceCaps(shared_dc,LOGPIXELSY) );
    qf.setUnderline(FALSE);
    qf.setOverline(FALSE);
    qf.setStrikeOut(FALSE);
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
    if ( !shared_dc )
	qSystemWarning( "QFont::initialize() (qfont_win.cpp, 163): couldn't create device context" );
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
    // NOTE: the X11 and Windows implementations of this function are
    // identical... if you change one, change both.

    // sanity checks
    Q_ASSERT( QFontCache::instance != 0);
    Q_ASSERT( script >= 0 && script < QFont::LastPrivateScript );

    QFontDef req = request;
    int px = int( pixelSize( req, paintdevice, screen ) + .5 );
    req.pixelSize = px;
    req.pointSize = 0;

    if ( ! engineData ) {
	QFontCache::Key key( req, QFont::NoScript, (int)paintdevice );

	// look for the requested font in the engine data cache
	engineData = QFontCache::instance->findEngineData( key );

	if ( ! engineData) {
	    // create a new one
	    engineData = new QFontEngineData;
	    QFontCache::instance->insertEngineData( key, engineData );
	} else {
	    engineData->ref();
	}
    }

    // the cached engineData could have already loaded the engine we want
    if ( engineData->engines[script] ) return;

    // load the font
    QFontEngine *engine = 0;
    //    double scale = 1.0; // ### TODO: fix the scale calculations

    // list of families to try
    QStringList family_list = QStringList::split( ',', req.family );

    // append the substitute list for each family in family_list
    QStringList subs_list;
    QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
    for ( ; it != end; ++it )
	subs_list += QFont::substitutes( *it );
    family_list += subs_list;

    if(qWinVersion() & Qt::WV_DOS_based && req.family.toLower() == "ms sans serif") {
	// small hack for Dos based machines to get the right font for non
	// latin text when using the default font.
	family_list << "Arial" << "Tahoma" << "Verdana";
    }
    // append the default fallback font for the specified script
    // family_list << ... ; ###########

    // add the default family
    QString defaultFamily = QApplication::font().family();
    if ( ! family_list.contains( defaultFamily ) )
	family_list << defaultFamily;

    // add QFont::defaultFamily() to the list, for compatibility with
    // previous versions
    family_list << QApplication::font().defaultFamily();

    // null family means find the first font matching the specified script
    family_list << QString::null;

    it = family_list.begin(), end = family_list.end();
    for ( ; ! engine && it != end; ++it ) {
	req.family = *it;

	engine = QFontDatabase::findFont( script, this, req );
	if ( engine ) {
	    if ( engine->type() != QFontEngine::Box )
		break;

	    if ( ! req.family.isEmpty() )
		engine = 0;

	    continue;
	}
    }

    engine->ref();
    engineData->engines[script] = engine;
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

int QFontMetrics::leftBearing(QChar ch) const
{
#ifdef Q_OS_TEMP
    return 0;
#else
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    Q_ASSERT( engine != 0 );

    if ( IS_TRUETYPE ) {
	ABC abc;
	QT_WA( {
	    uint ch16 = ch.unicode();
	    GetCharABCWidths(engine->dc(),ch16,ch16,&abc);
	} , {
	    uint ch8;
	    if ( ch.row() || ch.cell() > 127 ) {
		QByteArray w = QString(ch).toLocal8Bit();
		if ( w.length() != 1 )
		    return 0;
		ch8 = (uchar)w[0];
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
    Q_ASSERT( engine != 0 );

    if ( IS_TRUETYPE ) {
	ABC abc;
	QT_WA( {
	    uint ch16 = ch.unicode();
	    GetCharABCWidths(engine->dc(),ch16,ch16,&abc);
	    return abc.abcC;
	} , {
	    uint ch8;
	    if ( ch.row() || ch.cell() > 127 ) {
		QByteArray w = QString(ch).toLocal8Bit();
		if ( w.length() != 1 )
		    return 0;
		ch8 = (uchar)w[0];
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


int QFontMetrics::width( QChar ch ) const
{
    if ( ::category( ch ) == QChar::Mark_NonSpacing )
	return 0;

    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );

    QFontEngine *engine = d->engineForScript( script );
    Q_ASSERT( engine != 0 );

    glyph_t glyphs[8];
    advance_t advances[8];

    int nglyphs = 7;
    engine->stringToCMap( &ch, 1, glyphs, advances, &nglyphs, FALSE );

    return advances[0];
}


int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    if ( pos < 0 || pos > (int)str.length() )
	return 0;

    QTextEngine layout( str,  d );
    layout.itemize( QTextEngine::WidthOnly );
    int w = layout.width( pos, 1 );

    if ( (qWinVersion() & Qt::WV_NT_based) == 0 ) {
	QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
	Q_ASSERT( engine != 0 );
	w -= TMX.tmOverhang;
    }
    return w;
}
