#ifndef QTEXTLAYOUT_P_H
#define QTEXTLAYOUT_P_H

#include <qglobal.h>
#include <qstring.h>
#include <stdlib.h>
#include <qnamespace.h>

#include <assert.h>

class QFontEngineIface;
class QFont;
class QString;

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
    QScriptAnalysis analysis;
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

    const QScriptItem &operator[] (int i) const {
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
	return d->size;
    }
    void split( int pos );
    QScriptItemArray( const QScriptItemArray & ) {}
    QScriptItemArray &operator = ( const QScriptItemArray & ) { return *this; }

    void resize( int s );

    QScriptItemArrayPrivate *d;
};

struct Offset {
    short x;
    short y;
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

typedef unsigned short GlyphIndex;

class QShapedItemPrivate : public QShared
{
public:
    QShapedItemPrivate()
	: num_glyphs( 0 ), glyphs( 0 ), advances( 0 ), offsets( 0 ), logClusters( 0 ),
	  glyphAttributes( 0 ), fontEngine( 0 ),
	  from( 0 ), length( 0 ), ascent( 0 ), descent( 0 ),
	  isShaped( FALSE ), isPositioned( FALSE ) {}
    ~QShapedItemPrivate() {
	free( glyphs );
	free( offsets );
	free( advances );
	free( logClusters );
	free( glyphAttributes );
    }
    int num_glyphs;
    GlyphIndex * glyphs;
    Offset *advances;
    Offset *offsets;
    unsigned short *logClusters;
    GlyphAttributes *glyphAttributes;
    QFontEngineIface *fontEngine;
    QScriptAnalysis analysis;
    QString string;
    int from;
    int length;
    short ascent;
    short descent;
    bool isShaped : 1;
    bool isPositioned : 1;
};

class QShapedItem
{
public:
    QShapedItem();
    QShapedItem( const QShapedItem &other );

    ~QShapedItem();

    QShapedItem &operator =( const QShapedItem &other );

    const GlyphIndex *glyphs() const;
    int count() const;
    const Offset *offsets() const;
    const Offset *advances() const { return d->advances; }
    int ascent() const;
    int descent() const;

    QShapedItemPrivate *d;
};

struct QCharAttributes {
    int softBreak      :1;     // Potential linebreak point
    int whiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
    int charStop       :1;     // Valid cursor position (for left/right arrow)
    int wordStop       :1;     // Valid cursor position (for ctrl + left/right arrow)
    int reserved       :4;
};

struct QCharAttributesArrayPrivate {
    unsigned int alloc;
    unsigned int size;
    QCharAttributes attributes[1];
};

class QCharAttributesArray
{
public:
    QCharAttributesArray() : d( 0 ) {}
    ~QCharAttributesArray();

    QCharAttributesArray( const QCharAttributesArray & ) {}
    QCharAttributesArray & operator=( const QCharAttributesArray & ) { return *this; }

    const QCharAttributes &operator [] (int i) const {
	return d->attributes[i];
    }

    QCharAttributesArrayPrivate *d;
};

class QTextEngine
{
public:
    static const QTextEngine *instance();

    void itemize( QScriptItemArray &items, const QString & ) const;

    void attributes( QCharAttributesArray &attrs, const QString &string,
		     const QScriptItemArray &items, int item ) const;

    void bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder ) const;

    void shape( QShapedItem &shaped, const QFont &f, const QString &string,
			const QScriptItemArray &items, int item ) const;

    // ### we need something for justification

    enum Edge {
	Leading,
	Trailing
    };

    int cursorToX( QShapedItem &shaped, int cpos, Edge edge = Leading ) const;
    int xToCursor( QShapedItem &shaped, int x ) const;

    int width( QShapedItem &shaped ) const;
    int width( QShapedItem &shaped, int charFrom, int numChars ) const;
    bool split( QScriptItemArray &items, int item, QShapedItem &, QCharAttributesArray &,
		int width, QShapedItem *splitoff = 0 ) const;

//    static QScriptProperties scriptProperties( int script );

private:
    // not in the interface
    void shape( QShapedItem &shaped ) const;
    void position( QShapedItem &shaped ) const;
};

#endif
