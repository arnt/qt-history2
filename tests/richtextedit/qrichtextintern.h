/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qrichtextintern.h#16 $
**
** Internal rich text classes
**
** Created : 990124
**
** Copyright (C) 1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
#include "qstring.h"
#include "qmap.h"
#include "qstylesheet.h"
#include "qapplication.h"
#include "qmime.h"
#include "qabstractlayout.h"
#include "qlayout.h"
#include "qformatstuff.h"


class QtRichText;
class QtTextCustomItem;
class QtTextView;
class QtTextFlow;

class QtStyleSheet : public QStyleSheet
{
public:
    QtStyleSheet( QObject *parent=0, const char *name=0 );
    ~QtStyleSheet();
    QtTextCustomItem* tagEx( const QString& name,
			   const QMap<QString, QString> &attr,
			   const QString& context,
			   const QMimeSourceFactory& factory,
			   bool emptyTag = FALSE) const;
};


class QtTextRichString
{
    friend class QtTextCursor;
    struct Item {
	Item() {
	    format = 0;
	    width = -1;
	};
	~Item() {
	};
	Item( const Item& other) {
	    c = other.c;
	    format = other.format;
	    width = other.width;
	};
	Item& operator=( const Item& other) {
	    c = other.c;
	    format = other.format;
	    width = other.width;
	    return *this;
	};
	int width;
	QString c;
	QtTextCharFormat* format;
    };
    Item* items;
    int store;
    int len;
public:
    QtTextRichString( QtTextFormatCollection* fmt );
    QtTextRichString( const QtTextRichString &other );
    QtTextRichString& operator=( const QtTextRichString &other );
    ~QtTextRichString();

    inline int length() const;
    inline bool isEmpty() const;
    void remove( int index, int len );
    void insert( int index, const QString& c, const QtTextCharFormat& fmt );
    inline void append( const QString& c,const  QtTextCharFormat& fmt );
    void clear();

    inline QString charAt( int index ) const;
    QString& getCharAt( int index );
    inline QtTextCharFormat *formatAt( int index ) const;
    inline bool haveSameFormat( int index1, int index2 ) const;

    inline bool isCustomItem( int index ) const;
    inline QtTextCustomItem* customItemAt( int index ) const;

    QtTextFormatCollection* format; // make private
private:
    void setLength( int l );
};

class QtTextOptions {
public:
    QtTextOptions( const QBrush* p = 0, QColor lc = Qt::blue, bool lu = TRUE )
	:paper( p ), linkColor( lc ), linkUnderline( lu )
    {
    };
    QtTextOptions( const QtTextOptions& other )
    {
	paper = other.paper;
	linkColor = other.linkColor;
	linkUnderline = other.linkUnderline;
    }
    const QBrush* paper;
    QColor linkColor;
    bool linkUnderline;
};

class QtTextParagraph
{
public:
    QtTextParagraph( QtTextParagraph* p, QtTextFormatCollection* formatCol, const QtTextCharFormat& fmt,
	   const QStyleSheetItem *stl, const QMap<QString, QString> &attr );

    QtTextParagraph( QtTextParagraph* p, QtTextFormatCollection* formatCol, const QtTextCharFormat& fmt,
	   const QStyleSheetItem *stl );

    ~QtTextParagraph();

    QtTextParagraph* realParagraph();
    QtTextParagraph* parent;
    QtTextFormatCollection* formats;
    QtTextCharFormat format;
    QtTextRichString text;
    const QStyleSheetItem* style;
    QMap<QString, QString> attributes_;
    QtTextParagraph* child;
    QtTextParagraph* prev;
    QtTextParagraph* next;

    QtTextParagraph* nextInDocument();
    QtTextParagraph* prevInDocument();


    inline QMap<QString, QString> attributes()  const
    {
	return attributes_;
    }

    int y;
    int height;
    bool dirty;

    QtTextFlow* flow();

    inline int margin(QStyleSheetItem::Margin m) const
    {
	if (style->margin(m) != QStyleSheetItem::Undefined)
	    return style->margin(m);
	return 0;
    }

    inline int topMargin() const
    {
	int m = margin( QStyleSheetItem::MarginTop );
	if ( !prev && parent )
	    m += parent->topMargin();
	return m;
    }

    inline int bottomMargin() const
    {
	int m = margin( QStyleSheetItem::MarginBottom );
	if ( !next && parent )
	    m += parent->bottomMargin();
	return m;
    }

    inline int labelMargin() const
    {
	return style->displayMode() == QStyleSheetItem::DisplayListItem ? 25: 0;
    }

    inline int totalMargin(QStyleSheetItem::Margin m) const
    {
	int tm = parent? parent->totalMargin( m ) : 0;
	if (style->margin(m) != QStyleSheetItem::Undefined)
	    tm += style->margin(m);
	 if ( m == QStyleSheetItem::MarginLeft )
	     tm += labelMargin();
	return tm;
    }


