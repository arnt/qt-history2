/****************************************************************************
** $Id$
**
** Definition of internal QFontData struct
**
** Created : 941229
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#ifndef QFONTDATA_P_H
#define QFONTDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//

#include "qobject.h"
#include "qfont.h"

// forwards
class QFontEngine;
class QPaintDevice;


struct QFontDef
{
    QFontDef()
	: family_hash( 0 ), pointSize( -1 ), pixelSize( -1 ),
	  styleHint( QFont::AnyStyle ), styleStrategy( QFont::PreferDefault ),
	  weight( 50 ), italic( FALSE ), underline( FALSE ), strikeOut( FALSE ),
	  fixedPitch( FALSE ), stretch( 100 ), mask( 0 )
    {
    }

    QString family;
    uint family_hash;

#ifdef Q_WS_X11
    QString addStyle;
#endif // Q_WS_X11

    int pointSize;
    int pixelSize;

    uint styleHint     : 8;
    uint styleStrategy : 16;

    uint weight     :  7; // 0-99
    uint italic     :  1;
    uint underline  :  1;
    uint strikeOut  :  1;
    uint fixedPitch :  1;
    uint stretch    : 12; // 0-400

    enum {
	Family        = 0x0001,
	Size          = 0x0002,
	StyleHint     = 0x0004,
	StyleStrategy = 0x0008,
	Weight        = 0x0010,
	Italic        = 0x0020,
	Underline     = 0x0040,
	StrikeOut     = 0x0080,
	FixedPitch    = 0x0100,
	Stretch       = 0x0200,

	RawMode       = 0x10000000
    };

    uint mask;

    inline bool operator<( const QFontDef &other ) const
    {
	if ( pixelSize != other.pixelSize ) return pixelSize < other.pixelSize;
	if ( weight != other.weight ) return weight < other.weight;
	if ( italic != other.italic ) return italic < other.italic;
	if ( underline != other.underline ) return underline < other.underline;
	if ( fixedPitch != other.fixedPitch ) return fixedPitch < other.fixedPitch;
	if ( stretch != other.stretch ) return stretch < other.stretch;
	if ( styleHint != other.styleHint ) return styleHint < other.styleHint;
	if ( styleStrategy != other.styleStrategy ) return styleStrategy < other.styleStrategy;
	if ( family_hash != other.family_hash ) return family_hash < other.family_hash;
	if ( family != other.family ) return family < other.family;

#ifdef Q_WS_X11
	if ( addStyle != other.addStyle ) return addStyle < other.addStyle;
#endif // Q_WS_X11

	return FALSE;
    }
    inline bool operator==( const QFontDef &other ) const
    {
	return ( family_hash   == other.family_hash   &&
		 pointSize     == other.pointSize     &&
		 pixelSize     == other.pixelSize     &&
		 styleHint     == other.styleHint     &&
		 styleStrategy == other.styleStrategy &&
		 weight        == other.weight        &&
		 italic        == other.italic        &&
		 underline     == other.underline     &&
		 strikeOut     == other.strikeOut     &&
		 fixedPitch    == other.fixedPitch    &&
		 stretch       == other.stretch       &&
		 family        == other.family
#ifdef Q_WS_X11
		 && addStyle == other.addStyle
#endif // Q_WS_X11
		 );
    }

    inline void spew() const
    {
    }
};

class QFontEngineData : public QShared
{
public:
    QFontEngineData();
    ~QFontEngineData();

    short lbearing;
    short rbearing;
    uint lineWidth;

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    QFontEngine *engines[QFont::LastPrivateScript];

    enum { widthCacheSize = 0x500 };
    uchar widthCache[widthCacheSize];
#else
    QFontEngine *engine;
#endif // Q_WS_X11 || Q_WS_WIN
};


class QFontPrivate : public QShared
{
public:
    static QFont::Script defaultScript;

    QFontPrivate();
    QFontPrivate( const QFontPrivate &other );
    ~QFontPrivate();

    void load( QFont::Script script );
    QFontEngine *engineForScript( QFont::Script script ) const {
	if ( script == QFont::NoScript )
	    script = QFontPrivate::defaultScript;
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
	if ( ! engineData || ! engineData->engines[script] )
	    ((QFontPrivate *) this)->load( script );
	return engineData->engines[script];
#else
        if ( ! engineData || ! engineData->engine )
	  ((QFontPrivate *) this)->load( script );
        return engineData->engine;
#endif // Q_WS_X11 || Q_WS_WIN
    }

    QFontDef request;
    QFontEngineData *engineData;
    QPaintDevice *paintdevice;
    int screen;
};


class QFontCache : public QObject
{
public:
    static QFontCache *instance;

    QFontCache();
    ~QFontCache();

    // universal key structure.  QFontEngineDatas and QFontEngines are cached using
    // the same keys
    struct Key {
	Key() : screen( 0 ) { }
	Key( const QFontDef &d, QFont::Script c, int s = 0 )
	    : def( d ), script( c ), screen( s ) { }

	QFontDef def;
	int script;
	int screen;

	inline bool operator<( const Key &other ) const
	{
	    if ( script != other.script ) return script < other.script;
	    if ( screen != other.screen ) return screen < other.screen;
	    return def < other.def;
	}
	inline bool operator==( const Key &other ) const
	{ return def == other.def && script == other.script && screen == other.screen; }
    };

    // QFontEngineData cache
    typedef QMap<Key,QFontEngineData*> EngineDataCache;
    EngineDataCache engineDataCache;

    QFontEngineData *findEngineData( const Key &key ) const;
    void insertEngineData( const Key &key, QFontEngineData *engineData );

    // QFontEngine cache
    typedef QMap<Key,QFontEngine*> EngineCache;
    EngineCache engineCache;

    QFontEngine *findEngine( const Key &key ) const;
    void insertEngine( const Key &key, QFontEngine *engine );

    // need timer handler to automatically adjust the cache size and
    // clean out old stuff
};































#if 0

#ifndef QT_H
#include "qcache.h"
#include "qobject.h"
#include "qpaintdevice.h"
#endif // QT_H
#include <limits.h>

#ifdef Q_WS_WIN
#include "qt_windows.h"
#endif

#include "qfont.h"

// font description
struct QFontDef {
    QFontDef()
	: pixelSize(0), pointSize(0), lbearing(SHRT_MIN), rbearing(SHRT_MIN),
	  styleStrategy(QFont::PreferDefault), styleHint(QFont::AnyStyle),
	  weight(0), stretch( 100 ),
	  italic(FALSE), underline(FALSE), strikeOut(FALSE),
	  fixedPitch(FALSE), hintSetByUser(FALSE), rawMode(FALSE), dirty(TRUE)
    { ; }

    QString family;
    QString addStyle;

    int pixelSize;
    int pointSize;
    short lbearing;
    short rbearing;

    ushort styleStrategy;
    uchar styleHint;
    uchar weight;

    uint stretch       : 12;

    bool italic        : 1;
    bool underline     : 1;
    bool strikeOut     : 1;
    bool fixedPitch    : 1;
    bool hintSetByUser : 1;
    bool rawMode       : 1;

    bool dirty         : 1;
};


class QTextCodec;


struct glyph_metrics_t;
class QChar;
typedef unsigned short glyph_t;
struct offset_t;
typedef int advance_t;
class QOpenType;

#if defined (Q_WS_WIN) || defined (Q_WS_MAC)
class QFontEngine : public QShared
{
public:
    enum Error {
	NoError,
	OutOfMemory
    };

#ifdef Q_WS_X11
    enum Type {
	Box,
	Xlfd,
	Xft
    };
#elif defined( Q_WS_WIN )
    enum Type {
	Win,
	Uniscribe
    };
#elif defined( Q_WS_MAC )
    enum Type {
	Mac
    };
#endif

    virtual ~QFontEngine();

    /* returns 0 as glyph index for non existant glyphs */
    virtual Error stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const = 0;

