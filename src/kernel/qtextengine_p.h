#ifndef QTEXTENGINE_P_H
#define QTEXTENGINE_P_H

#include <qglobal.h>
#include <qstring.h>
#include <qnamespace.h>
#include <private/qfontdata_p.h>

#include <stdlib.h>
#include <assert.h>


class QFontPrivate;
class QString;

class QOpenType;
class QPainter;

// this uses the same coordinate system as Qt, but a different one to freetype and Xft.
// * y is usually negative, and is equal to the ascent.
// * negative yoff means the following stuff is drawn higher up.
// the characters bounding rect is given by QRect( x,y,width,height), it's advance by
// xoo and yoff
struct QGlyphMetrics
{
    QGlyphMetrics() {
	x = 100000;
	y = 100000;
	width = 0;
	height = 0;
	xoff = 0;
	yoff = 0;
    }
    QGlyphMetrics( int _x, int _y, int _width, int _height, int _xoff, int _yoff ) {
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

#if defined( Q_WS_X11 )
typedef unsigned short glyph_t;

struct offset_t {
    short x;
    short y;
};

typedef int advance_t;

struct QScriptAnalysis
{
    unsigned short script    : 7;
    unsigned short bidiLevel : 6;  // Unicode Bidi algorithm embedding level (0-61)
    unsigned short override  : 1;  // Set when in LRO/RLO embedding
    unsigned short reserved  : 2;
    bool operator == ( const QScriptAnalysis &other ) {
	return
	    script == other.script &&
	    bidiLevel == other.bidiLevel;
	// ###
// 	    && override == other.override;
    }

};

#elif defined( Q_WS_MAC )

typedef unsigned short glyph_t;

struct offset_t {
    short x;
    short y;
};

typedef int advance_t;

struct QScriptAnalysis
{
    unsigned short script    : 7;
    unsigned short bidiLevel : 6;  // Unicode Bidi algorithm embedding level (0-61)
    unsigned short override  : 1;  // Set when in LRO/RLO embedding
    unsigned short reserved  : 2;
    bool operator == ( const QScriptAnalysis &other ) {
	return
	    script == other.script &&
	    bidiLevel == other.bidiLevel;
	// ###
// 	    && override == other.override;
    }

};

#elif defined( Q_WS_WIN )

// do not change the definitions below unless you know what you are doing!
// it is designed to be compatible with the types found in uniscribe.

typedef unsigned short glyph_t;

struct offset_t {
    int x;
    int y;
};

typedef int advance_t;

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
    bool operator == ( const QScriptAnalysis &other ) {
	return
	    script == other.script &&
	    bidiLevel == other.bidiLevel;
	// ###
// 	    && override == other.override;
    }
};


#endif

// enum and struct are  made to be compatible with Uniscribe, dont change unless you know what you're doing.
struct GlyphAttributes {
    // highest value means highest priority for justification. Justification is done by first inserting kashidas
    // starting with the highest priority positions, then stretching spaces, afterwards extending inter char
    // spacing, and last spacing between arabic words.
    // NoJustification is for example set for arabic where no Kashida can be inserted or for diacritics.
    enum Justification {
	NoJustification= 0,   // Justification can't be applied at this glyph
	Arabic_Space   = 1,   // This glyph represents a space in an Arabic item
	Character      = 2,   // Inter-character justification point follows this glyph
	Space          = 4,   // This glyph represents a blank outside an Arabic run
	Arabic_Normal  = 7,   // Normal Middle-Of-Word glyph that connects to the right (begin)
	Arabic_Kashida = 8,   // Kashida(U+640) in middle of word
	Arabic_Alef    = 9,   // Final form of Alef-like (U+627, U+625, U+623, U+632)
	Arabic_Ha      = 10,  // Final Form Of Ha (U+647)
	Arabic_Ra      = 11,  // Final Form Of Ra (U+631)
	Arabic_Ba      = 12,  // Middle-Of-Word Form Of Ba (U+628)
	Arabic_Bara    = 13,  // Ligature Of Alike (U+628,U+631)
	Arabic_Seen    = 14   // Highest Priority: Initial Shape Of Seen(U+633) (End)
    };
    unsigned short justification   :4;  // Justification class
    unsigned short clusterStart    :1;  // First glyph of representation of cluster
    unsigned short mark            :1;  // needs to be positioned around base char
    unsigned short zeroWidth       :1;  // ZWJ, ZWNJ etc, with no width
    unsigned short reserved        :1;
    unsigned short combiningClass  :8;
};

// also this is compatible to uniscribe. Do not change.
struct QCharAttributes {
    uchar softBreak      :1;     // Potential linebreak point
    uchar whiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
    uchar charStop       :1;     // Valid cursor position (for left/right arrow)
    uchar wordStop       :1;     // Valid cursor position (for ctrl + left/right arrow)
    uchar invalid        :1;
    uchar reserved       :3;
};

struct QShapedItem
{
    QShapedItem()
	: num_glyphs( 0 ), glyphs( 0 ), advances( 0 ), offsets( 0 ), logClusters( 0 ),
	  glyphAttributes( 0 ), ownGlyphs( TRUE ) {}
    ~QShapedItem();
    int num_glyphs;
    glyph_t *glyphs;
    advance_t *advances;
    offset_t *offsets;
    unsigned short *logClusters;
    GlyphAttributes *glyphAttributes;
    bool ownGlyphs : 1;
};

class QFontEngine;

struct QScriptItem
{
    int position;
    QScriptAnalysis analysis;
    short baselineAdjustment;
    short ascent;
    short descent;
    int x;
    int y;
    int width;
    QShapedItem *shaped;
    QFontEngine *fontEngine;
};

struct QScriptItemArrayPrivate
{
    unsigned int alloc;
    unsigned int size;
    QScriptItem items[1];
};

class QScriptItemArray
{
public:
    QScriptItemArray() : d( 0 ) {}
    ~QScriptItemArray();

    QScriptItem &operator[] (int i) const {
	return d->items[i];
    }
    void append( const QScriptItem &item ) {
	if ( d->size && d->items[d->size-1].analysis == item.analysis ) {
	    //    qDebug("QScriptItemArray::append: trying to add same item" );
	    return;
	}
	if ( d->size == d->alloc )
	    resize( d->size + 1 );
	d->items[d->size] = item;
	d->size++;
    }
    int size() const {
	return d ? d->size : 0;
    }
    void split( int item, int pos );
    QScriptItemArray( const QScriptItemArray & ) {}
    QScriptItemArray &operator = ( const QScriptItemArray & ) { return *this; }

    void resize( int s );
    void clear();

    QScriptItemArrayPrivate *d;
};

class QFontPrivate;

class QTextEngine {
public:
    QTextEngine( const QString &str, QFontPrivate *f );
    ~QTextEngine();

    void itemize( bool doBidi = TRUE );

    static void bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder );

    void setFont( int item, QFontPrivate *f );
    QFontEngine *font( int item );

    const QCharAttributes *attributes();
    QShapedItem *shape( int item ) const;

    // ### we need something for justification

    enum Edge {
	Leading,
	Trailing
    };

    int width( int charFrom, int numChars ) const;
    QGlyphMetrics boundingBox( int from,  int len ) const;

    QScriptItemArray items;
    QString string;
    QFontPrivate *fnt;
    int lineWidth;
    int widthUsed;
    int firstItemInLine;
    int currentItem;
    QChar::Direction direction;
    QCharAttributes *charAttributes;

    int length( int item ) const {
	const QScriptItem &si = items[item];
	int from = si.position;
	item++;
	return ( item < items.size() ? items[item].position : string.length() ) - from;
    }
};

#endif
