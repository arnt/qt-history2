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

#ifndef QTEXTLAYOUT_P_H
#define QTEXTLAYOUT_P_H

#ifndef QT_H
#include "qstring.h"
#include "qnamespace.h"
#include "qrect.h"
#endif // QT_H

class QTextEngine;
class QFont;

class Q_GUI_EXPORT QTextItem
{
public:
    QTextItem( int i, QTextEngine *e ) : itm( i ), eng( e ) {}
    inline QTextItem() : itm(0), eng(0) {}
    inline bool isValid() const { return (bool)eng; }

    QRect rect() const;
    int x() const;
    int y() const;
    int width() const;
    int ascent() const;
    int descent() const;

    enum Edge {
	Leading,
	Trailing
    };
    enum CursorPosition {
	BetweenCharacters,
	OnCharacters
    };

    /* cPos gets set to the valid position */
    int cursorToX( int *cPos, Edge edge = Leading ) const;
    inline int cursorToX( int cPos, Edge edge = Leading ) const { return cursorToX( &cPos, edge ); }
    int xToCursor( int x, CursorPosition = BetweenCharacters ) const;

    bool isRightToLeft() const;
    bool isObject() const;
    bool isSpace() const;
    bool isTab() const;

    void setWidth( int w );
    void setAscent( int a );
    void setDescent( int d );

    int from() const;
    int length() const;

    QTextEngine *engine() const { return eng; }
    int item() const { return itm; }

    int custom() const;

private:
    friend class QTextLayout;
    friend class QPainter;
    friend class QPSPrinter;
    int itm;
    QTextEngine *eng;
};

#if 0
class QTextLine
{
public:
    QTextLine( int line, QTextEngine *e ) : i( line ), eng( e ) {}
    inline QTextLine() : i(0), eng(0) {}
    inline bool isValid() const { return (bool)eng; }

    QRect rect() const;
    int x1() const;
    int x2() const;
    int y() const;
    int ascent() const;
    int descent() const;
    int height() const;

#if 0
    enum Edge {
	Leading,
	Trailing
    };
    enum CursorPosition {
	BetweenCharacters,
	OnCharacters
    };

    /* cPos gets set to the valid position */
    int cursorToX( int *cPos, Edge edge = Leading ) const;
    inline int cursorToX( int cPos, Edge edge = Leading ) const { return cursorToX( &cPos, edge ); }
    int xToCursor( int x, CursorPosition = BetweenCharacters ) const;
#endif

    void adjust(int y, int x1, int x2);

    void setWidth( int w );
    void setAscent( int a );
    void setDescent( int d );

    int from() const;
    int length() const;

    QTextEngine *engine() const { return eng; }
    int line() const { return i; }

private:
    friend class QTextLayout;
    int i;
    QTextEngine *eng;
};
#endif

class QPainter;

class Q_GUI_EXPORT QTextLayout
{
public:
    // does itemization
    QTextLayout();
    QTextLayout(const QString& string);
    QTextLayout( const QString& string, QPainter *);
    QTextLayout( const QString& string, const QFont& fnt );
    ~QTextLayout();

    void setText( const QString& string, const QFont& fnt );
    void setText( const QString& string);

    void enableKerning(bool enable);

    enum LineBreakStrategy {
	AtWordBoundaries,
	AtCharBoundaries
    };

    /* add an additional item boundary eg. for style change */
    void setBoundary( int strPos );

    void setProperty(int from, int length, const QFont &f, int custom = 0);
    inline void setFont(int from, int length, const QFont &f)
	{ setProperty(from, length, f); }

    int numItems() const;
    QTextItem itemAt( int i ) const;
    QTextItem findItem( int strPos ) const;

    enum LayoutMode {
	NoBidi,
	SingleLine,
	MultiLine
    };
#if 0
    QTextLine createLine(int from, int y, int x1, int x2);
#else
    void beginLayout( LayoutMode m = MultiLine );
    void beginLine( int width );

    bool atEnd() const;
    QTextItem nextItem();
    QTextItem currentItem();
    /* ## maybe also currentItem() */
    void setLineWidth( int newWidth );
    int lineWidth() const;
    int widthUsed() const;
    int availableWidth() const;

    enum Result {
	Ok,
	LineFull,
	LineEmpty,
	Error
    };
    /* returns true if completely added */
    Result addCurrentItem();

    /* Note: if ascent and descent are used they must be initialized to the minimum ascent/descent
       acceptable for the line. QFontMetrics::ascent/descent() is usually the right choice */
    Result endLine( int x = 0, int y = 0, int alignment = Qt::AlignLeft,
		    int *ascent = 0, int *descent = 0, int *left = 0, int *right = 0 );
    void endLayout();
#endif

    enum CursorMode {
	SkipCharacters,
	SkipWords
    };
    bool validCursorPosition( int pos ) const;
    int nextCursorPosition( int oldPos, CursorMode mode = SkipCharacters ) const;
    int previousCursorPosition( int oldPos, CursorMode mode = SkipCharacters ) const;

    QTextEngine *engine() { return d; }

private:
    QTextLayout( QTextEngine *e ) : d( e ) {}
    /* disable copy and assignment */
    QTextLayout( const QTextLayout & ) {}
    void operator = ( const QTextLayout & ) {}

    friend class QTextItem;
    friend class QPainter;
    friend class QPSPrinter;
    QTextEngine *d;
};


/*
  class QPainter {
      .....
      void drawTextItem( int x, int y, QTextItem *item );
  };
*/

#endif