#ifdef Q_WS_X11
    virtual QOpenType *openType() const { return 0; }
#endif

    virtual void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
		       const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse ) = 0;

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
				    const advance_t *advances, const offset_t *offsets, int numGlyphs ) = 0;
    virtual glyph_metrics_t boundingBox( glyph_t glyph ) = 0;

    virtual int ascent() const = 0;
    virtual int descent() const = 0;
    virtual int leading() const = 0;
    virtual int maxCharWidth() const = 0;

    virtual const char *name() const = 0;

    virtual bool canRender( const QChar *string,  int len ) = 0;

    virtual void setScale( double ) {}
    virtual int scale() const { return 1; }

    virtual Type type() const = 0;

    int cache_cost;
#ifdef Q_WS_WIN
    HDC dc() const;
    void getGlyphIndexes( const QChar *ch, int numChars, glyph_t *glyphs ) const;
    void getCMap();

    QCString	_name;
    HDC		hdc;
    HFONT	hfont;
    uint	stockFont   : 1;
    uint	paintDevice : 1;
    uint        useTextOutA : 1;
    uint        ttf         : 1;
    union {
	TEXTMETRICW	w;
	TEXTMETRICA	a;
    } tm;
    int		lw;
    unsigned char *cmap;
    void *script_cache;
