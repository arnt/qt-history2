/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdata_p.h#51 $
**
** Definition of internal QFontData struct
**
** Created : 941229
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
#endif // QT_H


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

#ifndef QT_H
#endif // QT_H

#ifdef Q_WS_WIN
#include <qt_windows.h>
#endif

#ifdef Q_WS_X11
#include <qt_x11.h>
#endif

// font description
struct QFontDef {
    QFontDef()
	: pixelSize(0), pointSize(0), lbearing(SHRT_MIN), rbearing(SHRT_MIN),
	  styleHint(QFont::AnyStyle), styleStrategy(QFont::PreferDefault),
	  weight(0), italic(FALSE), underline(FALSE), strikeOut(FALSE),
	  fixedPitch(FALSE), hintSetByUser(FALSE), rawMode(FALSE), dirty(TRUE)
    { ; }

    QString family;
    QString addStyle;

    int pixelSize;
    int pointSize;
    short lbearing;
    short rbearing;

    uchar styleHint;
    uchar styleStrategy;
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

// this is a shared wrapper for XFontStruct (to prevent a font being freed by
// the cache while it's being used)
class QFontStruct : public QShared
{
public:
    QFontStruct(Qt::HANDLE h, Qt::HANDLE xfth, QCString n, QTextCodec *c, int a) :
	QShared(), handle(h), xfthandle(xfth), name(n), codec(c), cache_cost(a)
    { ; }

    ~QFontStruct();

    Qt::HANDLE handle, xfthandle;
    QCString name;
    QTextCodec *codec;
    int cache_cost;
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
    TEXTMETRICA	   *textMetricA() const;
    TEXTMETRICW	   *textMetricW() const;
    const QFontDef *spec()	 const { return &s; }
	QString key() const  { return k; }
    void	    reset();

    QString	k;
    HDC		hdc;
    HFONT	hfont;
    bool	stockFont;
    union {
	TEXTMETRICW	w;
	TEXTMETRICA	a;
    } tm;
    QFontDef	s;
    int		lw;
	int cache_cost;
//    friend void QFont::initFontInfo() const;
};

#endif // Q_WS_WIN

#if defined( Q_WS_MAC )

#include "qt_mac.h"
class QMacFontInfo;

class QFontStruct : public QShared
{
public:
    inline QFontStruct( const QFontDef& d ) :   QShared(), s(d), info(NULL), cache_cost(0), internal_fi(NULL) { }
    inline const QFontDef *spec()  const { return &s; }
    int ascent() const { return info->ascent; }
    int descent() const { return info->descent; }
    int minLeftBearing() const { return 0; }
    int minRightBearing() const { return 0; }
    int leading() const { return info->leading; }
    int maxWidth() const { return info->widMax; }

    int psize;
    QFontDef s;
    FontInfo *info;
    int cache_cost;
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
    void deleteItem(Item);

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
    QFontPrivate()
	: QShared(), exactMatch(FALSE), lineWidth(1)
    {

#if defined(Q_WS_WIN) || defined(Q_WS_QWS) || defined(Q_WS_MAC)
	fin = 0;
#endif // Q_WS_WIN || Q_WS_QWS
#if defined(Q_WS_WIN)
	currHDC = 0;
#endif // Q_WS_WIN

    }

    QFontPrivate(const QFontPrivate &fp)
	: QShared(fp), request(fp.request), actual(fp.actual),
	  exactMatch(fp.exactMatch), lineWidth(1)
    {

#if defined(Q_WS_WIN) || defined(Q_WS_QWS) || defined(Q_WS_MAC)
	fin = 0;
#endif // Q_WS_WIN || Q_WS_QWS
#if defined(Q_WS_WIN)
	currHDC = 0;
#endif // Q_WS_WIN

    }

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
#ifdef Q_WS_WIN
	    mapped = 0;
#endif
	}

	~TextRun()
	{
	    if ( next )
		delete next;
#ifdef Q_WS_WIN
#if !(defined(UNICODE) && !defined(Q_OS_WIN32BYTESWAP_))
	    delete [] mapped;
#endif
#endif
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
#ifdef Q_WS_X11
	QByteArray mapped;
#endif
#ifdef Q_WS_WIN
	TCHAR *mapped;
#endif
    };

    // some replacement functions for native calls. This is needed, because shaping and
    // non spacing marks can change the extents of a string to draw. At the same time
    // drawing needs to take care to correctly position non spacing marks.
    int textWidth( const QString &str, int pos, int len );