    inline QStyleSheetItem::WhiteSpaceMode  whiteSpaceMode() const
    {
	if ( style->whiteSpaceMode() != QStyleSheetItem::WhiteSpaceNormal )
	    return style->whiteSpaceMode();
	return parent?parent->whiteSpaceMode():QStyleSheetItem::WhiteSpaceNormal;
    }

    int numberOfSubParagraph( QtTextParagraph* subparagraph, bool onlyListItems);
    QStyleSheetItem::ListStyle listStyle();
    inline int alignment() const
    {
	if ( style->alignment() != QStyleSheetItem::Undefined )
	    return style->alignment();
	return parent?parent->alignment():QStyleSheetItem::AlignLeft;
    }

    void invalidateLayout();


private:
    void init();
protected:
    QtTextFlow* flow_;
};


class QtTextCustomItem : public Qt
{
public:
    QtTextCustomItem()
	: width(-1), height(0)
    {}
    virtual ~QtTextCustomItem() {}
    virtual void draw(QPainter* p, int x, int y,
		      int ox, int oy, int cx, int cy, int cw, int ch,
		      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to) = 0;

    virtual void realize( QPainter* ) { width = 0; }

    virtual bool noErase() const { return FALSE; };
    virtual bool expandsHorizontally() const { return FALSE; }
    virtual void resize( QPainter*, int nwidth ){ width = nwidth; };
    int width;
    int height;
};

class QtTextImage : public QtTextCustomItem
{
public:
    QtTextImage(const QMap<QString, QString> &attr, const QString& context,
		       const QMimeSourceFactory &factory);
    ~QtTextImage();
    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to);
private:
    QRegion* reg;
    QPixmap pm;
};

class QtTextTable;
class QtTextTableCell : public QLayoutItem
{
public:
    QtTextTableCell(QtTextTable* table,
      int row, int column, 		
      const QMap<QString, QString> &attr, 
      const QStyleSheetItem* style, 
      const QtTextCharFormat& fmt, const QString& context,
      const QMimeSourceFactory &factory, const QtStyleSheet *sheet, const QString& doc, int& pos );
    ~QtTextTableCell();
    QSize sizeHint() const ;
    QSize minimumSize() const ;
    QSize maximumSize() const ;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry( const QRect& ) ;
    QRect geometry() const;

    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;

    void realize();

    int row() const { return row_; }
    int column() const { return col_; }
    int rowspan() const { return rowspan_; }
    int colspan() const { return colspan_; }

    void draw( int x, int y,
	       int ox, int oy, int cx, int cy, int cw, int ch,
	       QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to);
private:

    QPainter* painter() const;
    QRect geom;
    QtTextTable* parent;
    QtRichText* richtext;
    QBrush* background;
    int row_;
    int col_;
    int rowspan_;
    int colspan_;
    int maxw;
    int minw;
};

class QtTextTable: public QtTextCustomItem
{
public:
    QtTextTable(const QMap<QString, QString> &attr);
    ~QtTextTable();
    void realize( QPainter* );
    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to);

    bool noErase() const { return TRUE; };
    bool expandsHorizontally() const { return TRUE; }
    void resize( QPainter*, int nwidth );

private:
    QGridLayout* layout;
    QList<QtTextTableCell> cells;

    friend class QtTextTableCell;
    void addCell( QtTextTableCell* cell );
    QPainter* painter;
    int cachewidth;
};


class QtTextHorizontalLine : public QtTextCustomItem
{
public:
    QtTextHorizontalLine();
    ~QtTextHorizontalLine();
    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to);

    bool expandsHorizontally() const { return TRUE; }
private:
};

class QtTextCursor {
 public:
    QtTextCursor(QtRichText& document );
    ~QtTextCursor();


    QtTextParagraph* paragraph;
    QtTextFlow* flow;
    void update( QPainter* p );
    void updateParagraph( QPainter* );
    int first;
    int last;
    int current;
    int currentx;
    int currentasc;
    int currentdesc;
    int currentoffset;
    int currentoffsetx;
    bool atEnd() const;
    bool pastEnd() const;
    bool atEndOfLine() const;
    bool pastEndOfLine() const;
    void gotoParagraph( QPainter* p, QtTextParagraph* b );

    void initFlow( QtTextFlow* frm, int w  );
    void initParagraph( QPainter* p, QtTextParagraph* b );
    bool doLayout( QPainter* p, int ymax = -1, QtTextFlow* backFlow = 0 );


    void makeLineLayout( QPainter* p, const QFontMetrics& fm );
    bool gotoNextLine( QPainter* p, const QFontMetrics& fm );
    void gotoLineStart( QPainter* p, const QFontMetrics& fm );
    void drawLine( QPainter* p, int ox, int oy,
		   int cx, int cy, int cw, int ch,
		   QRegion& backgroundRegion,
		   const QColorGroup& cg, const QtTextOptions& to );
    void drawLabel( QPainter* p, QtTextParagraph* par, int x, int y, int w, int h, int ox, int oy,
		   QRegion& backgroundRegion,
		   const QColorGroup& cg, const QtTextOptions& to );
    void gotoNextItem( QPainter* p, const QFontMetrics& fm );