#endif
};
#elif defined( Q_WS_QWS )
class QGfx;

class QFontEngine : public QShared
{
public:
    QFontEngine( const QFontDef& );
   ~QFontEngine();
    /*QMemoryManager::FontID*/ void *handle() const;

    enum Error {
	NoError,
	OutOfMemory
    };
    /* returns 0 as glyph index for non existant glyphs */
    Error stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs ) const;

    void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
	       const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse );

    glyph_metrics_t boundingBox( const glyph_t *glyphs,
			       const advance_t *advances, const offset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;
    int minLeftBearing() const;
    int minRightBearing() const;
    int underlinePos() const;
    int lineWidth() const;

    bool canRender( const QChar *string,  int len );

    QFontDef s;
    /*QMemoryManager::FontID*/ void *id;
    int cache_cost;
};
#endif // WIN || X11 || MAC


#ifdef Q_WS_X11
enum { widthCacheSize = 0x500 };

class QFontX11Data  // used as a QFontPrivate member
{
public:
    // X fontstruct handles for each character set
    QFontEngine *fontstruct[QFont::LastPrivateScript];

    uchar widthCache[widthCacheSize];

    QFontX11Data();
    ~QFontX11Data();
};

#endif

typedef QFontEngine QFontStruct;

typedef QCacheIterator<QFontStruct> QFontCacheIterator;
class QFontCache : public QObject, public QCache<QFontStruct>
{
public:
    QFontCache();
    ~QFontCache();

    bool insert(const QString &, const QFontStruct *, int c);
#ifndef Q_WS_MAC
    void deleteItem(Item d);
#endif
    void timerEvent(QTimerEvent *);

private:
    int timer_id;
    bool fast;
};


// QFontPrivate - holds all data on which a font operates
class QFontPrivate : public QShared
{
public:
    static QFontCache *fontCache;
public:
    QFontPrivate();
    QFontPrivate(const QFontPrivate &fp);
    QFontPrivate( const QFontPrivate &fp, QPaintDevice *pd );
    ~QFontPrivate();

    // requested font
    QFontDef request;
    // actual font
    QFontDef actual;

    bool exactMatch;
    int lineWidth;

    // common functions
    QString defaultFamily() const;
    QString lastResortFamily() const;
    QString lastResortFont() const;
    QString key() const;
    static QString key( const QFontDef & );

