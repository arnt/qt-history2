/*

This needs to get an abstract interface that offers everything we need
to do complex script procesing. Should be similar to, but simpler than
Uniscribe.

It is defined as an abstract interface, so that we can load an engine
at runtime. If we find uniscribe, use it otherwise use our own engine
(that in this case might not support indic).

It should have a set of methods that are fine grained enough to do rich
text processing and a set of simpler methods for plain text.

Some of the ideas are stolen from the Uniscibe API or from Pango.

*/
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
    unsigned int refCount;
    unsigned int alloc;
    unsigned int size;
    ScriptItem items[1];
};

class ScriptItemArray
{
public:
    ScriptItemArray() : d( 0 ) {}
    ~ScriptItemArray();

    void itemize( const QRTString & );
    void itemize( const QString & );

    const ScriptItem &operator[] (int i) {
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
private:
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

typedef QMemArray<CharAttributes> CharAttributesArray;

class QTextLayout {
public:

    // ScriptShape && ScriptPlace
    static ShapedItem shape( const QRTString &string, const ScriptItemArray &items, int item );

    // ScriptBreak
    static CharAttributesArray lineBreaks( const QRTString &string, const ScriptItemArray &items, int item, const ShapedItem &shaped );

    // corresponds to ScriptLayout in Uniscribe
    static void bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder, int *visualPositions );


    // ### we need something for justification

    // ### cursor handling
    static int cursorToX();
    static int xToCursor();

//    static ScriptProperties scriptProperties( int script );

};
