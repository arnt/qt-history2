/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTENGINE_P_H
#define QTEXTENGINE_P_H

#ifndef QT_H
#include "qglobal.h"
#include "qstring.h"
#include "qnamespace.h"
#include "private/qtextlayout_p.h"
#include <private/qfontdata_p.h>
#include <qvector.h>
#include <qpaintengine.h>
#endif // QT_H

#include <stdlib.h>
#ifndef Q_OS_TEMP
#include <assert.h>
#endif // Q_OS_TEMP

class QFontPrivate;
class QString;

class QOpenType;
class QPainter;

class QTextInlineObjectInterface;

// this uses the same coordinate system as Qt, but a different one to freetype and Xft.
// * y is usually negative, and is equal to the ascent.
// * negative yoff means the following stuff is drawn higher up.
// the characters bounding rect is given by QRect( x,y,width,height), it's advance by
// xoo and yoff
struct glyph_metrics_t
{
    inline glyph_metrics_t() {
	x = 100000;
	y = 100000;
	width = 0;
	height = 0;
	xoff = 0;
	yoff = 0;
    }
    inline glyph_metrics_t( int _x, int _y, int _width, int _height, int _xoff, int _yoff ) {
	x = _x;
	y = _y;
	width = _width;
	height = _height;
	xoff = _xoff;
	yoff = _yoff;
    }
    int x;
    int y;
    int width;
    int height;
    int xoff;
    int yoff;
};
Q_DECLARE_TYPEINFO(glyph_metrics_t, Q_PRIMITIVE_TYPE);

typedef unsigned short glyph_t;

#if defined( Q_WS_X11 ) || defined ( Q_WS_QWS ) || defined (Q_WS_MAC)


struct QScriptAnalysis
{
    unsigned short script    : 7;
    unsigned short override  : 1;  // Set when in LRO/RLO embedding
    unsigned short bidiLevel : 6;  // Unicode Bidi algorithm embedding level (0-61)
    unsigned short reserved  : 2;
    bool operator == ( const QScriptAnalysis &other ) {
	return
	    script == other.script &&
	    bidiLevel == other.bidiLevel;
	// ###
// 	    && override == other.override;
    }

};
Q_DECLARE_TYPEINFO(QScriptAnalysis, Q_PRIMITIVE_TYPE);

#elif defined( Q_WS_WIN )

// do not change the definitions below unless you know what you are doing!
// it is designed to be compatible with the types found in uniscribe.

struct qoffset_t {
    int x;
    int y;
};
Q_DECLARE_TYPEINFO(qoffset_t, Q_PRIMITIVE_TYPE);

struct QScriptAnalysis {
    unsigned short script         :10;
    unsigned short rtl            :1;
    unsigned short layoutRTL      :1;
    unsigned short linkBefore     :1;
    unsigned short linkAfter      :1;
    unsigned short logicalOrder   :1;
    unsigned short noGlyphIndex   :1;
    unsigned short bidiLevel         :5;
    unsigned short override          :1;
    unsigned short inhibitSymSwap    :1;
    unsigned short charShape         :1;
    unsigned short digitSubstitute   :1;
    unsigned short inhibitLigate     :1;
    unsigned short fDisplayZWG        :1;
    unsigned short arabicNumContext  :1;
    unsigned short gcpClusters       :1;
    unsigned short reserved          :1;
    unsigned short engineReserved    :2;
};
Q_DECLARE_TYPEINFO(QScriptAnalysis, Q_PRIMITIVE_TYPE);

inline bool operator == ( const QScriptAnalysis &sa1, const QScriptAnalysis &sa2 )
{
    return
	sa1.script == sa2.script &&
	sa1.bidiLevel == sa2.bidiLevel;
	// ###
// 	    && override == other.override;
}

#endif

struct QGlyphLayout
{
    // highest value means highest priority for justification. Justification is done by first inserting kashidas
    // starting with the highest priority positions, then stretching spaces, afterwards extending inter char
    // spacing, and last spacing between arabic words.
    // NoJustification is for example set for arabic where no Kashida can be inserted or for diacritics.
    enum Justification {
	NoJustification= 0,   // Justification can't be applied after this glyph
	Arabic_Space   = 1,   // This glyph represents a space inside arabic text
	Character      = 2,   // Inter-character justification point follows this glyph
	Space          = 4,   // This glyph represents a blank outside an Arabic run
	Arabic_Normal  = 7,   // Normal Middle-Of-Word glyph that connects to the right (begin)
	Arabic_Waw     = 8,    // Next character is final form of Waw/Ain/Qaf/Fa
	Arabic_BaRa    = 9,   // Next two chars are Ba + Ra/Ya/AlefMaksura
	Arabic_Alef    = 10,  // Next character is final form of Alef/Tah/Lam/Kaf/Gaf
	Arabic_HaaDal  = 11,  // Next character is final form of Haa/Dal/Taa Marbutah
	Arabic_Seen    = 12,  // Initial or Medial form Of Seen/Sad
	Arabic_Kashida = 13   // Kashida(U+640) in middle of word
    };

