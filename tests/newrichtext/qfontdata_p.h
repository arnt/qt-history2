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
#include "qfont.h"
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

#include <qt_x11.h>

class QCharStruct;

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


class QFontEngine;

class QFontX11Data  // used as a QFontPrivate member
{
public:
    // X fontstruct handles for each character set
    QFontEngine *fontstruct[QFont::LastPrivateScript];

    QFontX11Data();
    ~QFontX11Data();
};




typedef QCacheIterator<QFontEngine> QFontCacheIterator;
class QFontCache : public QObject, public QCache<QFontEngine>
{
public:
    QFontCache();
    ~QFontCache();

    bool insert(const QString &, const QFontEngine *, int c);
    void deleteItem(Item d);
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

#ifdef Q_WS_X11
    QFontEngine *engineForScript( QFont::Script script ) const;

    static char **getXFontNames(const char *, int *);
    static bool fontExists(const QString &);
    static bool parseXFontName(char *, char **);
    static QCString fixXLFD( const QCString & );
    static bool fillFontDef(XFontStruct *, QFontDef *, int);
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

};

inline QFontPrivate::QFontPrivate()
    : QShared(), exactMatch(FALSE), lineWidth(1)
{

    x11Screen = QPaintDevice::x11AppScreen();
    paintdevice = 0;
}

inline QFontPrivate::QFontPrivate(const QFontPrivate &fp)
    : QShared(), request(fp.request), actual(fp.actual),
exactMatch(fp.exactMatch), lineWidth(1)
{
    Q_ASSERT(!fp.paintdevice);
    x11Screen = fp.x11Screen;
    paintdevice = 0;
}

inline QFontPrivate::QFontPrivate( const QFontPrivate &fp, QPaintDevice *pd )
    : QShared(), request(fp.request), actual(fp.actual),
exactMatch(fp.exactMatch), lineWidth(1)
{

    x11Screen = pd->x11Screen();
    paintdevice = pd;
}

inline QFontPrivate::~QFontPrivate()
{
}

#endif // QFONTDATA_P_H
