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

#ifndef QT_H
#include <qcache.h>
#include <qobject.h>
#include <qpaintdevice.h>
#endif // QT_H
#include <limits.h>

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

class QPaintDevice;

#ifdef Q_WS_WIN
#include <qt_windows.h>
#endif

#include <qfont.h>

// font description
struct QFontDef {
    QFontDef()
	: pixelSize(0), pointSize(0), lbearing(SHRT_MIN), rbearing(SHRT_MIN),
	  styleStrategy(QFont::PreferDefault), styleHint(QFont::AnyStyle),
	  weight(0), italic(FALSE), underline(FALSE), strikeOut(FALSE),
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

    bool italic;
    bool underline;
    bool strikeOut;
    bool fixedPitch;
    bool hintSetByUser;
    bool rawMode;

    bool dirty;
};


class QTextCodec;

#ifdef Q_WS_X11

struct QGlyphMetrics;
class QChar;
typedef unsigned short glyph_t;
struct offset_t;
class QOpenType;

class QFontEngine : public QShared
{
public:
    enum Error {
	NoError,
	OutOfMemory
    };

    enum Type {
	Box,
	Xlfd,
	Xft
    };

    virtual ~QFontEngine() = 0;

    /* returns 0 as glyph index for non existant glyphs */
    virtual Error stringToCMap( const QChar *str, int len, glyph_t *glyphs, int *nglyphs ) const = 0;

    virtual QOpenType *openType() const { return 0; }
    virtual int cmap() const = 0;

    virtual void draw( QPainter *p, int x, int y, const glyph_t *glyphs,
		       const offset_t *advances, const offset_t *offsets, int numGlyphs, bool reverse ) = 0;

    virtual QGlyphMetrics boundingBox( const glyph_t *glyphs,
				    const offset_t *advances, const offset_t *offsets, int numGlyphs ) = 0;
    virtual QGlyphMetrics boundingBox( glyph_t glyph ) = 0;

    virtual int ascent() const = 0;
    virtual int descent() const = 0;
    virtual int leading() const = 0;
    virtual int maxCharWidth() const = 0;

    virtual const char *name() const = 0;

    virtual bool canRender( const QChar *string,  int len ) = 0;

    virtual void setScale( double ) {}
    virtual int scale() const { return 1; }

    virtual Type type() const = 0;
};


inline QFontEngine::~QFontEngine()
{
}

typedef QFontEngine QFontStruct;

enum { widthCacheSize = 0x500 };

class QFontX11Data  // used as a QFontPrivate member
{
public:
    // X fontstruct handles for each character set
    QFontStruct *fontstruct[QFont::LastPrivateScript];

    uchar widthCache[widthCacheSize];

    QFontX11Data();
    ~QFontX11Data();
};

#endif // Q_WS_X11


#ifdef Q_WS_WIN

class QFontStruct : public QShared
{
public:
    QFontStruct( const QString &key );
    ~QFontStruct() { reset(); }
    bool	    dirty()      const { return hfont == 0; }
    HDC		    dc()	 const;
    HFONT	    font()	 const { return hfont; }
    const TEXTMETRICA	   *textMetricA() const { return &tm.a; }
    const TEXTMETRICW	   *textMetricW() const { return &tm.w; }
    QString key() const  { return k; }
    void	    reset();

    QString	k;
    HDC		hdc;
    HFONT	hfont;
    uint	stockFont:1;
    uint	paintDevice:1;
    uint        useTextOutA:1;
    union {
	TEXTMETRICW	w;
	TEXTMETRICA	a;
    } tm;
    int		lw;
    int cache_cost;
//    friend void QFont::initFontInfo() const;
};

#endif // Q_WS_WIN

#if defined( Q_WS_MAC )

#if defined( Q_WS_MACX )
# define QMAC_FONT_ATSUI
#endif
#include "qt_mac.h"
class QMacFontInfo;

class QFontStruct : public QShared
{
public:
    inline QFontStruct() :   QShared(), info(NULL), fnum(-1), cache_cost(0), internal_fi(NULL) { }
#if defined( QMAC_FONT_ATSUI ) && 0
    ATSFontMetrics *info;
    int maxWidth() const { return (int)info->maxAdvanceWidth; }
#else
    FontInfo *info;
    int maxWidth() const { return info->widMax; }
#endif
    int ascent() const { return (int)info->ascent; }
    int descent() const { return (int)info->descent; }
    int leading() const { return (int)info->leading; }
    int minLeftBearing() const { return 0; }
    int minRightBearing() const { return 0; }

    short fnum;
    int psize, cache_cost;
    QMacFontInfo *internal_fi;
};

#endif

#ifdef Q_WS_QWS
class QFontStruct;
class QGfx;
#endif

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


protected:


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

    static int getFontWeight(const QCString &, bool = FALSE);
    QRect boundingRect( const QChar &ch );

#ifndef Q_WS_X11
    struct TextRun {
	TextRun()
	{
	    xoff = 0;
	    yoff = 0;
	    x2off = 0;
	    script = QFont::NoScript;
	    string = 0;
	    length = 0;
	    next = 0;
	}

	~TextRun()
	{
	    if ( next )
		delete next;
	}

	void setParams( int x, int y, int x2, const QChar *s, int len,
			QFont::Script sc = QFont::NoScript ) {
	    xoff = x;
	    yoff = y;
	    x2off = x2;
	    string = s;
	    length = len;
	    script = sc;
	}
	int xoff;
	int yoff;
	int x2off;
	QFont::Script script;
	const QChar *string;
	int length;
	TextRun *next;
    };

    // some replacement functions for native calls. This is needed, because shaping and
    // non spacing marks can change the extents of a string to draw. At the same time
    // drawing needs to take care to correctly position non spacing marks.
    int textWidth( const QString &str, int pos, int len );
#endif

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
    void load();
    void initFontInfo();
    HFONT create( bool *stockFont, HDC hdc = 0, bool compatMode = FALSE );
    QFontStruct *fin;

    void buildCache( HDC hdc, const QString &str, int pos, int len, TextRun *cache );
    void drawText( HDC hdc, int x, int y, TextRun *cache );
#endif // Q_WS_WIN

#ifdef Q_WS_QWS
    void load();
    QFontStruct *fin;
    int textWidth( const QString &str, int pos, int len, TextRun *cache );
    void drawText( QGfx *gfx, int x, int y, const TextRun *cache );
#endif

#if defined( Q_WS_MAC )
    void macSetFont(QPaintDevice *);
    void drawText(int x, int y, const QString &s, int len, QPaintDevice *dev, const QRegion *rgn, int dir);
    void computeLineWidth();
    void load();
    QFontStruct *fin;
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

#endif // QFONTDATA_P_H