    unsigned short glyph;
    struct Attributes {
	unsigned short justification   :4;  // Justification class
	unsigned short clusterStart    :1;  // First glyph of representation of cluster
	unsigned short mark            :1;  // needs to be positioned around base char
	unsigned short zeroWidth       :1;  // ZWJ, ZWNJ etc, with no width
	unsigned short dontPrint       :1;
	unsigned short combiningClass  :8;
    };
    Attributes attributes;
    struct {short x; short y; } advance;
    struct { short x; short y; } offset;
};
Q_DECLARE_TYPEINFO(QGlyphLayout, Q_PRIMITIVE_TYPE);

// also this is compatible to uniscribe. Do not change.
struct QCharAttributes {
    uchar softBreak      :1;     // Potential linebreak point _before_ this character
    uchar whiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
    uchar charStop       :1;     // Valid cursor position (for left/right arrow)
    uchar wordStop       :1;     // Valid cursor position (for ctrl + left/right arrow)
    uchar invalid        :1;
    uchar reserved       :3;
};
Q_DECLARE_TYPEINFO(QCharAttributes, Q_PRIMITIVE_TYPE);

class QFontEngine;

struct QScriptItem
{
    inline QScriptItem() : position( 0 ), isSpace( FALSE ), isTab( FALSE ),
			   isObject( FALSE ), hasPositioning( FALSE ),
			   descent( -1 ), ascent( -1 ), width( -1 ),
			   num_glyphs( 0 ), glyph_data_offset( 0 ),
			   format(-1) { }

    int position;
    QScriptAnalysis analysis;
    unsigned short isSpace  : 1;
    unsigned short isTab    : 1;
    unsigned short isObject : 1;
    unsigned short hasPositioning : 1;
    unsigned short reserved : 12;
    short descent;
    int ascent;
    int width;
    int num_glyphs;
    int glyph_data_offset;
    int format;
};


Q_DECLARE_TYPEINFO(QScriptItem, Q_MOVABLE_TYPE);

typedef QVector<QScriptItem> QScriptItemArray;

struct QScriptLine
{
    short descent;
    short ascent;
    short x;
    int width;
    int y;
    int from;
    int length;
    int textWidth;
};
Q_DECLARE_TYPEINFO(QScriptLine, Q_PRIMITIVE_TYPE);

typedef QVector<QScriptLine> QScriptLineArray;


class QFontPrivate;
class QTextFormatCollection;
class QPalette;

class QTextEngine {
public:
    QTextEngine()
	: fnt(0), formats(0), inlineObjectIface(0), allocated(0), memory(0),
	  cursorPos(-1), selections(0), nSelections(0), underlinePositions(0)
	{}
    QTextEngine(const QString &str, QFontPrivate *f )
	: fnt(f), formats(0), inlineObjectIface(0), allocated(0), memory(0),
    	  cursorPos(-1), selections(0), nSelections(0), underlinePositions(0)
	{ setText(str); fnt->ref(); }
    ~QTextEngine();

    void setText(const QString &str);
    void setFormatCollection(const QTextFormatCollection *fmts) { formats = fmts; }
    void setInlineObjectInterface(QTextInlineObjectInterface *iface) { inlineObjectIface = iface; }

    enum Mode {
	Full = 0x00,
	NoBidi = 0x01,
	SingleLine = 0x02,
	WidthOnly = 0x07
    };

    void itemize( int mode = Full );

    static void bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder );

    const QCharAttributes *attributes();

    void setFormat(int from, int length, int format);
    void setBoundary(int strPos);

    void shape( int item ) const;

    // ### we need something for justification

    enum Edge {
	Leading,
	Trailing
    };

    int width( int charFrom, int numChars ) const;
    glyph_metrics_t boundingBox( int from,  int len ) const;

    int length( int item ) const {
	const QScriptItem &si = items[item];
	int from = si.position;
	item++;
	return ( item < items.size() ? items[item].position : string.length() ) - from;
    }

    QFontEngine *fontEngine(const QScriptItem &si) const;
    QFont font(const QScriptItem &si) const;
    QFont font() const { if (fnt) return QFont(fnt, 0); return QFont(); }

    void splitItem( int item, int pos );

    unsigned short *logClustersPtr;
    QGlyphLayout *glyphPtr;

    inline unsigned short *logClusters( const QScriptItem *si ) const
	{ return logClustersPtr+si->position; }
    inline QGlyphLayout *glyphs(const QScriptItem *si) const
	{ return glyphPtr + si->glyph_data_offset; }

    void reallocate( int totalGlyphs );
    inline void ensureSpace( int nGlyphs ) const {
	if ( num_glyphs - used < nGlyphs )
	    ((QTextEngine *)this)->reallocate( ( (used + nGlyphs + 16) >> 4 ) << 4 );
    }


    int findItem(int strPos) const;

    mutable QScriptItemArray items;
    mutable QScriptLineArray lines;

    QString string;
    QFontPrivate *fnt;
    const QTextFormatCollection *formats;
    QChar::Direction direction : 5;
    unsigned int haveCharAttributes : 1;
    unsigned int widthOnly : 1;
    unsigned int reserved : 24;
    unsigned int textFlags;
    QPalette *pal;
    QTextInlineObjectInterface *inlineObjectIface;

    int allocated;
    void **memory;
    int num_glyphs;
    int used;
    int minWidth;

    int cursorPos;
    const QTextLayout::Selection *selections;
    int nSelections;
    int *underlinePositions;
private:
    void shapeText( int item ) const;
};

#endif
