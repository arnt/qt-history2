/****************************************************************************
** $Id$
**
** Definition of internal rich text classes
**
** Created : 990124
**
** Copyright (C) 1999-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QRICHTEXT_P_H
#define QRICHTEXT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qstring.h"
#include "qptrlist.h"
#include "qrect.h"
#include "qfontmetrics.h"
#include "qintdict.h"
#include "qmap.h"
#include "qstringlist.h"
#include "qfont.h"
#include "qcolor.h"
#include "qsize.h"
#include "qvaluelist.h"
#include "qvaluestack.h"
#include "qobject.h"
#include "qdict.h"
#include "qpixmap.h"
#include "qstylesheet.h"
#include "qptrvector.h"
#include "qpainter.h"
#include "qlayout.h"
#include "qobject.h"
#include "private/qcomplextext_p.h"
#include "qapplication.h"
#endif // QT_H

#ifndef QT_NO_RICHTEXT

//#define DEBUG_COLLECTION

class QTextDocument;
class QTextString;
class QTextPreProcessor;
class QTextFormat;
class QTextCursor;
class QTextParag;
class QTextFormatter;
class QTextIndent;
class QTextFormatCollection;
class QStyleSheetItem;
#ifndef QT_NO_TEXTCUSTOMITEM
class QTextCustomItem;
#endif
class QTextFlow;
struct QBidiContext;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Q_EXPORT QTextStringChar
{
    friend class QTextString;

public:
    // this is never called, initialize variables in QTextString::insert()!!!
    QTextStringChar() : lineStart( 0 ), type( Regular ), startOfRun( 0 ) {d.format=0;}
    ~QTextStringChar();

    QChar c;
    enum Type { Regular=0, Custom=1, Anchor=2, CustomAnchor=3 };
    uint lineStart : 1;
    uint rightToLeft : 1;
    uint hasCursor : 1;
    uint canBreak : 1;
    Type type : 2;
    uint startOfRun : 1;

    int x;
    int height() const;
    int ascent() const;
    int descent() const;
    bool isCustom() const { return (type & Custom) != 0; }
    QTextFormat *format() const;
#ifndef QT_NO_TEXTCUSTOMITEM
    QTextCustomItem *customItem() const;
#endif
    void setFormat( QTextFormat *f );
#ifndef QT_NO_TEXTCUSTOMITEM
    void setCustomItem( QTextCustomItem *i );
#endif
    QTextStringChar *clone() const;
    struct CustomData
    {
	QTextFormat *format;
#ifndef QT_NO_TEXTCUSTOMITEM
	QTextCustomItem *custom;
#endif
	QString anchorName;
	QString anchorHref;
    };

#ifndef QT_NO_TEXTCUSTOMITEM
    void loseCustomItem();
#endif    

    union {
	QTextFormat* format;
	CustomData* custom;
    } d;

    bool isAnchor() const { return ( type & Anchor) != 0; }
    QString anchorName() const;
    QString anchorHref() const;
    void setAnchor( const QString& name, const QString& href );

private:
    QTextStringChar &operator=( const QTextStringChar & ) {
	//abort();
	return *this;
    }
    friend class QComplexText;
    friend class QTextParag;
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QMemArray<QTextStringChar>;
// MOC_SKIP_END
#endif

class Q_EXPORT QTextString
{
public:

    QTextString();
    QTextString( const QTextString &s );
    virtual ~QTextString();

    static QString toString( const QMemArray<QTextStringChar> &data );
    QString toString() const;

    QTextStringChar &at( int i ) const;
    int length() const;

    int width( int idx ) const;

    void insert( int index, const QString &s, QTextFormat *f );
    void insert( int index, QTextStringChar *c );
    void truncate( int index );
    void remove( int index, int len );
    void clear();

    void setFormat( int index, QTextFormat *f, bool useCollection );

    void setBidi( bool b ) { bidi = b; }
    bool isBidi() const;
    bool isRightToLeft() const;
    QChar::Direction direction() const;
    void setDirection( QChar::Direction d ) { dir = d; bidiDirty = TRUE; }

    QMemArray<QTextStringChar> subString( int start = 0, int len = 0xFFFFFF ) const;
    QMemArray<QTextStringChar> rawData() const { return data; }

    void operator=( const QString &s ) { clear(); insert( 0, s, 0 ); }
    void operator+=( const QString &s );
    void prepend( const QString &s ) { insert( 0, s, 0 ); }

private:
    void checkBidi() const;

    QMemArray<QTextStringChar> data;
    uint bidiDirty : 1;
    uint bidi : 1; // true when the paragraph has right to left characters
    uint rightToLeft : 1;
    uint dir : 5;
};

inline bool QTextString::isBidi() const
{
    if ( bidiDirty )
	checkBidi();
    return bidi;
}

inline bool QTextString::isRightToLeft() const
{
    if ( bidiDirty )
	checkBidi();
    return rightToLeft;
}

inline QChar::Direction QTextString::direction() const
{
    return (QChar::Direction) dir;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QValueStack<int>;
template class Q_EXPORT QValueStack<QTextParag*>;
template class Q_EXPORT QValueStack<bool>;
// MOC_SKIP_END
#endif

class Q_EXPORT QTextCursor
{
public:
    QTextCursor( QTextDocument *d );
    QTextCursor();
    QTextCursor( const QTextCursor &c );
    QTextCursor &operator=( const QTextCursor &c );
    virtual ~QTextCursor() {}

    bool operator==( const QTextCursor &c ) const;
    bool operator!=( const QTextCursor &c ) const { return !(*this == c); }

    QTextDocument *document() const { return doc; }
    void setDocument( QTextDocument *d );

    QTextParag *parag() const;
    int index() const;
    void setParag( QTextParag *s, bool restore = TRUE );

    void gotoLeft();
    void gotoRight();
    void gotoNextLetter();
    void gotoPreviousLetter();
    void gotoUp();
    void gotoDown();
    void gotoLineEnd();
    void gotoLineStart();
    void gotoHome();
    void gotoEnd();
    void gotoPageUp( int visibleHeight );
    void gotoPageDown( int visibleHeight );
    void gotoNextWord();
    void gotoPreviousWord();
    void gotoWordLeft();
    void gotoWordRight();

    void insert( const QString &s, bool checkNewLine, QMemArray<QTextStringChar> *formatting = 0 );
    void splitAndInsertEmptyParag( bool ind = TRUE, bool updateIds = TRUE );
    bool remove();
    void killLine();
    void indent();

    bool atParagStart();
    bool atParagEnd();

    void setIndex( int i, bool restore = TRUE );

    void checkIndex();

    int offsetX() const { return ox; }
    int offsetY() const { return oy; }

    QTextParag *topParag() const { return parags.isEmpty() ? string : parags.first(); }
    int totalOffsetX() const;
    int totalOffsetY() const;

    bool place( const QPoint &pos, QTextParag *s ) { return place( pos, s, FALSE ); }
    bool place( const QPoint &pos, QTextParag *s, bool link );
    void restoreState();

    int x() const;
    int y() const;

    int nestedDepth() const { return (int)indices.count(); } //### size_t/int cast
    void oneUp() { if ( !indices.isEmpty() ) pop(); }
    void setValid( bool b ) { valid = b; }
    bool isValid() const { return valid; }

private:
    enum Operation { EnterBegin, EnterEnd, Next, Prev, Up, Down };

    void push();
    void pop();
    void processNesting( Operation op );
    void invalidateNested();
#ifndef QT_NO_TEXTCUSTOMITEM
    void gotoIntoNested( const QPoint &globalPos );
#endif
    QTextParag *string;
    QTextDocument *doc;
    int idx, tmpIndex;
    int ox, oy;
    QValueStack<int> indices;
    QValueStack<QTextParag*> parags;
    QValueStack<int> xOffsets;
    QValueStack<int> yOffsets;
    QValueStack<bool> nestedStack;
    uint nested : 1;
    uint valid : 1;

};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Q_EXPORT QTextCommand
{
public:
    enum Commands { Invalid, Insert, Delete, Format, Alignment, ParagType };

    QTextCommand( QTextDocument *d ) : doc( d ), cursor( d ) {}
    virtual ~QTextCommand();

    virtual Commands type() const;

    virtual QTextCursor *execute( QTextCursor *c ) = 0;
    virtual QTextCursor *unexecute( QTextCursor *c ) = 0;

protected:
    QTextDocument *doc;
    QTextCursor cursor;

};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QPtrList<QTextCommand>;
// MOC_SKIP_END
#endif

class Q_EXPORT QTextCommandHistory
{
public:
    QTextCommandHistory( int s ) : current( -1 ), steps( s ) { history.setAutoDelete( TRUE ); }
    virtual ~QTextCommandHistory();

    void clear() { history.clear(); current = -1; }

    void addCommand( QTextCommand *cmd );
    QTextCursor *undo( QTextCursor *c );
    QTextCursor *redo( QTextCursor *c );

    bool isUndoAvailable();
    bool isRedoAvailable();

    void setUndoDepth( int d ) { steps = d; }
    int undoDepth() const { return steps; }

    int historySize() const { return history.count(); }
    int currentPosition() const { return current; }

private:
    QPtrList<QTextCommand> history;
    int current, steps;

};

inline QTextCommandHistory::~QTextCommandHistory()
{
    clear();
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef QT_NO_TEXTCUSTOMITEM
class Q_EXPORT QTextCustomItem
{
public:
    QTextCustomItem( QTextDocument *p )
	:  xpos(0), ypos(-1), width(-1), height(0), parent( p )
    {}
    virtual ~QTextCustomItem();
    virtual void draw(QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected ) = 0;

    virtual void adjustToPainter( QPainter* );

    enum Placement { PlaceInline = 0, PlaceLeft, PlaceRight };
    virtual Placement placement() const;
    bool placeInline() { return placement() == PlaceInline; }

    virtual bool ownLine() const;
    virtual void resize( int nwidth );
    virtual void invalidate();
    virtual int ascent() const { return height; }

    virtual bool isNested() const;
    virtual int minimumWidth() const;

    virtual QString richText() const;

    int xpos; // used for floating items
    int ypos; // used for floating items
    int width;
    int height;

    QRect geometry() const { return QRect( xpos, ypos, width, height ); }

    virtual bool enter( QTextCursor *, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy, bool atEnd = FALSE );
    virtual bool enterAt( QTextCursor *, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy, const QPoint & );
    virtual bool next( QTextCursor *, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool prev( QTextCursor *, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool down( QTextCursor *, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool up( QTextCursor *, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );

    void setParagraph( QTextParag *p ) { parag = p; }
    QTextParag *paragrapth() const { return parag; }

    QTextDocument *parent;
    QTextParag *parag;

    virtual void pageBreak( int  y, QTextFlow* flow );
};
#endif

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QMap<QString, QString>;
// MOC_SKIP_END
#endif

#ifndef QT_NO_TEXTCUSTOMITEM
class Q_EXPORT QTextImage : public QTextCustomItem
{
public:
    QTextImage( QTextDocument *p, const QMap<QString, QString> &attr, const QString& context,
		QMimeSourceFactory &factory );
    virtual ~QTextImage();

    Placement placement() const { return place; }
    void adjustToPainter( QPainter* );
    int minimumWidth() const { return width; }

    QString richText() const;

    void draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected );

private:
    QRegion* reg;
    QPixmap pm;
    Placement place;
    int tmpwidth, tmpheight;
    QMap<QString, QString> attributes;
    QString imgId;

};
#endif

#ifndef QT_NO_TEXTCUSTOMITEM
class Q_EXPORT QTextHorizontalLine : public QTextCustomItem
{
public:
    QTextHorizontalLine( QTextDocument *p, const QMap<QString, QString> &attr, const QString& context,
			 QMimeSourceFactory &factory );
    virtual ~QTextHorizontalLine();

    void adjustToPainter( QPainter* );
    void draw(QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected );
    QString richText() const;

    bool ownLine() const { return TRUE; }

private:
    int tmpheight;
    QColor color;

};
#endif

#ifndef QT_NO_TEXTCUSTOMITEM
#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QPtrList<QTextCustomItem>;
// MOC_SKIP_END
#endif
#endif

class Q_EXPORT QTextFlow
{
    friend class QTextDocument;
#ifndef QT_NO_TEXTCUSTOMITEM
    friend class QTextTableCell;
#endif

public:
    QTextFlow();
    virtual ~QTextFlow();

    virtual void setWidth( int width );
    int width() const;

    virtual void setPageSize( int ps );
    int pageSize() const { return pagesize; }

    virtual int adjustLMargin( int yp, int h, int margin, int space );
    virtual int adjustRMargin( int yp, int h, int margin, int space );

#ifndef QT_NO_TEXTCUSTOMITEM
    virtual void registerFloatingItem( QTextCustomItem* item );
    virtual void unregisterFloatingItem( QTextCustomItem* item );
#endif
    virtual QRect boundingRect() const;
    virtual void drawFloatingItems(QPainter* p, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected );

    virtual int adjustFlow( int  y, int w, int h ); // adjusts y according to the defined pagesize. Returns the shift.

    virtual bool isEmpty();

    void clear();

private:
    int w;
    int pagesize;

#ifndef QT_NO_TEXTCUSTOMITEM
    QPtrList<QTextCustomItem> leftItems;
    QPtrList<QTextCustomItem> rightItems;
#endif
};

inline int QTextFlow::width() const { return w; }

#ifndef QT_NO_TEXTCUSTOMITEM
class QTextTable;

class Q_EXPORT QTextTableCell : public QLayoutItem
{
    friend class QTextTable;

public:
    QTextTableCell( QTextTable* table,
		    int row, int column,
		    const QMap<QString, QString> &attr,
		    const QStyleSheetItem* style,
		    const QTextFormat& fmt, const QString& context,
		    QMimeSourceFactory &factory, QStyleSheet *sheet, const QString& doc );
    QTextTableCell( QTextTable* table, int row, int column );
    virtual ~QTextTableCell();

    QSize sizeHint() const ;
    QSize minimumSize() const ;
    QSize maximumSize() const ;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry( const QRect& ) ;
    QRect geometry() const;

    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;

    void adjustToPainter( QPainter* );

    int row() const { return row_; }
    int column() const { return col_; }
    int rowspan() const { return rowspan_; }
    int colspan() const { return colspan_; }
    int stretch() const { return stretch_; }

    QTextDocument* richText()  const { return richtext; }
    QTextTable* table() const { return parent; }

    void draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected );

    QBrush *backGround() const { return background; }
    virtual void invalidate();

    int verticalAlignmentOffset() const;
    int horizontalAlignmentOffset() const;

private:
    QRect geom;
    QTextTable* parent;
    QTextDocument* richtext;
    int row_;
    int col_;
    int rowspan_;
    int colspan_;
    int stretch_;
    int maxw;
    int minw;
    bool hasFixedWidth;
    QBrush *background;
    int cached_width;
    int cached_sizehint;
    QMap<QString, QString> attributes;
    int align;
};
#endif

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QPtrList<QTextTableCell>;
template class Q_EXPORT QMap<QTextCursor*, int>;
// MOC_SKIP_END
#endif

#ifndef QT_NO_TEXTCUSTOMITEM
class Q_EXPORT QTextTable: public QTextCustomItem
{
    friend class QTextTableCell;

public:
    QTextTable( QTextDocument *p, const QMap<QString, QString> &attr );
    virtual ~QTextTable();

    void adjustToPainter( QPainter *p );
    void pageBreak( int  y, QTextFlow* flow );
    void draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch,
	       const QColorGroup& cg, bool selected );

    bool noErase() const { return TRUE; }
    bool ownLine() const { return TRUE; }
    Placement placement() const { return place; }
    bool isNested() const { return TRUE; }
    void resize( int nwidth );
    virtual void invalidate();

    virtual bool enter( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy, bool atEnd = FALSE );
    virtual bool enterAt( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy, const QPoint &pos );
    virtual bool next( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool prev( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool down( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool up( QTextCursor *c, QTextDocument *&doc, QTextParag *&parag, int &idx, int &ox, int &oy );

    QString richText() const;

    int minimumWidth() const;

    QPtrList<QTextTableCell> tableCells() const { return cells; }

    bool isStretching() const { return stretch; }

private:
    void format( int w );
    void addCell( QTextTableCell* cell );

private:
    QGridLayout* layout;
    QPtrList<QTextTableCell> cells;
    int cachewidth;
    int fixwidth;
    int cellpadding;
    int cellspacing;
    int border;
    int outerborder;
    int stretch;
    int innerborder;
    int us_cp, us_ib, us_b, us_ob, us_cs;
    QMap<QString, QString> attributes;
    QMap<QTextCursor*, int> currCell;
    Placement place;
    void adjustCells( int y , int shift );
    int pageBreakFor;
};
#endif
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef QT_NO_TEXTCUSTOMITEM
class QTextTableCell;
#endif
class QTextParag;

struct Q_EXPORT QTextDocumentSelection
{
    QTextCursor startCursor, endCursor;
    bool swapped;
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QMap<int, QColor>;
template class Q_EXPORT QMap<int, bool>;
template class Q_EXPORT QMap<int, QTextDocumentSelection>;
template class Q_EXPORT QPtrList<QTextDocument>;
// MOC_SKIP_END
#endif

class Q_EXPORT QTextDocument : public QObject
{
    Q_OBJECT

    friend class QTextTableCell;
    friend class QTextCursor;
    friend class QTextEdit;
    friend class QTextParag;

public:
    enum SelectionIds {
	Standard = 0,
	Temp = 32000 // This selection must not be drawn, it's used e.g. by undo/redo to
	// remove multiple lines with removeSelectedText()
    };

    QTextDocument( QTextDocument *p );
    QTextDocument( QTextDocument *d, QTextFormatCollection *f );
    virtual ~QTextDocument();

    QTextDocument *parent() const { return par; }
    QTextParag *parentParag() const { return parParag; }

    void setText( const QString &text, const QString &context );
    QMap<QString, QString> attributes() const { return attribs; }
    void setAttributes( const QMap<QString, QString> &attr ) { attribs = attr; }

    QString text() const;
    QString text( int parag ) const;
    QString originalText() const;

    int x() const;
    int y() const;
    int width() const;
    int widthUsed() const;
    int visibleWidth() const;
    int height() const;
    void setWidth( int w );
    int minimumWidth() const;
    bool setMinimumWidth( int needed, int used = -1, QTextParag *parag = 0 );

    void setY( int y );
    int leftMargin() const;
    void setLeftMargin( int lm );
    int rightMargin() const;
    void setRightMargin( int rm );

    QTextParag *firstParag() const;
    QTextParag *lastParag() const;
    void setFirstParag( QTextParag *p );
    void setLastParag( QTextParag *p );

    void invalidate();

    void setPreProcessor( QTextPreProcessor *sh );
    QTextPreProcessor *preProcessor() const;

    void setFormatter( QTextFormatter *f );
    QTextFormatter *formatter() const;

    void setIndent( QTextIndent *i );
    QTextIndent *indent() const;

    QColor selectionColor( int id ) const;
    bool invertSelectionText( int id ) const;
    void setSelectionColor( int id, const QColor &c );
    void setInvertSelectionText( int id, bool b );
    bool hasSelection( int id, bool visible = FALSE ) const;
    void setSelectionStart( int id, QTextCursor *cursor );
    bool setSelectionEnd( int id, QTextCursor *cursor );
    void selectAll( int id );
    bool removeSelection( int id );
    void selectionStart( int id, int &paragId, int &index );
    QTextCursor selectionStartCursor( int id );
    QTextCursor selectionEndCursor( int id );
    void selectionEnd( int id, int &paragId, int &index );
    void setFormat( int id, QTextFormat *f, int flags );
    QTextParag *selectionStart( int id );
    QTextParag *selectionEnd( int id );
    int numSelections() const { return nSelections; }
    void addSelection( int id );

    QString selectedText( int id, bool withCustom = TRUE ) const;
    void copySelectedText( int id );
    void removeSelectedText( int id, QTextCursor *cursor );
    void indentSelection( int id );

    QTextParag *paragAt( int i ) const;

    void addCommand( QTextCommand *cmd );
    QTextCursor *undo( QTextCursor *c = 0 );
    QTextCursor *redo( QTextCursor *c  = 0 );
    QTextCommandHistory *commands() const { return commandHistory; }

    QTextFormatCollection *formatCollection() const;

    bool find( const QString &expr, bool cs, bool wo, bool forward, int *parag, int *index, QTextCursor *cursor );

    void setTextFormat( Qt::TextFormat f );
    Qt::TextFormat textFormat() const;

    bool inSelection( int selId, const QPoint &pos ) const;

    QStyleSheet *styleSheet() const { return sheet_; }
#ifndef QT_NO_MIME
    QMimeSourceFactory *mimeSourceFactory() const { return factory_; }
#endif
    QString context() const { return contxt; }

    void setStyleSheet( QStyleSheet *s );
    void updateFontSizes( int base, bool usePixels );
    void updateFontAttributes( const QFont &f, const QFont &old );
#ifndef QT_NO_MIME
    void setMimeSourceFactory( QMimeSourceFactory *f ) { if ( f ) factory_ = f; }
#endif
    void setContext( const QString &c ) { if ( !c.isEmpty() ) contxt = c; }

    void setUnderlineLinks( bool b );
    bool underlineLinks() const { return underlLinks; }

    void setPaper( QBrush *brush ) { if ( backBrush ) delete backBrush; backBrush = brush; }
    QBrush *paper() const { return backBrush; }

    void doLayout( QPainter *p, int w );
    void draw( QPainter *p, const QRect& rect, const QColorGroup &cg, const QBrush *paper = 0 );
    void drawParag( QPainter *p, QTextParag *parag, int cx, int cy, int cw, int ch,
		    QPixmap *&doubleBuffer, const QColorGroup &cg,
		    bool drawCursor, QTextCursor *cursor, bool resetChanged = TRUE );
    QTextParag *draw( QPainter *p, int cx, int cy, int cw, int ch, const QColorGroup &cg,
		      bool onlyChanged = FALSE, bool drawCursor = FALSE, QTextCursor *cursor = 0,
		      bool resetChanged = TRUE );

    void setDefaultFont( const QFont &f );

#ifndef QT_NO_TEXTCUSTOMITEM
    void registerCustomItem( QTextCustomItem *i, QTextParag *p );
    void unregisterCustomItem( QTextCustomItem *i, QTextParag *p );
#endif

    void setFlow( QTextFlow *f );
    void takeFlow();
    QTextFlow *flow() const { return flow_; }
    bool isPageBreakEnabled() const { return pages; }
    void setPageBreakEnabled( bool b ) { pages = b; }

    void setUseFormatCollection( bool b ) { useFC = b; }
    bool useFormatCollection() const { return useFC; }

    QTextTableCell *tableCell() const { return tc; }
    void setTableCell( QTextTableCell *c ) { tc = c; }

    void setPlainText( const QString &text );
    void setRichText( const QString &text, const QString &context );
    QString richText( QTextParag *p = 0 ) const;
    QString plainText( QTextParag *p = 0 ) const;

    bool focusNextPrevChild( bool next );

    int alignment() const;
    void setAlignment( int a );

    int *tabArray() const;
    int tabStopWidth() const;
    void setTabArray( int *a );
    void setTabStops( int tw );

    void setUndoDepth( int d ) { commandHistory->setUndoDepth( d ); }
    int undoDepth() const { return commandHistory->undoDepth(); }

    int length() const;
    void clear( bool createEmptyParag = FALSE );

    virtual QTextParag *createParag( QTextDocument *d, QTextParag *pr = 0, QTextParag *nx = 0, bool updateIds = TRUE );
    void insertChild( QObject *o ) { QObject::insertChild( o ); }
    void removeChild( QObject *o ) { QObject::removeChild( o ); }
    void insertChild( QTextDocument *d ) { childList.append( d ); }
    void removeChild( QTextDocument *d ) { childList.removeRef( d ); }
    QPtrList<QTextDocument> children() const { return childList; }

    bool hasFocusParagraph() const;
    QString focusHref() const;
    QString focusName() const;

    void invalidateOriginalText() { oTextValid = FALSE; oText = ""; }

signals:
    void minimumWidthChanged( int );

private:
    void init();
    QPixmap *bufferPixmap( const QSize &s );
    // HTML parser
    bool hasPrefix(const QChar* doc, int length, int pos, QChar c);
    bool hasPrefix(const QChar* doc, int length, int pos, const QString& s);
#ifndef QT_NO_TEXTCUSTOMITEM
    QTextCustomItem* parseTable( const QMap<QString, QString> &attr, const QTextFormat &fmt,
				 const QChar* doc, int length, int& pos, QTextParag *curpar );
#endif
    bool eatSpace(const QChar* doc, int length, int& pos, bool includeNbsp = FALSE );
    bool eat(const QChar* doc, int length, int& pos, QChar c);
    QString parseOpenTag(const QChar* doc, int length, int& pos, QMap<QString, QString> &attr, bool& emptyTag);
    QString parseCloseTag( const QChar* doc, int length, int& pos );
    QChar parseHTMLSpecialChar(const QChar* doc, int length, int& pos);
    QString parseWord(const QChar* doc, int length, int& pos, bool lower = TRUE);
    QChar parseChar(const QChar* doc, int length, int& pos, QStyleSheetItem::WhiteSpaceMode wsm );
    void setRichTextInternal( const QString &text );

private:
    struct Q_EXPORT Focus {
	QTextParag *parag;
	int start, len;
	QString href;
	QString name;
    };

    int cx, cy, cw, vw;
    QTextParag *fParag, *lParag;
    QTextPreProcessor *pProcessor;
    QMap<int, QColor> selectionColors;
    QMap<int, QTextDocumentSelection> selections;
    QMap<int, bool> selectionText;
    QTextCommandHistory *commandHistory;
    QTextFormatter *pFormatter;
    QTextIndent *indenter;
    QTextFormatCollection *fCollection;
    Qt::TextFormat txtFormat;
    uint preferRichText : 1;
    uint pages : 1;
    uint useFC : 1;
    uint withoutDoubleBuffer : 1;
    uint underlLinks : 1;
    uint nextDoubleBuffered : 1;
    uint oTextValid : 1;
    uint mightHaveCustomItems : 1;
    int align;
    int nSelections;
    QTextFlow *flow_;
    QTextDocument *par;
    QTextParag *parParag;
    QTextTableCell *tc;
    QTextCursor *tmpCursor;
    QBrush *backBrush;
    QPixmap *buf_pixmap;
    Focus focusIndicator;
    int minw;
    int wused;
    int leftmargin;
    int rightmargin;
    QTextParag *minwParag, *curParag;
    QStyleSheet* sheet_;
#ifndef QT_NO_MIME
    QMimeSourceFactory* factory_;
#endif
    QString contxt;
    QMap<QString, QString> attribs;
    int *tArray;
    int tStopWidth;
    int uDepth;
    QString oText;
    QPtrList<QTextDocument> childList;
    QColor linkColor;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


class Q_EXPORT QTextDeleteCommand : public QTextCommand
{
public:
    QTextDeleteCommand( QTextDocument *d, int i, int idx, const QMemArray<QTextStringChar> &str,
			const QValueList< QPtrVector<QStyleSheetItem> > &os,
			const QValueList<QStyleSheetItem::ListStyle> &ols,
			const QMemArray<int> &oas );
    QTextDeleteCommand( QTextParag *p, int idx, const QMemArray<QTextStringChar> &str );
    virtual ~QTextDeleteCommand();

    Commands type() const { return Delete; }
    QTextCursor *execute( QTextCursor *c );
    QTextCursor *unexecute( QTextCursor *c );

protected:
    int id, index;
    QTextParag *parag;
    QMemArray<QTextStringChar> text;
    QValueList< QPtrVector<QStyleSheetItem> > oldStyles;
    QValueList<QStyleSheetItem::ListStyle> oldListStyles;
    QMemArray<int> oldAligns;

};

class Q_EXPORT QTextInsertCommand : public QTextDeleteCommand
{
public:
    QTextInsertCommand( QTextDocument *d, int i, int idx, const QMemArray<QTextStringChar> &str,
			const QValueList< QPtrVector<QStyleSheetItem> > &os,
			const QValueList<QStyleSheetItem::ListStyle> &ols,
			const QMemArray<int> &oas )
	: QTextDeleteCommand( d, i, idx, str, os, ols, oas ) {}
    QTextInsertCommand( QTextParag *p, int idx, const QMemArray<QTextStringChar> &str )
	: QTextDeleteCommand( p, idx, str ) {}
    virtual ~QTextInsertCommand() {}

    Commands type() const { return Insert; }
    QTextCursor *execute( QTextCursor *c ) { return QTextDeleteCommand::unexecute( c ); }
    QTextCursor *unexecute( QTextCursor *c ) { return QTextDeleteCommand::execute( c ); }

};

class Q_EXPORT QTextFormatCommand : public QTextCommand
{
public:
    QTextFormatCommand( QTextDocument *d, int sid, int sidx, int eid, int eidx, const QMemArray<QTextStringChar> &old, QTextFormat *f, int fl );
    virtual ~QTextFormatCommand();

    Commands type() const { return Format; }
    QTextCursor *execute( QTextCursor *c );
    QTextCursor *unexecute( QTextCursor *c );

protected:
    int startId, startIndex, endId, endIndex;
    QTextFormat *format;
    QMemArray<QTextStringChar> oldFormats;
    int flags;

};

class Q_EXPORT QTextAlignmentCommand : public QTextCommand
{
public:
    QTextAlignmentCommand( QTextDocument *d, int fParag, int lParag, int na, const QMemArray<int> &oa );
    virtual ~QTextAlignmentCommand() {}

    Commands type() const { return Alignment; }
    QTextCursor *execute( QTextCursor *c );
    QTextCursor *unexecute( QTextCursor *c );

private:
    int firstParag, lastParag;
    int newAlign;
    QMemArray<int> oldAligns;

};

class Q_EXPORT QTextParagTypeCommand : public QTextCommand
{
public:
    QTextParagTypeCommand( QTextDocument *d, int fParag, int lParag, bool l,
			   QStyleSheetItem::ListStyle s, const QValueList< QPtrVector<QStyleSheetItem> > &os,
			   const QValueList<QStyleSheetItem::ListStyle> &ols );
    virtual ~QTextParagTypeCommand() {}

    Commands type() const { return ParagType; }
    QTextCursor *execute( QTextCursor *c );
    QTextCursor *unexecute( QTextCursor *c );

private:
    int firstParag, lastParag;
    bool list;
    QStyleSheetItem::ListStyle listStyle;
    QValueList< QPtrVector<QStyleSheetItem> > oldStyles;
    QValueList<QStyleSheetItem::ListStyle> oldListStyles;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct Q_EXPORT QTextParagSelection
{
    int start, end;
};

struct Q_EXPORT QTextParagLineStart
{
    QTextParagLineStart() : y( 0 ), baseLine( 0 ), h( 0 )
#ifndef QT_NO_COMPLEXTEXT
	, bidicontext( 0 )
#endif
    {  }
    QTextParagLineStart( ushort y_, ushort bl, ushort h_ ) : y( y_ ), baseLine( bl ), h( h_ ),
	w( 0 )
#ifndef QT_NO_COMPLEXTEXT
	, bidicontext( 0 )
#endif
    {  }
#ifndef QT_NO_COMPLEXTEXT
    QTextParagLineStart( QBidiContext *c, QBidiStatus s ) : y(0), baseLine(0), h(0),
	status( s ), bidicontext( c ) { if ( bidicontext ) bidicontext->ref(); }
#endif

    virtual ~QTextParagLineStart()
    {
#ifndef QT_NO_COMPLEXTEXT
	if ( bidicontext && bidicontext->deref() )
	    delete bidicontext;
#endif
    }

#ifndef QT_NO_COMPLEXTEXT
    void setContext( QBidiContext *c ) {
	if ( c == bidicontext )
	    return;
	if ( bidicontext && bidicontext->deref() )
	    delete bidicontext;
	bidicontext = c;
	if ( bidicontext )
	    bidicontext->ref();
    }
    QBidiContext *context() const { return bidicontext; }
#endif

public:
    ushort y, baseLine, h;
#ifndef QT_NO_COMPLEXTEXT
    QBidiStatus status;
#endif
    int w;

private:
#ifndef QT_NO_COMPLEXTEXT
    QBidiContext *bidicontext;
#endif
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QMap<int, QTextParagSelection>;
template class Q_EXPORT QMap<int, QTextParagLineStart*>;
// MOC_SKIP_END
#endif

class Q_EXPORT QTextParagData
{
public:
    QTextParagData() {}
    virtual ~QTextParagData();
    virtual void join( QTextParagData * );
};

class Q_EXPORT QTextParagPseudoDocument
{
public:
    QTextParagPseudoDocument();
    ~QTextParagPseudoDocument();
    QRect docRect;
    QTextFormatter *pFormatter;
    QTextCommandHistory *commandHistory;
    int minw;
    int wused;
};

//nase
class Q_EXPORT QTextParag
{
    friend class QTextDocument;
    friend class QTextCursor;

public:
    QTextParag( QTextDocument *d, QTextParag *pr = 0, QTextParag *nx = 0, bool updateIds = TRUE );
    virtual ~QTextParag();

    QTextString *string() const;
    QTextStringChar *at( int i ) const; // maybe remove later
    int leftGap() const;
    int length() const; // maybe remove later

    void setListStyle( QStyleSheetItem::ListStyle ls );
    QStyleSheetItem::ListStyle listStyle() const;
    void setListValue( int v ) { list_val = v; }
    int listValue() const { return list_val > 0 ? list_val : -1; }

    void setList( bool b, QStyleSheetItem::ListStyle listStyle = QStyleSheetItem::ListDisc);
    void incDepth();
    void decDepth();
    int listDepth() const;

    void setFormat( QTextFormat *fm );
    QTextFormat *paragFormat() const;

    QTextDocument *document() const;
    QTextParagPseudoDocument *pseudoDocument() const;

    QRect rect() const;
    void setHeight( int h ) { r.setHeight( h ); }
    void show();
    void hide();
    bool isVisible() const { return visible; }

    QTextParag *prev() const;
    QTextParag *next() const;
    void setPrev( QTextParag *s );
    void setNext( QTextParag *s );

    void insert( int index, const QString &s );
    void append( const QString &s, bool reallyAtEnd = FALSE );
    void truncate( int index );
    void remove( int index, int len );
    void join( QTextParag *s );

    void invalidate( int chr );

    void move( int &dy );
    void format( int start = -1, bool doMove = TRUE );

    bool isValid() const;
    bool hasChanged() const;
    void setChanged( bool b, bool recursive = FALSE );

    int lineHeightOfChar( int i, int *bl = 0, int *y = 0 ) const;
    QTextStringChar *lineStartOfChar( int i, int *index = 0, int *line = 0 ) const;
    int lines() const;
    QTextStringChar *lineStartOfLine( int line, int *index = 0 ) const;
    int lineY( int l ) const;
    int lineBaseLine( int l ) const;
    int lineHeight( int l ) const;
    void lineInfo( int l, int &y, int &h, int &bl ) const;

    void setSelection( int id, int start, int end );
    void removeSelection( int id );
    int selectionStart( int id ) const;
    int selectionEnd( int id ) const;
    bool hasSelection( int id ) const;
    bool hasAnySelection() const;
    bool fullSelected( int id ) const;

    void setEndState( int s );
    int endState() const;

    void setParagId( int i );
    int paragId() const;

    bool firstPreProcess() const;
    void setFirstPreProcess( bool b );

    void indent( int *oldIndent = 0, int *newIndent = 0 );

    void setExtraData( QTextParagData *data );
    QTextParagData *extraData() const;

    QMap<int, QTextParagLineStart*> &lineStartList();

    void setFormat( int index, int len, QTextFormat *f, bool useCollection = TRUE, int flags = -1 );

    void setAlignment( int a );
    int alignment() const;

    virtual void paint( QPainter &painter, const QColorGroup &cg, QTextCursor *cursor = 0, bool drawSelections = FALSE,
			int clipx = -1, int clipy = -1, int clipw = -1, int cliph = -1 );

    void setStyleSheetItems( const QPtrVector<QStyleSheetItem> &vec );
    QPtrVector<QStyleSheetItem> styleSheetItems() const;
    QStyleSheetItem *style() const;

    virtual int topMargin() const;
    virtual int bottomMargin() const;
    virtual int leftMargin() const;
    virtual int firstLineMargin() const;
    virtual int rightMargin() const;
    virtual int lineSpacing() const;

#ifndef QT_NO_TEXTCUSTOMITEM
    void registerFloatingItem( QTextCustomItem *i );
    void unregisterFloatingItem( QTextCustomItem *i );
#endif

    void setFullWidth( bool b ) { fullWidth = b; }
    bool isFullWidth() const { return fullWidth; }

#ifndef QT_NO_TEXTCUSTOMITEM
    QTextTableCell *tableCell() const;
#endif

    QBrush *background() const;

    int documentWidth() const;
    int documentVisibleWidth() const;
    int documentX() const;
    int documentY() const;
    QTextFormatCollection *formatCollection() const;
    QTextFormatter *formatter() const;

    virtual int nextTab( int i, int x );
    int *tabArray() const;
    void setTabArray( int *a );
    void setTabStops( int tw );

    void adjustToPainter( QPainter *p );

    void setNewLinesAllowed( bool b );
    bool isNewLinesAllowed() const;

    QString richText() const;

    void addCommand( QTextCommand *cmd );
    QTextCursor *undo( QTextCursor *c = 0 );
    QTextCursor *redo( QTextCursor *c  = 0 );
    QTextCommandHistory *commands() const;
    virtual void copyParagData( QTextParag *parag );

    void setBreakable( bool b ) { breakable = b; }
    bool isBreakable() const { return breakable; }

    void setBackgroundColor( const QColor &c );
    QColor *backgroundColor() const { return bgcol; }
    void clearBackgroundColor();

    void setMovedDown( bool b ) { movedDown = b; }
    bool wasMovedDown() const { return movedDown; }

    void setDirection( QChar::Direction d );
    QChar::Direction direction() const;
    void setPaintDevice( QPaintDevice *pd ) { paintdevice = pd; }

protected:
    virtual void drawLabel( QPainter* p, int x, int y, int w, int h, int base, const QColorGroup& cg );
    virtual void drawParagString( QPainter &painter, const QString &str, int start, int len, int startX,
				  int lastY, int baseLine, int bw, int h, bool drawSelections,
				  QTextStringChar *formatChar, int i, const QMemArray<int> &selectionStarts,
				  const QMemArray<int> &selectionEnds, const QColorGroup &cg, bool rightToLeft  );

private:
    QMap<int, QTextParagSelection> &selections() const;
    QPtrVector<QStyleSheetItem> &styleSheetItemsVec() const;
    QPtrList<QTextCustomItem> &floatingItems() const;
    void invalidateStyleCache();

    QMap<int, QTextParagLineStart*> lineStarts;
    int invalid;
    QRect r;
    QTextParag *p, *n;
    void *docOrPseudo;
    uint changed : 1;
    uint firstFormat : 1;
    uint firstPProcess : 1;
    uint needPreProcess : 1;
    uint fullWidth : 1;
    uint newLinesAllowed : 1;
    uint lastInFrame : 1;
    uint visible : 1;
    uint breakable : 1;
    uint movedDown : 1;
    uint mightHaveCustomItems : 1;
    uint hasdoc : 1;
    int align : 4;
    int state, id;
    QTextString *str;
    QMap<int, QTextParagSelection> *mSelections;
    QPtrVector<QStyleSheetItem> *mStyleSheetItemsVec;
    QPtrList<QTextCustomItem> *mFloatingItems;
    QStyleSheetItem::ListStyle listS;
    short tm, bm, lm, rm, flm;
    short utm, ubm, ulm, urm, uflm;
    QTextFormat *defFormat;
    int *tArray;
    short tabStopWidth;
    QTextParagData *eData;
    short list_val;
    QColor *bgcol;
    QPaintDevice *paintdevice;
};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Q_EXPORT QTextFormatter
{
public:
    QTextFormatter();
    virtual ~QTextFormatter();

    virtual int format( QTextDocument *doc, QTextParag *parag, int start, const QMap<int, QTextParagLineStart*> &oldLineStarts ) = 0;
    virtual int formatVertically( QTextDocument* doc, QTextParag* parag );

    bool isWrapEnabled( QTextParag *p ) const { if ( !wrapEnabled ) return FALSE; if ( p && !p->isBreakable() ) return FALSE; return TRUE;}
    int wrapAtColumn() const { return wrapColumn;}
    virtual void setWrapEnabled( bool b );
    virtual void setWrapAtColumn( int c );
    virtual void setAllowBreakInWords( bool b ) { biw = b; }
    bool allowBreakInWords() const { return biw; }

    int minimumWidth() const { return thisminw; }
    int widthUsed() const { return thiswused; }

protected:
    virtual QTextParagLineStart *formatLine( QTextParag *parag, QTextString *string, QTextParagLineStart *line, QTextStringChar *start,
					       QTextStringChar *last, int align = Qt::AlignAuto, int space = 0 );
#ifndef QT_NO_COMPLEXTEXT
    virtual QTextParagLineStart *bidiReorderLine( QTextParag *parag, QTextString *string, QTextParagLineStart *line, QTextStringChar *start,
						    QTextStringChar *last, int align, int space );
#endif
    virtual bool isBreakable( QTextString *string, int pos ) const;
    void insertLineStart( QTextParag *parag, int index, QTextParagLineStart *ls );

    int thisminw;
    int thiswused;

private:
    bool wrapEnabled;
    int wrapColumn;
    bool biw;

#ifdef HAVE_THAI_BREAKS
    static QCString *thaiCache;
    static QTextString *cachedString;
    static ThBreakIterator *thaiIt;
#endif
};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Q_EXPORT QTextFormatterBreakInWords : public QTextFormatter
{
public:
    QTextFormatterBreakInWords();
    virtual ~QTextFormatterBreakInWords() {}

    int format( QTextDocument *doc, QTextParag *parag, int start, const QMap<int, QTextParagLineStart*> &oldLineStarts );

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Q_EXPORT QTextFormatterBreakWords : public QTextFormatter
{
public:
    QTextFormatterBreakWords();
    virtual ~QTextFormatterBreakWords() {}

    int format( QTextDocument *doc, QTextParag *parag, int start, const QMap<int, QTextParagLineStart*> &oldLineStarts );

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Q_EXPORT QTextIndent
{
public:
    QTextIndent();
    virtual ~QTextIndent() {}

    virtual void indent( QTextDocument *doc, QTextParag *parag, int *oldIndent = 0, int *newIndent = 0 ) = 0;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Q_EXPORT QTextPreProcessor
{
public:
    enum Ids {
	Standard = 0
    };

    QTextPreProcessor();
    virtual ~QTextPreProcessor() {}

    virtual void process( QTextDocument *doc, QTextParag *, int, bool = TRUE ) = 0;
    virtual QTextFormat *format( int id ) = 0;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Q_EXPORT QTextFormat
{
    friend class QTextFormatCollection;
    friend class QTextDocument;

public:
    enum Flags {
	NoFlags,
	Bold = 1,
	Italic = 2,
	Underline = 4,
	Family = 8,
	Size = 16,
	Color = 32,
	Misspelled = 64,
	VAlign = 128,
	Font = Bold | Italic | Underline | Family | Size,
	Format = Font | Color | Misspelled | VAlign
    };

    enum VerticalAlignment { AlignNormal, AlignSuperScript, AlignSubScript };

    QTextFormat();
    virtual ~QTextFormat();

    QTextFormat( const QStyleSheetItem *s );
    QTextFormat( const QFont &f, const QColor &c, QTextFormatCollection *parent = 0 );
    QTextFormat( const QTextFormat &fm );
    QTextFormat makeTextFormat( const QStyleSheetItem *style, const QMap<QString,QString>& attr ) const;
    QTextFormat& operator=( const QTextFormat &fm );
    QColor color() const;
    QFont font() const;
    bool isMisspelled() const;
    VerticalAlignment vAlign() const;
    int minLeftBearing() const;
    int minRightBearing() const;
    int width( const QChar &c ) const;
    int width( const QString &str, int pos ) const;
    int height() const;
    int ascent() const;
    int descent() const;
    int leading() const;
    bool useLinkColor() const;

    void setBold( bool b );
    void setItalic( bool b );
    void setUnderline( bool b );
    void setFamily( const QString &f );
    void setPointSize( int s );
    void setFont( const QFont &f );
    void setColor( const QColor &c );
    void setMisspelled( bool b );
    void setVAlign( VerticalAlignment a );

    bool operator==( const QTextFormat &f ) const;
    QTextFormatCollection *parent() const;
    QString key() const;

    static QString getKey( const QFont &f, const QColor &c, bool misspelled, VerticalAlignment vAlign );

    void addRef();
    void removeRef();

    QString makeFormatChangeTags( QTextFormat *f, const QString& oldAnchorHref, const QString& anchorHref ) const;
    QString makeFormatEndTags( const QString& anchorHref ) const;

    static void setPainter( QPainter *p );
    static QPainter* painter();

    bool fontSizesInPixels() { return usePixelSizes; }

protected:
    virtual void generateKey();

private:
    void update();

private:
    QFont fn;
    QColor col;
    QFontMetrics fm;
    uint missp : 1;
    uint linkColor : 1;
    uint usePixelSizes : 1;
    int leftBearing, rightBearing;
    VerticalAlignment ha;
    uchar widths[ 256 ];
    int hei, asc, dsc;
    QTextFormatCollection *collection;
    int ref;
    QString k;
    int logicalFontSize;
    int stdSize;
    static QPainter *pntr;

};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QDict<QTextFormat>;
// MOC_SKIP_END
#endif

class Q_EXPORT QTextFormatCollection
{
    friend class QTextDocument;
    friend class QTextFormat;

public:
    QTextFormatCollection();
    virtual ~QTextFormatCollection();

    void setDefaultFormat( QTextFormat *f );
    QTextFormat *defaultFormat() const;
    virtual QTextFormat *format( QTextFormat *f );
    virtual QTextFormat *format( QTextFormat *of, QTextFormat *nf, int flags );
    virtual QTextFormat *format( const QFont &f, const QColor &c );
    virtual void remove( QTextFormat *f );
    virtual QTextFormat *createFormat( const QTextFormat &f ) { return new QTextFormat( f ); }
    virtual QTextFormat *createFormat( const QFont &f, const QColor &c ) { return new QTextFormat( f, c, this ); }
    void debug();

    void updateFontSizes( QStyleSheet* sheet, int base, bool usePixels );
    void updateFontAttributes( const QFont &f, const QFont &old );
    QDict<QTextFormat> dict() const { return cKey; }

    QPaintDevice *paintDevice() const { return paintdevice; }
    void setPaintDevice( QPaintDevice * );

private:
    void updateKeys();

private:
    QTextFormat *defFormat, *lastFormat, *cachedFormat;
    QDict<QTextFormat> cKey;
    QTextFormat *cres;
    QFont cfont;
    QColor ccol;
    QString kof, knf;
    int cflags;

    QStyleSheet *sheet;
    QPaintDevice *paintdevice;
};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline int QTextString::length() const
{
    return data.size();
}

inline void QTextString::operator+=( const QString &s )
{
    insert( length(), s, 0 );
}

inline int QTextParag::length() const
{
    return str->length();
}

inline QRect QTextParag::rect() const
{
    return r;
}

inline QTextParag *QTextCursor::parag() const
{
    return string;
}

inline int QTextCursor::index() const
{
    return idx;
}

inline void QTextCursor::setIndex( int i, bool restore )
{
    if ( restore )
	restoreState();
    if ( i < 0 || i >= string->length() ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QTextCursor::setIndex: %d out of range", i );
#endif
	i = i < 0 ? 0 : string->length() - 1;
    }

    tmpIndex = -1;
    idx = i;
}

inline void QTextCursor::setParag( QTextParag *s, bool restore )
{
    if ( restore )
	restoreState();
    idx = 0;
    string = s;
    tmpIndex = -1;
}

inline void QTextCursor::checkIndex()
{
    if ( idx >= string->length() )
	idx = string->length() - 1;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline int QTextDocument::x() const
{
    return cx;
}

inline int QTextDocument::y() const
{
    return cy;
}

inline int QTextDocument::width() const
{
    return QMAX( cw, flow_->width() );
}

inline int QTextDocument::visibleWidth() const
{
    return vw;
}

inline QTextParag *QTextDocument::firstParag() const
{
    return fParag;
}

inline QTextParag *QTextDocument::lastParag() const
{
    return lParag;
}

inline void QTextDocument::setFirstParag( QTextParag *p )
{
    fParag = p;
}

inline void QTextDocument::setLastParag( QTextParag *p )
{
    lParag = p;
}

inline void QTextDocument::setWidth( int w )
{
    cw = QMAX( w, minw );
    flow_->setWidth( cw );
    vw = w;
}

inline int QTextDocument::minimumWidth() const
{
    return minw;
}

inline void QTextDocument::setY( int y )
{
    cy = y;
}

inline int QTextDocument::leftMargin() const
{
    return leftmargin;
}

inline void QTextDocument::setLeftMargin( int lm )
{
    leftmargin = lm;
}

inline int QTextDocument::rightMargin() const
{
    return rightmargin;
}

inline void QTextDocument::setRightMargin( int rm )
{
    rightmargin = rm;
}

inline QTextPreProcessor *QTextDocument::preProcessor() const
{
    return pProcessor;
}

inline void QTextDocument::setPreProcessor( QTextPreProcessor * sh )
{
    pProcessor = sh;
}

inline void QTextDocument::setFormatter( QTextFormatter *f )
{
    delete pFormatter;
    pFormatter = f;
}

inline QTextFormatter *QTextDocument::formatter() const
{
    return pFormatter;
}

inline void QTextDocument::setIndent( QTextIndent *i )
{
    indenter = i;
}

inline QTextIndent *QTextDocument::indent() const
{
    return indenter;
}

inline QColor QTextDocument::selectionColor( int id ) const
{
    return selectionColors[ id ];
}

inline bool QTextDocument::invertSelectionText( int id ) const
{
    return selectionText[ id ];
}

inline void QTextDocument::setSelectionColor( int id, const QColor &c )
{
    selectionColors[ id ] = c;
}

inline void QTextDocument::setInvertSelectionText( int id, bool b )
{
    selectionText[ id ] = b;
}

inline QTextFormatCollection *QTextDocument::formatCollection() const
{
    return fCollection;
}

inline int QTextDocument::alignment() const
{
    return align;
}

inline void QTextDocument::setAlignment( int a )
{
    align = a;
}

inline int *QTextDocument::tabArray() const
{
    return tArray;
}

inline int QTextDocument::tabStopWidth() const
{
    return tStopWidth;
}

inline void QTextDocument::setTabArray( int *a )
{
    tArray = a;
}

inline void QTextDocument::setTabStops( int tw )
{
    tStopWidth = tw;
}

inline QString QTextDocument::originalText() const
{
    if ( oTextValid )
	return oText;
    return text();
}

inline void QTextDocument::setFlow( QTextFlow *f )
{
    if ( flow_ )
	delete flow_;
    flow_ = f;
}

inline void QTextDocument::takeFlow()
{
    flow_ = 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QColor QTextFormat::color() const
{
    return col;
}

inline QFont QTextFormat::font() const
{
    return fn;
}

inline bool QTextFormat::isMisspelled() const
{
    return missp;
}

inline QTextFormat::VerticalAlignment QTextFormat::vAlign() const
{
    return ha;
}

inline bool QTextFormat::operator==( const QTextFormat &f ) const
{
    return k == f.k;
}

inline QTextFormatCollection *QTextFormat::parent() const
{
    return collection;
}

inline void QTextFormat::addRef()
{
    ref++;
#ifdef DEBUG_COLLECTION
    qDebug( "add ref of '%s' to %d (%p)", k.latin1(), ref, this );
#endif
}

inline void QTextFormat::removeRef()
{
    ref--;
    if ( !collection )
	return;
    if ( this == collection->defFormat )
	return;
#ifdef DEBUG_COLLECTION
    qDebug( "remove ref of '%s' to %d (%p)", k.latin1(), ref, this );
#endif
    if ( ref == 0 )
	collection->remove( this );
}

inline QString QTextFormat::key() const
{
    return k;
}

inline bool QTextFormat::useLinkColor() const
{
    return linkColor;
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextStringChar &QTextString::at( int i ) const
{
    return data[ i ];
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextStringChar *QTextParag::at( int i ) const
{
    return &str->at( i );
}

inline bool QTextParag::isValid() const
{
    return invalid == -1;
}

inline bool QTextParag::hasChanged() const
{
    return changed;
}

inline void QTextParag::setBackgroundColor( const QColor & c )
{
    delete bgcol;
    bgcol = new QColor( c );
    setChanged( TRUE );
}

inline void QTextParag::clearBackgroundColor()
{
    delete bgcol; bgcol = 0; setChanged( TRUE );
}

inline void QTextParag::append( const QString &s, bool reallyAtEnd )
{
    if ( reallyAtEnd )
	insert( str->length(), s );
    else
	insert( QMAX( str->length() - 1, 0 ), s );
}

inline QTextParag *QTextParag::prev() const
{
    return p;
}

inline QTextParag *QTextParag::next() const
{
    return n;
}

inline bool QTextParag::hasAnySelection() const
{
    return mSelections ? !selections().isEmpty() : FALSE;
}

inline void QTextParag::setEndState( int s )
{
    if ( s == state )
	return;
    state = s;
}

inline int QTextParag::endState() const
{
    return state;
}

inline void QTextParag::setParagId( int i )
{
    id = i;
}

inline int QTextParag::paragId() const
{
    if ( id == -1 )
	qWarning( "invalid parag id!!!!!!!! (%p)", (void*)this );
    return id;
}

inline bool QTextParag::firstPreProcess() const
{
    return firstPProcess;
}

inline void QTextParag::setFirstPreProcess( bool b )
{
    firstPProcess = b;
}

inline QMap<int, QTextParagLineStart*> &QTextParag::lineStartList()
{
    return lineStarts;
}

inline QTextString *QTextParag::string() const
{
    return str;
}

inline QTextDocument *QTextParag::document() const
{
    if ( hasdoc )
	return (QTextDocument*) docOrPseudo;
    return 0;
}

inline QTextParagPseudoDocument *QTextParag::pseudoDocument() const
{
    if ( hasdoc )
	return 0;
    return (QTextParagPseudoDocument*) docOrPseudo;
}


inline QTextTableCell *QTextParag::tableCell() const
{
    return hasdoc ? document()->tableCell () : 0;
}

inline QTextCommandHistory *QTextParag::commands() const
{
    return hasdoc ? document()->commands() : pseudoDocument()->commandHistory;
}


inline void QTextParag::setAlignment( int a )
{
    if ( a == (int)align )
	return;
    align = a;
    invalidate( 0 );
}

inline void QTextParag::setListStyle( QStyleSheetItem::ListStyle ls )
{
    listS = ls;
    invalidate( 0 );
}

inline QStyleSheetItem::ListStyle QTextParag::listStyle() const
{
    return listS;
}

inline QTextFormat *QTextParag::paragFormat() const
{
    return defFormat;
}

#ifndef QT_NO_TEXTCUSTOMITEM
inline void QTextParag::registerFloatingItem( QTextCustomItem *i )
{
    floatingItems().append( i );
}

inline void QTextParag::unregisterFloatingItem( QTextCustomItem *i )
{
    floatingItems().removeRef( i );
}

#endif

inline QBrush *QTextParag::background() const
{
#ifndef QT_NO_TEXTCUSTOMITEM
    return tableCell() ? tableCell()->backGround() : 0;
#else
    return 0;
#endif
}

inline int QTextParag::documentWidth() const
{
    return hasdoc ? document()->width() : pseudoDocument()->docRect.width();
}

inline int QTextParag::documentVisibleWidth() const
{
    return hasdoc ? document()->visibleWidth() : pseudoDocument()->docRect.width();
}

inline int QTextParag::documentX() const
{
    return hasdoc ? document()->x() : pseudoDocument()->docRect.x();
}

inline int QTextParag::documentY() const
{
    return hasdoc ? document()->y() : pseudoDocument()->docRect.y();
}

inline void QTextParag::setExtraData( QTextParagData *data )
{
    eData = data;
}

inline QTextParagData *QTextParag::extraData() const
{
    return eData;
}

inline void QTextParag::setNewLinesAllowed( bool b )
{
    newLinesAllowed = b;
}

inline bool QTextParag::isNewLinesAllowed() const
{
    return newLinesAllowed;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void QTextFormatCollection::setDefaultFormat( QTextFormat *f )
{
    defFormat = f;
}

inline QTextFormat *QTextFormatCollection::defaultFormat() const
{
    return defFormat;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline QTextFormat *QTextStringChar::format() const
{
    return (type == Regular) ? d.format : d.custom->format;
}


#ifndef QT_NO_TEXTCUSTOMITEM
inline QTextCustomItem *QTextStringChar::customItem() const
{
    return isCustom() ? d.custom->custom : 0;
}
#endif

inline int QTextStringChar::height() const
{
#ifndef QT_NO_TEXTCUSTOMITEM
    return !isCustom() ? format()->height() : ( customItem()->placement() == QTextCustomItem::PlaceInline ? customItem()->height : 0 );
#else
    return format()->height();
#endif
}

inline int QTextStringChar::ascent() const
{
#ifndef QT_NO_TEXTCUSTOMITEM
    return !isCustom() ? format()->ascent() : ( customItem()->placement() == QTextCustomItem::PlaceInline ? customItem()->ascent() : 0 );
#else
    return format()->ascent();
#endif
}

inline int QTextStringChar::descent() const
{
#ifndef QT_NO_TEXTCUSTOMITEM
    return !isCustom() ? format()->descent() : 0;
#else
    return format()->descent();
#endif
}

#endif //QT_NO_RICHTEXT

#endif