    void updateCharFormat( QPainter* p, const QFontMetrics& fm );

    int y_;
    int y() const { return y_; }
    QtTextCharFormat* currentFormat();
    int width;
    int widthUsed;
    int height;
    int base;
    int fill;
    int lmargin;
    int rmargin;


    bool adjustFlowMode;

    void draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch);
    QRect caretGeometry() const;
    QRect lineGeometry() const;
    void right( QPainter* p );
    void left( QPainter* p );
    void up( QPainter* p );
    void down( QPainter* p );
    void insert( QPainter*, const QString& text );

    bool rightOneItem( QPainter* p );
    QtRichText* doc;
    int xline;
    QtTextParagraph* xline_paragraph;
    int xline_current;

    int referenceTop();
    int referenceBottom();
};

class QtTextFlow {
public:

    QtTextFlow( QtTextFlow* parentFlow = 0, int ncolumns  = 1 );
    ~QtTextFlow();

    void initialize( int w );

    void mapToView( int yp, int& gx, int& gy );
    int availableWidth( int yp );

    void countFlow( int yp, int w, int h, bool pages = TRUE  );
    void adjustFlow( int  &yp, int w, int h, bool pages = TRUE );

    int x;
    int y;
    int width;
    int widthUsed;
    int height;

    int ncols;
    int colwidth;
    int colheight;
    int totalheight;
    int additional_height;

    QtTextFlow* parent;
};


class QtRichText : public QtTextParagraph
{
public:
    QtRichText( const QString &doc, const QFont& fnt = QApplication::font(),
		const QString& context = QString::null,
		int margin = 8, const QMimeSourceFactory* factory = 0, const QtStyleSheet* sheet = 0 );
    QtRichText( const QMap<QString, QString> &attr, const QString &doc, int& pos, 
		const QStyleSheetItem* style, const QtTextCharFormat& fmt,
		const QString& context = QString::null,
		int margin = 8, const QMimeSourceFactory* factory = 0, const QtStyleSheet* sheet = 0 );
    ~QtRichText();

    bool isValid() const;

    QString context() const;
    void dump();

    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to);

    void doLayout( QPainter* p, int nwidth );



private:
    void init( const QString& doc, int& pos );

    bool parse (QtTextParagraph* current, const QStyleSheetItem* cursty, QtTextParagraph* dummy,
		QtTextCharFormat fmt, const QString& doc, int& pos);
    bool eatSpace(const QString& doc, int& pos, bool includeNbsp = FALSE );
    bool eat(const QString& doc, int& pos, QChar c);
    bool lookAhead(const QString& doc, int& pos, QChar c);
    QString parseOpenTag(const QString& doc, int& pos, QMap<QString, QString> &attr, bool& emptyTag);
    QString parseCloseTag( const QString& doc, int& pos );
    bool eatCloseTag(const QString& doc, int& pos, const QString& open);
    QChar parseHTMLSpecialChar(const QString& doc, int& pos);
    QString parseWord(const QString& doc, int& pos, bool insideTag = FALSE, bool lower = FALSE);
    QString parsePlainText(const QString& doc, int& pos, bool pre, bool justOneWord);
    bool hasPrefix(const QString& doc, int pos, QChar c);
    bool hasPrefix(const QString& doc, int pos, const QString& s);
    bool valid;
    QString contxt;
    const QtStyleSheet* sheet_;
    QStyleSheetItem* base;
    const QMimeSourceFactory* factory_;
    const QStyleSheetItem* nullstyle;

    QtTextCustomItem* parseTable( const QMap<QString, QString> &attr, const QtTextCharFormat &fmt, const QString &doc, int& pos );


};

inline void QtTextRichString::append( const QString& c,const  QtTextCharFormat& fmt )
{
	insert( length(), c, fmt);
}

inline int QtTextRichString::length() const
{
    return len;
}

inline bool QtTextRichString::isEmpty() const
{
    return len == 0;
}

inline QString QtTextRichString::charAt( int index ) const
{
    return items[index].c;
}


inline QtTextCharFormat *QtTextRichString::formatAt( int index ) const
{
    return items[index].format;
}


inline QtTextCustomItem* QtTextRichString::customItemAt( int index ) const
{
    return items[index].format->customItem();
}


inline bool QtTextRichString::isCustomItem( int index ) const
{
    return customItemAt( index ) != 0;
}


inline bool QtTextRichString::haveSameFormat( int index1, int index2 ) const
{
    return items[index1].format == items[index2].format;
}

inline QtTextCharFormat* QtTextCursor::currentFormat()
{
    return paragraph->text.formatAt( current );
}
