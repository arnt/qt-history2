/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Definition of font rendering infrastructure for Embedded Qt
**
** Created : 940721
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QFONTMANAGER_QWS_H
#define QFONTMANAGER_QWS_H

#ifndef QT_H
#include "qfont.h"
#include "qlist.h"
#endif // QT_H

#include <stdlib.h>


// These are stored in the shared memory segment in front of their
// data, and indexed at the start of the segment

// This needs to be a multiple of 64 bits

class QGlyphMetrics {

public:
    Q_UINT8 linestep;
    Q_UINT8 width;
    Q_UINT8 height;
    Q_UINT8 padding;

    Q_INT8 bearingx;      // Difference from pen position to glyph's left bbox
    Q_UINT8 advance;       // Difference between pen positions
    Q_INT8 bearingy;      // Used for putting characters on baseline

};

class QGlyph {
public:
    QGlyph() { }
    QGlyph(QGlyphMetrics* m, uchar* d) :
	metrics(m), data(d) { }

    QGlyphMetrics* metrics;
    uchar* data;
};



class QFontFactory;
class QDiskFont;

// This is a particular font instance at a particular resolution
// e.g. Truetype Times, 10 point. There's only one of these though;
// we want to share generated glyphs

class QRenderedFont {

public:

    // Normal font-type is monochrome; glyph data is a
    //   bitmap, which doesn't use much memory

    // Initialise for name A, renderer B, font type C, D glyphs

    QRenderedFont(QDiskFont *,const QFontDef&);
    virtual ~QRenderedFont();

    int refcount;

    int ptsize;

    bool italic;
    unsigned int weight;

    void ref() { refcount++; }
    bool deref() { refcount--; return (refcount<1); }

    QDiskFont* diskfont;
    int fascent,fdescent;
    int fleftbearing,frightbearing;
    int fmaxwidth;
    bool smooth;
    int maxchar;

    int ascent() { return fascent; }
    int descent() { return fdescent; }
    int width(int);
    int width( const QString&, int =-1 );
    int leftBearing(int);
    int rightBearing(int);

    // Calling any of these can trigger a full-font metrics check
    // which can be expensive
    int minLeftBearing();
    int minRightBearing();
    int maxWidth();

    void save(const QString& filename);

    virtual bool inFont(QChar ch) const=0;
    virtual QGlyph render(QChar)=0;

private:

};

// Keeps track of available renderers and which font is which

class QDiskFontPrivate {};

class QDiskFont {

public:
    QDiskFont(QFontFactory *f, const QString& n, bool i, int w, int s,
	      const QString &fl, const QString& fi) :
	factory(f), name(n), italic(i), weight(w), size(s), flags(fl), file(fi)
    {
	loaded=false;
	p=0;
    }

    QFontFactory *factory;
    QString name;
    bool italic;
    int weight;
    int size;
    QString flags;
    QString file;
    bool loaded;

    QDiskFontPrivate * p;
};

class QFontManager {

public:

    QList<QFontFactory> factories;
    QList<QRenderedFont> cachedfonts;
    QList<QDiskFont> diskfonts;

    QFontManager();
    ~QFontManager();

    // Font definition, type and colour

    QRenderedFont * get(const QFontDef &);

    static void initialize();
    static void cleanup();

};

class QFontFactory {

public:

    QFontFactory() {}
    virtual ~QFontFactory() {}

    virtual QRenderedFont * get(const QFontDef &,QDiskFont *)=0;
    virtual void load(QDiskFont *) const=0;
    virtual QString name()=0;
};

void qt_init_fonts();

extern QFontManager * qt_fontmanager;

#endif




