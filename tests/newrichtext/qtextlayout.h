/*

This needs to get an abstract interface that offers everything we need
to do complex script procesing. Should be similar to, but simpler than
Uniscribe.

It is defined as an abstract interface, so that we can load an engine
at runtime. If we find uniscribe, use it otherwise use our own engine
(that in this case might not support indic).

It should have a set of methods that are fine grained enough to do rich
text processing and a set of simpler methods for plain text.

Some of the ideas are stolen from the Uniscribe API or from Pango.

*/

#ifndef QTEXTLAYOUT_H
#define QTEXTLAYOUT_H

#include <qglobal.h>
#include <qstring.h>
#include <stdlib.h>

#include <assert.h>

class FontEngineIface;
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


struct ScriptAnalysis
{
    int script : 10;
    int bidiLevel : 6;  // Unicode Bidi algorithm embedding level (0-61)
    int override :1;  // Set when in LRO/RLO embedding
    int linkBefore : 1;
    int linkAfter : 1;
    int reserved : 13;
    bool operator == ( const ScriptAnalysis &other ) {
	return
	    script == other.script &&
	    bidiLevel == other.bidiLevel;
	// ###
// 	    &&
// 	    override == other.override;
    }

};

struct ScriptItem
{
    int position;
    ScriptAnalysis analysis;
};

struct ScriptItemArrayPrivate
{
    unsigned int alloc;
    unsigned int size;
    ScriptItem items[1];
};

class ScriptItemArray
{
public:
    ScriptItemArray() : d( 0 ) {}
    ~ScriptItemArray();

    const ScriptItem &operator[] (int i) const {
	return d->items[i];
    }
    void append( const ScriptItem &item ) {
	if ( d->size && d->items[d->size-1].analysis == item.analysis ) {
	    //    qDebug("ScriptItemArray::append: trying to add same item" );
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
    ScriptItemArray( const ScriptItemArray & ) {}
    ScriptItemArray &operator = ( const ScriptItemArray & ) { return *this; }

    void resize( int s );

    ScriptItemArrayPrivate *d;
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

class ShapedItemPrivate : public QShared
{
public:
    ShapedItemPrivate()
	: num_glyphs( 0 ), glyphs( 0 ), advances( 0 ), offsets( 0 ), logClusters( 0 ),
	  glyphAttributes( 0 ), fontEngine( 0 ),
	  from( 0 ), length( 0 ), ascent( 0 ), descent( 0 ),
	  isShaped( FALSE ), isPositioned( FALSE ) {}
    ~ShapedItemPrivate() {
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
    FontEngineIface *fontEngine;
    ScriptAnalysis analysis;
    QString string;
    int from;
    int length;
    short ascent;
    short descent;
    bool isShaped : 1;
    bool isPositioned : 1;
};

class ShapedItem
{
public:
    ShapedItem();
    ShapedItem( const ShapedItem &other );

    ~ShapedItem();

    ShapedItem &operator =( const ShapedItem &other );

    const GlyphIndex *glyphs() const;
    int count() const;
    const Offset *offsets() const;
    const Offset *advances() const { return d->advances; }
    int ascent() const;
    int descent() const;

    ShapedItemPrivate *d;
};

struct CharAttributes {
    int softBreak      :1;     // Potential linebreak point
    int whiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
    int charStop       :1;     // Valid cursor position (for left/right arrow)
    int wordStop       :1;     // Valid cursor position (for ctrl + left/right arrow)
    int reserved       :4;
};

struct CharAttributesArrayPrivate {
    unsigned int alloc;
    unsigned int size;
    CharAttributes attributes[1];
};

class CharAttributesArray
{
public:
    CharAttributesArray() : d( 0 ) {}
    ~CharAttributesArray();

    CharAttributesArray( const CharAttributesArray & ) {}
    CharAttributesArray & operator=( const CharAttributesArray & ) { return *this; }

    const CharAttributes &operator [] (int i) const {
	return d->attributes[i];
    }

    CharAttributesArrayPrivate *d;
};

class TextLayout
{
public:
    static const TextLayout *instance();

    virtual void itemize( ScriptItemArray &items, const QString & ) const = 0;

    virtual void attributes( CharAttributesArray &attrs, const QString &string,
		     const ScriptItemArray &items, int item ) const = 0;

    void bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder ) const;

    virtual void shape( ShapedItem &shaped, const QFont &f, const QString &string,
			const ScriptItemArray &items, int item ) const = 0;

    // ### we need something for justification

    enum Edge {
	Leading,
	Trailing
    };

    virtual int cursorToX( ShapedItem &shaped, int cpos, Edge edge = Leading ) const = 0;
    virtual int xToCursor( ShapedItem &shaped, int x ) const = 0;

    virtual int width( ShapedItem &shaped ) const = 0;
    virtual int width( ShapedItem &shaped, int charFrom, int numChars ) const = 0;
    virtual bool split( ScriptItemArray &items, int item, ShapedItem &, CharAttributesArray &,
			int width, ShapedItem *splitoff = 0 ) const = 0;

//    static ScriptProperties scriptProperties( int script );

};



#endif
