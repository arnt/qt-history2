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

#include "qrtstring.h"
#include <qmemarray.h>

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
	    bidiLevel == other.bidiLevel &&
	    override == other.override;
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
	if ( d->items[d->size-1].analysis == item.analysis ) {
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
    ScriptItemArray( const ScriptItemArray & ) {}
    ScriptItemArray &operator = ( const ScriptItemArray & ) { return *this; }

    void resize( int s );

    ScriptItemArrayPrivate *d;
};

class ShapedItemPrivate;

class ShapedItem
{
public:
    ShapedItem();
    ~ShapedItem();

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

    CharAttributesArrayPrivate *d;
};

class TextLayout
{
public:

    virtual void itemize( ScriptItemArray &items, const QRTString & ) const = 0;
    virtual void itemize( ScriptItemArray &items, const QString & ) const = 0;

    virtual void attributes( CharAttributesArray &attrs, const QString &string,
		     const ScriptItemArray &items, int item ) const = 0;
    void attributes( CharAttributesArray &attrs, const QRTString &string,
		     const ScriptItemArray &items, int item ) const {
	attributes( attrs, string.str(), items, item );
    }


    static const TextLayout *instance();

    // corresponds to ScriptLayout in Uniscribe
    void bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder ) const;

    // ScriptShape && ScriptPlace
    virtual void shape( ShapedItem &shaped, const QRTString &string,
			const ScriptItemArray &items, int item ) const = 0;

#if 0


    // ### we need something for justification

    // ### cursor handling
    int cursorToX() {}
    int xToCursor() {}

//    static ScriptProperties scriptProperties( int script );
#endif

};



#endif
