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
#include "qvector.h"
#include "qcolor.h"
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

    int format() const;

private:
    friend class QTextLayout;
    friend class QPainter;
    friend class QPSPrinter;
    int itm;
    QTextEngine *eng;
};

class QTextLine
{
public:
    QTextLine( int line, QTextEngine *e ) : i( line ), eng( e ) {}
    inline QTextLine() : i(0), eng(0) {}
    inline bool isValid() const { return (bool)eng; }

    QRect rect() const;
    int x() const;
    int y() const;
    int width() const;
    int ascent() const;
    int descent() const;
    int textWidth() const;

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

    void adjust(int y, int x1, int x2);
    int from() const;
    int length() const;

    QTextEngine *engine() const { return eng; }
    int line() const { return i; }

    void draw(QPainter *p, int x, int y, int *underlinePositions = 0) const;

private:
    friend class QTextLayout;
    int i;
    QTextEngine *eng;
};

class QPainter;
class QTextFormatCollection;

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
    void setFormatCollection(const QTextFormatCollection *formats);
    void setText( const QString& string);
    QString text() const;

    enum LineBreakStrategy {
	AtWordBoundaries,
	AtCharBoundaries
    };

    void setFormat(int from, int length, int format);
    void setTextFlags(int textFlags);
    void setPalette(const QPalette &);

    int numItems() const;
    QTextItem itemAt( int i ) const;
    QTextItem findItem( int strPos ) const;

    enum LayoutMode {
	NoBidi,
	SingleLine,
	MultiLine
    };

    QTextLine createLine(int from, int y, int x1, int x2);
    int numLines() const;
    QTextLine lineAt(int i) const;
    QTextLine findLine(int pos) const;

    // ### go away!
    void beginLayout( LayoutMode m = MultiLine, int textFlags = 0 );


    enum CursorMode {
	SkipCharacters,
	SkipWords
    };
    bool validCursorPosition( int pos ) const;
    int nextCursorPosition( int oldPos, CursorMode mode = SkipCharacters ) const;
    int previousCursorPosition( int oldPos, CursorMode mode = SkipCharacters ) const;

    enum SelectionType {
	Highlight = -1,
	ImText = -2,
	ImSelection = -3
    };
    class Selection {
	int f;
	int l;
	int t;
    public:
	Selection() : f(-1), l(0), t(0) {}
	//Selection(int f, int l, int formatIndex) : from(f), length(l), selectionType(formatIndex) {}
	Selection(int from, int length, SelectionType type) : f(from), l(length), t(type) {}
	inline int from() const { return f; }
	inline int length() const { return l; }
	inline int type() const { return t; }
	inline void setRange(int from, int length) { f = from; l = length; }
	inline void setType(SelectionType type) { t = type; }
    };

    enum { NoCursor = -1 };

    inline void draw(QPainter *p, const QPoint &pos, int cursorPos, const QVector<Selection> &selections) const {
	draw(p, pos, cursorPos, selections.constData(), selections.size());
    }
    void draw(QPainter *p, const QPoint &pos, int cursorPos = NoCursor, const Selection *selections = 0, int nSelections = 0, const QRect &cr = QRect()) const;


    QTextEngine *engine() const { return d; }

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
