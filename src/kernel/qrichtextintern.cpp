/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrichtextintern.cpp#17 $
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

#include <qmap.h>
#include <qdict.h>
#include <qcolor.h>
#include <qfont.h>
#include <qstring.h>


class QStyleSheetItem;
class QTextCustomItem;
class QTextFormatCollection;

class QTextCharFormat
{
    friend class QTextFormatCollection;

public:
    QTextCharFormat();
    QTextCharFormat( const QTextCharFormat &format );
    QTextCharFormat( const QFont &f, const QColor &c );
    QTextCharFormat &QTextCharFormat::operator=( const QTextCharFormat &fmt );
    bool operator==( const QTextCharFormat &format );
    virtual ~QTextCharFormat();

    QTextCharFormat makeTextFormat( const QStyleSheetItem *style, const QMap<QString,QString>& attr,
				     QTextCustomItem* item = 0) const;

    QColor color() const;
    QFont font() const;
    QString anchorHref() const;
    QString anchorName() const;

    bool isAnchor() const;

    QTextCharFormat formatWithoutCustom();

    int addRef();
    int removeRef();

    QTextCustomItem *customItem() const;

private:
    QFont font_;
    QColor color_;
    QString key;
    int ref;
    int logicalFontSize;
    int stdPointSize;
    QString anchor_href;
    QString anchor_name;
    void createKey();
    QTextFormatCollection* parent;
    QTextCustomItem* custom;
};


class QTextFormatCollection
{
    friend class QTextCharFormat;

public:
    QTextFormatCollection();

    QTextCharFormat*  registerFormat( const QTextCharFormat &format );
    void unregisterFormat( const QTextCharFormat &format  );

protected:
    QDict<QTextCharFormat > cKey;
    QTextCharFormat* lastRegisterFormat;
};


inline QColor QTextCharFormat::color() const
{
    return color_;
}

inline QFont QTextCharFormat::font() const
{
    return font_;
}

inline QString QTextCharFormat::anchorHref() const
{
    return anchor_href;
}

inline QString QTextCharFormat::anchorName() const
{
    return anchor_name;
}

inline QTextCustomItem * QTextCharFormat::customItem() const
{
    return custom;
}

inline bool QTextCharFormat::isAnchor() const
{
    return !anchor_href.isEmpty()  || !anchor_href.isEmpty();
}

class QRichText;
class QTextView;
class QTextFlow;

class QTextOptions {
public:
    QTextOptions( const QBrush* p = 0, QColor lc = Qt::blue, bool lu = TRUE )
	:paper( p ), linkColor( lc ), linkUnderline( lu ), offsetx( 0 ), offsety( 0 )
    {
    };
    const QBrush* paper;
    QColor linkColor;
    bool linkUnderline;
    int offsetx;
    int offsety;
    void erase( QPainter* p, const QRect& r ) const;
};

class QTextCustomItem : public Qt
{
public:
    QTextCustomItem()
	: width(-1), height(0)
    {}
    virtual ~QTextCustomItem() {}
    virtual void draw(QPainter* p, int x, int y,
		      int ox, int oy, int cx, int cy, int cw, int ch,
		      QRegion& backgroundRegion, const QColorGroup& cg, 
		      const QTextOptions& to ) = 0;

    virtual void realize( QPainter* ) { width = 0; }

    enum Placement { PlaceInline = 0, PlaceLeft, PlaceRight };
    virtual Placement placement() { return PlaceInline; }

    bool placeInline() { return placement() == PlaceInline; }

    virtual bool noErase() const { return FALSE; };
    virtual bool expandsHorizontally() const { return FALSE; }
    virtual bool ownLine() const { return expandsHorizontally(); };
    virtual void resize( QPainter*, int nwidth ){ width = nwidth; };

    virtual QString anchorAt( QPainter* /*p*/, int /*x*/, int /*y*/)  { return QString::null; }

    int y; // used for floating items
    int x; // used for floating items
    int width;
    int height;
};

class QTextHorizontalLine : public QTextCustomItem
{
public:
    QTextHorizontalLine();
    ~QTextHorizontalLine();
    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QTextOptions& to );

    bool expandsHorizontally() const { return TRUE; }
private:
};


class QTextRichString
{
    friend class QTextCursor;
    struct Item {
	Item() 
	: base(-1), width(-1),selected(0),newline(0),format(0)
	{
	};
// 	Item( const Item& other ) {
// 	    width = other.width;
// 	    selected = other.selected;
// 	    c = other.c;
// 	    format = other.format;
// 	}
// 	Item& operator=( const Item& other ) {
// 	    width = other.width;
// 	    selected = other.selected;
// 	    c = other.c;
// 	    format = other.format;
// 	    return *this;
// 	}
	~Item() {
	};
	int base;
	int width; // : 30;
	uint selected : 1;
	uint newline : 1;
	QTextCharFormat* format;
	QString c;
    };
    Item* items;
    int store;
    int len;
    bool selection;
public:
    QTextRichString( QTextFormatCollection* fmt );
    QTextRichString( const QTextRichString &other );
    QTextRichString& operator=( const QTextRichString &other );
    ~QTextRichString();

