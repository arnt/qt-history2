#ifndef QTEXTENGINE_P_H
#define QTEXTENGINE_P_H

#include <qglobal.h>
#include <qstring.h>
#include <stdlib.h>
#include <qnamespace.h>
#include <private/qfontdata_p.h>

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

typedef unsigned short glyph_t;

struct offset_t {
    short x;
    short y;
};

struct QShapedItem;
struct QCharAttributes;

struct QScriptAnalysis
{
    int script    : 7;
    int bidiLevel : 6;  // Unicode Bidi algorithm embedding level (0-61)
    int override  : 1;  // Set when in LRO/RLO embedding
    int reserved  : 2;
    bool operator == ( const QScriptAnalysis &other ) {
	return
	    script == other.script &&
	    bidiLevel == other.bidiLevel;
	// ###
// 	    &&
// 	    override == other.override;
    }

};

struct QScriptItem
{
    int position;
    int x;
    int y;
    int width;
    QScriptAnalysis analysis;
    short baselineAdjustment;
    short ascent;
    short descent;
    QFontEngine *fontEngine;
    QShapedItem *shaped;
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


// enum and struct are  made to be compatible with Uniscribe
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
    unsigned int justification   :4;  // Justification class
    unsigned int clusterStart    :1;  // First glyph of representation of cluster
    unsigned int mark            :1;  // needs to be positioned around base char
    unsigned int zeroWidth       :1;  // ZWJ, ZWNJ etc, with no width
    unsigned int reserved        :1;
    unsigned char combiningClass :8;
};

struct QShapedItem
{
    QShapedItem()
	: num_glyphs( 0 ), glyphs( 0 ), advances( 0 ), offsets( 0 ), logClusters( 0 ),
	  glyphAttributes( 0 ), ownGlyphs( TRUE ) {}
    ~QShapedItem();
    int num_glyphs;
    glyph_t *glyphs;
    offset_t *advances;
    offset_t *offsets;
    unsigned short *logClusters;
    GlyphAttributes *glyphAttributes;
    bool ownGlyphs : 1;
};

struct QCharAttributes {
    uint softBreak      :1;     // Potential linebreak point
    uint whiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
    uint charStop       :1;     // Valid cursor position (for left/right arrow)
    uint wordStop       :1;     // Valid cursor position (for ctrl + left/right arrow)
    uint invalid        :1;
    uint reserved       :3;
};

class QTextEngine;
class QFontEngine;
class QFontPrivate;

struct QTextEngine {
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
