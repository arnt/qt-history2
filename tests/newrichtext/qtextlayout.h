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


#if 0
class QTextItemPrivate;

class QTextItem
{
public:
    int width() const;
    int ascent() const;
    int descent() const;
    int baselineAdjustment() const;

    enum Edge {
	Leading,
	Trailing
    };
    /* cPos gets set to the valid position */
    int cursorToX( int *cPos, Edge edge = Leading );
    int xToCursor( int x );

    bool isRightToLeft() const;
    bool isObject() const;

    void setWidth( int w );
    void setAscent( int a );
    void setDescent( int d );
    void setBaselineAdjustment( int adjust );

private:
    int item;
    QTextLayout *layout;
}

// to CPP file
struct QTextItemData {
    int y;
    int x;
    short baselineAdjustment;
    short width;
    short ascent;
    short descent;
    ScriptAnalysis analysis;
    int from;
};

class QTextLayoutPrivate;
class QPaintDevice;

class QTextLayout
{
public:
    // does itemization
    QTextLayout( const QString &string, QPainter * = 0 );

    enum LineBreakStrategy {
	AtWordBoundaries,
	AtCharBoundaries
    };

    /* add an additional item boundary eg. for style change */
    void set( int strPos );

    bool validCursorPosition( int strPos );

    int numItems() const;
    QTextItem *at( int i );

    void beginLayout();
    void beginLine( int width );

    QTextItem *nextItem();
    /* ## maybe also currentItem() */
    void setLineWidth( int newWidth );
    int availableWidth() const;

    /* returns true if completely added */
    bool addCurrentItem();

    void endLine( int x, int y, Qt::AlignmentFlags alignment );
    void endLayout();

private:
    /* disable copy and assignment */
    QTextLayout( const QTextLayout & ) {}
    void operator = ( const QTextLayout & ) {}
};


/*
  class QPainter {
      .....
      void drawTextItem( int x, int y, QTextItem *item );
  };
*/
#endif

#endif