    static int getFontWeight(const QCString &, bool = FALSE);

#ifdef Q_WS_X11
    QFontEngine *engineForScript( QFont::Script script ) const;

    static char **getXFontNames(const char *, int *);
    static bool fontExists(const QString &);
    static bool parseXFontName(char *, char **);
    static QCString fixXLFD( const QCString & );
    static bool fillFontDef(/*XFontStruct*/ void *, QFontDef *, int);
    static bool fillFontDef(const QCString &, QFontDef *, int);

    // XLFD fields
    enum FontFieldNames {
	Foundry,
	Family,
	Weight,
	Slant,
	Width,
	AddStyle,
	PixelSize,
	PointSize,
	ResolutionX,
	ResolutionY,
	Spacing,
	AverageWidth,
	CharsetRegistry,
	CharsetEncoding,
	NFontFields
    };

    void initFontInfo(QFont::Script, double scale);
    void load(QFont::Script = QFont::NoScript, bool = TRUE);
    void computeLineWidth();

    QFontX11Data x11data;
    static QFont::Script defaultScript;
    int x11Screen;
#endif // Q_WS_X11


    QPaintDevice *paintdevice;

#ifdef Q_WS_WIN
    QFontEngine *engineForScript( QFont::Script ) const { ((QFontPrivate*)this)->load(); return fin; }
    void load();
    void initFontInfo();
    HFONT create( bool *stockFont, HDC hdc = 0, bool compatMode = FALSE );
    QFontEngine *fin;
#endif // Q_WS_WIN

#ifdef Q_WS_QWS
    QFontEngine *engineForScript( QFont::Script ) const { ((QFontPrivate*)this)->load(); return fin; }
    void load();
    QFontEngine *fin;
#endif

#if defined( Q_WS_MAC )
    QFontEngine *engineForScript(QFont::Script) const { ((QFontPrivate*)this)->load(); return fin; }
    void computeLineWidth();
    void load();
    QFontEngine *fin;
#endif

};

inline QFontPrivate::QFontPrivate()
    : QShared(), exactMatch(FALSE), lineWidth(1)
{

#if defined(Q_WS_WIN) || defined(Q_WS_QWS) || defined(Q_WS_MAC)
    fin = 0;
#endif // Q_WS_WIN || Q_WS_QWS
#if defined(Q_WS_X11)
    x11Screen = QPaintDevice::x11AppScreen();
#endif // Q_WS_X11
    paintdevice = 0;
}

inline QFontPrivate::QFontPrivate(const QFontPrivate &fp)
    : QShared(), request(fp.request), actual(fp.actual),
exactMatch(fp.exactMatch), lineWidth(1)
{
    Q_ASSERT(!fp.paintdevice);
#if defined(Q_WS_WIN) || defined(Q_WS_QWS) || defined(Q_WS_MAC)
    fin = 0;
#endif // Q_WS_WIN || Q_WS_QWS
#if defined(Q_WS_X11)
    x11Screen = fp.x11Screen;
#endif // Q_WS_X11
    paintdevice = 0;
}

inline QFontPrivate::QFontPrivate( const QFontPrivate &fp, QPaintDevice *pd )
    : QShared(), request(fp.request), actual(fp.actual),
exactMatch(fp.exactMatch), lineWidth(1)
{

#if defined(Q_WS_WIN) || defined(Q_WS_QWS) || defined(Q_WS_MAC)
    fin = 0;
#endif // Q_WS_WIN || Q_WS_QWS
#if defined(Q_WS_X11)
    x11Screen = pd->x11Screen();
#endif // Q_WS_X11
    paintdevice = pd;
}

#ifndef Q_WS_QWS
inline QFontPrivate::~QFontPrivate()
{
#if defined(Q_WS_WIN)
    if( fin )
	fin->deref();
#endif
#if defined(Q_WS_MAC)
    if( fin && fin->deref() )
	delete fin;
#endif
}
#endif

#endif // 0

#endif // QFONTDATA_P_H