    int length() const;
    bool isEmpty() const;
    void remove( int index, int len );
    void insert( int index, const QString& c, const QTextCharFormat& fmt );
    void append( const QString& c,const  QTextCharFormat& fmt );
    void clear();

    QString charAt( int index ) const;
    QString& getCharAt( int index );
    QTextCharFormat *formatAt( int index ) const;
    bool haveSameFormat( int index1, int index2 ) const;

    bool isCustomItem( int index ) const;
    QTextCustomItem* customItemAt( int index ) const;

    void setSelected( int index, bool selected );
    bool isSelected( int index ) const;
    void clearSelection();
    bool isSelected() const;
    void setBold( int index, bool b );
    bool bold( int index ) const;

    QTextFormatCollection* formats; // make private
private:
    void setLength( int l );
};

class QTextParagraph
{
public:
    QTextParagraph( QTextParagraph* p, QTextFormatCollection* formatCol, const QTextCharFormat& fmt,
	   const QStyleSheetItem *stl, const QMap<QString, QString> &attr );

    QTextParagraph( QTextParagraph* p, QTextFormatCollection* formatCol, const QTextCharFormat& fmt,
	   const QStyleSheetItem *stl );

    ~QTextParagraph();

    QTextParagraph* realParagraph();
    QTextParagraph* parent;
    QTextFormatCollection* formats;
    QTextCharFormat format;
    QTextRichString text;
    const QStyleSheetItem* style;
    QMap<QString, QString> attributes_;
    QTextParagraph* child;
    QTextParagraph* prev;
    QTextParagraph* next;

    QTextParagraph* nextInDocument();
    QTextParagraph* prevInDocument();

    QTextParagraph* lastChild();

    inline QMap<QString, QString> attributes()  const
    {
	return attributes_;
    }

    int y;
    int height;
    bool dirty;
    bool selected;

    QTextFlow* flow();

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

    inline int totalLabelMargin() const
    {
	int tlm = parent? parent->totalLabelMargin() : 0;
	tlm += labelMargin();
	return tlm;
    }


    inline QStyleSheetItem::WhiteSpaceMode  whiteSpaceMode() const
    {
	if ( style->whiteSpaceMode() != QStyleSheetItem::WhiteSpaceNormal )
	    return style->whiteSpaceMode();
	return parent?parent->whiteSpaceMode():QStyleSheetItem::WhiteSpaceNormal;
    }

    int numberOfSubParagraph( QTextParagraph* subparagraph, bool onlyListItems);
    QStyleSheetItem::ListStyle listStyle();
    inline int alignment() const
    {
	if ( align != QStyleSheetItem::Undefined )
	    return align;
	if ( style->alignment() != QStyleSheetItem::Undefined )
	    return style->alignment();
	return parent?parent->alignment():QStyleSheetItem::AlignLeft;
    }

    void invalidateLayout();


private:
    void init();
    int align;
protected:
    QTextFlow* flow_;
};


class QTextImage : public QTextCustomItem
{
public:
    QTextImage(const QMap<QString, QString> &attr, const QString& context,
		       const QMimeSourceFactory &factory);
    ~QTextImage();

    Placement placement();

    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QTextOptions& to );
private:
    QRegion* reg;
    QPixmap pm;
    Placement place;
};

class QTextTable;

class QTextCursor {
 public:
    QTextCursor(QRichText& document );
    ~QTextCursor();


    QTextParagraph* paragraph;
    QTextFlow* flow;
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
    bool inLastLine() const;
    void gotoParagraph( QPainter* p, QTextParagraph* b );

    void initParagraph( QPainter* p, QTextParagraph* b );
    bool updateLayout( QPainter* p, int ymax = -1 );


    void makeLineLayout( QPainter* p, const QFontMetrics& fm );
    bool gotoNextLine( QPainter* p, const QFontMetrics& fm );
    void gotoLineStart( QPainter* p, const QFontMetrics& fm );
    void drawLine( QPainter* p, int ox, int oy,
		   int cx, int cy, int cw, int ch,
		   QRegion& backgroundRegion,
		   const QColorGroup& cg, const QTextOptions& to );
    void drawLabel( QPainter* p, QTextParagraph* par, int x, int y, int w, int h, int ox, int oy,
		   QRegion& backgroundRegion,
		   const QColorGroup& cg, const QTextOptions& to );
    void gotoNextItem( QPainter* p, const QFontMetrics& fm );