    // returns the script a certain character is in. Needed to separate the string
    // into runs of different scripts as required for X11 and opentype.
    QFont::Script scriptForChar(const QChar &c);

#ifdef Q_WS_X11
    QFont::Script hanHack( const QChar & c );
    static char **getXFontNames(const char *, int *);
    static bool fontExists(const QString &);
    static bool parseXFontName(const QCString &, char **);
    static bool fillFontDef(const QCString &, QFontDef *, QCString *);

    static inline bool isZero(char *x)
    {
	return (x[0] == '0' && x[1] == 0);

    }

    static inline bool isScalable( char **tokens )
    {
	return (isZero(tokens[PixelSize]) &&
		isZero(tokens[PointSize]) &&
		isZero(tokens[AverageWidth]));
    }

    static inline bool isSmoothlyScalable( char **tokens )
    {
	return (isZero(tokens[ResolutionX]) && isZero(tokens[ResolutionY]));
    }

    static inline bool isFixedPitch( char **tokens )
    {
	return (tokens[Spacing][0] == 'm' ||
		tokens[Spacing][0] == 'c' ||
		tokens[Spacing][0] == 'M' ||
		tokens[Spacing][0] == 'C');
    }

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

#ifndef QT_NO_XFTFREETYPE
    XftPattern *findXftFont(const QChar &, bool *) const;
    XftPattern *bestXftPattern(const QString &, const QString &) const;
#endif // QT_NO_XFTFREETYPE
    QCString findFont(QFont::Script, bool *) const;
    QCString findXftFont(QFont::Script, bool *) const;
    QCString bestFamilyMember(QFont::Script, const QString &, const QString &,
			      const QString &, int *) const;
    QCString bestMatch(const char *, int *, QFont::Script) const;
    int fontMatchScore(const char *, QCString &, float *, int *, bool *,
		       bool *, QFont::Script) const;
    void initFontInfo(QFont::Script);
    void load(QFont::Script = QFont::NoScript, bool = TRUE);
    bool loadUnicode(QFont::Script, const QChar &);
    void computeLineWidth();

    int textWidth( const QString &str, int pos, int len, TextRun *cache );
    void textExtents( const QString &str, int pos, int len, XCharStruct *overall );
    void drawText( Display *dpy, int screen, Qt::HANDLE hd, Qt::HANDLE rendhd,
		   GC gc, const QColor &pen, Qt::BGMode, const QColor &bgcolor,
		   int x, int y, const TextRun *cache );
    bool inFont( const QChar &ch );

    class QFontX11Data {
    public:
	// X fontstruct handles for each character set
	QFontStruct *fontstruct[QFont::LastPrivateScript];

	QFontX11Data()
	{
	    for (int i = 0; i < QFont::LastPrivateScript; i++) {
		fontstruct[i] = 0;
	    }
	}

	~QFontX11Data()
	{
	    QFontStruct *qfs;

	    for (int i = 0; i < QFont::LastPrivateScript; i++) {
		qfs = fontstruct[i];
		fontstruct[i] = 0;

		if (qfs && qfs != (QFontStruct *) -1) {
		    qfs->deref();
		}
	    }
	}
    } x11data;

    static QFont::Script defaultScript;

#endif // Q_WS_X11

#ifdef Q_WS_WIN
    ~QFontPrivate() { if( fin ) fin->deref(); }
    void load();
    void initFontInfo();
    HFONT create( bool *stockFont, HDC hdc = 0, bool compatMode = FALSE );
    QFontStruct *fin;
    HDC currHDC;

    void buildCache( HDC hdc, const QString &str, int pos, int len, TextRun *cache );
    void drawText( HDC hdc, int x, int y, TextRun *cache );
#endif // Q_WS_WIN

#ifdef Q_WS_QWS
    ~QFontPrivate();
    void load();
    QFontStruct *fin;
    int textWidth( const QString &str, int pos, int len, TextRun *cache );
    void drawText( QGfx *gfx, int x, int y, const TextRun *cache );
#endif

#if defined( Q_WS_MAC )
    ~QFontPrivate() { if( fin ) fin->deref(); }
    void macSetFont(QPaintDevice *);
    void drawText( int x, int y, QString s, int len );
    void load();
    QFontStruct *fin;
#endif

};


#endif // QFONTDATA_P_H