    void updateCharFormat( QPainter* p, const QFontMetrics& fm );

    int y() const { return y_; }
    int x() const { return currentx + currentoffsetx; }
    QTextCharFormat* currentFormat();
    int width;
    int widthUsed;
    int height;
    int base;
    int fill;
    int lmargin;
    int rmargin;

    int static_lmargin;
    int static_rmargin;
    int static_labelmargin;

    void draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch);
    QRect caretGeometry() const;
    QRect lineGeometry() const;
    void right( QPainter* p );
    void left( QPainter* p );
    void up( QPainter* p );
    void down( QPainter* p );
    void insert( QPainter*, const QString& text );
    bool split();

    void goTo( QPainter* p, int xpos, int ypos );

    bool rightOneItem( QPainter* p );
    QRichText* doc;
    int xline;
    QTextParagraph* xline_paragraph;
    int xline_current;

    void setSelected( bool selected );
    bool isSelected() const;

private:
    int y_;
    QTextCharFormat* formatinuse;

};

class QTextFlow {
public:

    QTextFlow();
    ~QTextFlow();

    void initialize( int w );

    int adjustLMargin( int yp, int margin );
    int adjustRMargin( int yp, int margin );


    void registerFloatingItem( QTextCustomItem* item, bool right = FALSE );
    void drawFloatingItems(QPainter* p,
			   int ox, int oy, int cx, int cy, int cw, int ch,
			   QRegion& backgroundRegion, const QColorGroup& cg, const QTextOptions& to );
    void adjustFlow( int  &yp, int w, int h, bool pages = TRUE );

    int width;
    int widthUsed;
    int height;
    
    int pagesize;

    QTextFlow* parent;

    QRect updateRect() { return updaterect; }
    void invalidateRect( const QRect& r ) {
	updaterect = updaterect.unite( r );
// 	qDebug("invalidate with %d, now %d", r.height(), updaterect.height() );
    }
    void validateRect() { updaterect = QRect(); }

private:
    QList<QTextCustomItem> leftItems;
    QList<QTextCustomItem> rightItems;
    QRect updaterect;

};


class QRichText : public QTextParagraph
{
public:
    QRichText( const QString &doc, const QFont& fnt = QApplication::font(),
		const QString& context = QString::null,
		int margin = 8, const QMimeSourceFactory* factory = 0, const QStyleSheet* sheet = 0 );
    QRichText( const QMap<QString, QString> &attr, const QString &doc, int& pos,
		const QStyleSheetItem* style, const QTextCharFormat& fmt,
		const QString& context = QString::null,
		int margin = 8, const QMimeSourceFactory* factory = 0, const QStyleSheet* sheet = 0 );
    ~QRichText();

    bool isValid() const;

    QString context() const;
    void dump();

    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QTextOptions& to );

    void doLayout( QPainter* p, int nwidth );
    QString anchorAt( QPainter* p, int x, int y ) const;
    bool clearSelection();
    QString selectedText();
    void selectAll();

    void append( const QString& txt, const QMimeSourceFactory* factory = 0, const QStyleSheet* sheet = 0 );

private:
    void init( const QString& doc, int& pos );

    bool parse (QTextParagraph* current, const QStyleSheetItem* cursty, QTextParagraph* dummy,
		QTextCharFormat fmt, const QString& doc, int& pos);

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
    const QStyleSheet* sheet_;
    QStyleSheetItem* base;
    const QMimeSourceFactory* factory_;
    const QStyleSheetItem* nullstyle;

    QTextCustomItem* parseTable( const QMap<QString, QString> &attr, const QTextCharFormat &fmt, const QString &doc, int& pos );

    bool keep_going;

};

inline void QTextRichString::append( const QString& c,const  QTextCharFormat& fmt )
{
	insert( length(), c, fmt);
}

inline int QTextRichString::length() const
{
    return len;
}

inline bool QTextRichString::isEmpty() const
{
    return len == 0;
}

inline QString QTextRichString::charAt( int index ) const
{
    return items[index].c;
}


inline QTextCharFormat *QTextRichString::formatAt( int index ) const
{
    return items[index].format;
}


inline QTextCustomItem* QTextRichString::customItemAt( int index ) const
{
    return items[index].format->customItem();
}


inline bool QTextRichString::isCustomItem( int index ) const
{
    return customItemAt( index ) != 0;
}


inline bool QTextRichString::haveSameFormat( int index1, int index2 ) const
{
    return items[index1].format == items[index2].format;
}

inline QTextCharFormat* QTextCursor::currentFormat()
{
    return paragraph->text.formatAt( current );
}



