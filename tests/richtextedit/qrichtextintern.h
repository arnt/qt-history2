/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qrichtextintern.h#7 $
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
#include "qformatstuff.h"

class QtTextContainer;
class QtTextBox;
class QtTextIterator;
class QtRichText;
class QtTextRow;
class QtTextCustomItem;


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
    friend class QtTextRow;
    struct Item {
	Item() {
	    format = 0;
	    width = -1;
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
    const QBrush* paper;
    QColor linkColor;
    bool linkUnderline;
};


class QtBox
{
public:
    QtBox( QtBox* p, QtTextFormatCollection* formatCol, const QtTextCharFormat& fmt,
	   const QStyleSheetItem *stl, const QMap<QString, QString> &attr );

    QtBox( QtBox* p, QtTextFormatCollection* formatCol, const QtTextCharFormat& fmt,
	   const QStyleSheetItem *stl );

    ~QtBox();

    void draw(QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion,
	      const QColorGroup& cg, const QtTextOptions& ,
	      bool onlyDirty = FALSE, bool onlySelection = FALSE);
    void setWidth (QPainter* p, QFontMetrics& fm, int newWidth, bool forceResize = FALSE);

    QtBox* parent;
    QtTextFormatCollection* formats;
    QtTextCharFormat format;
    QtTextRichString text;
    const QStyleSheetItem* style;
    QMap<QString, QString> attributes_;
    QtBox* child;
    QtBox* next;

    QList<QtTextRow> rows;

    inline QMap<QString, QString> attributes()  const
    {
	return attributes_;
    }

    int width;
    int widthUsed;
    int height;

    inline int margin(QStyleSheetItem::Margin m) const
    {
	if (style->margin(m) != QStyleSheetItem::Undefined)
	    return style->margin(m);
	return 0;
    }

    inline QStyleSheetItem::WhiteSpaceMode  whiteSpaceMode() const
    {
	if ( style->whiteSpaceMode() != QStyleSheetItem::WhiteSpaceNormal )
	    return style->whiteSpaceMode();
	return parent?parent->whiteSpaceMode():QStyleSheetItem::WhiteSpaceNormal;
    }

    int numberOfSubBox( QtBox* subbox, bool onlyListItems);
    QStyleSheetItem::ListStyle listStyle();
    inline int alignment() const
    {
	if ( style->alignment() != QStyleSheetItem::Undefined )
	    return style->alignment();
	return parent?parent->alignment():QStyleSheetItem::AlignLeft;
    }

};




// internal class for qmlbox, also used in qmlcursor.
class QtTextRow
{
public:
    QtTextRow();
    QtTextRow( QPainter* p, QFontMetrics &fm,
	       QtBox* b, int w, int& min, int align);
    QtTextRow( QPainter* p, QFontMetrics &fm,
	       const QtTextRichString* t, int &index, int w, int& min, int align);
    ~QtTextRow();
    int x;
    int y;
    int width;
    int height;
    int base;
    int fill;
    bool intersects(int xr, int yr, int wr, int hr);
    void draw(QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions&,
	      bool onlyDirty = FALSE, bool onlySelection = FALSE);
//     QtTextNode* hitTest(QPainter* p, int obx, int oby, int xarg, int yarg);


//     bool locate(QPainter* p, QtTextNode* node, int &lx, int &ly, int &lh);

    bool dirty;

    QtBox* box;
    const QtTextRichString* text;
    int first;
    int last;
};


inline bool QtTextRow::intersects(int xr, int yr, int wr, int hr)
{
    return ( QMAX( x, xr ) <= QMIN( x+width, xr+wr ) &&
	     QMAX( y, yr ) <= QMIN( y+height, yr+hr ) );

}



/*

class QtTextMulticol : public QtTextContainer
{
private:
    int ncols;
public:
    QtTextMulticol( const QStyleSheetItem *stl)
	: QtTextContainer(stl)
	{
	    ncols = 1;
	}
    QtTextMulticol( const QStyleSheetItem *stl, const QMap<QString, QString> &attr )
	: QtTextContainer(stl, attr)
	{
	    if ( attr.contains("cols") )
		ncols =  attr["cols"].toInt();
	    ncols = QMAX( 1, ncols);
	}

    ~QtTextMulticol()
	{
	}

    int numberOfColumns() const
	{
	    return ncols;
	}
};

*/

class QtTextCustomItem : public Qt
{
public:
    QtTextCustomItem()
	: reg(0),width(0), height(0)
    {}
    virtual ~QtTextCustomItem() {}
    virtual void draw(QPainter* p, int x, int y,
		      int ox, int oy, int cx, int cy, int cw, int ch,
		      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to) = 0;
    QRegion* reg;
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
    QPixmap pm;
};


class QtRichText : public QtBox
{
public:
    QtRichText( const QString &doc, const QFont& fnt = QApplication::font(),
	       const QString& context = QString::null,
	       int margin = 8, const QMimeSourceFactory* factory = 0, const QtStyleSheet* sheet = 0 );
    ~QtRichText();


    bool isValid() const;

    void setWidth (QPainter* p, int newWidth );

    QString context() const;
    void dump();

private:
    void init( const QString& doc, const QFont& fnt, int margin = 8 );

    bool parse (QtBox* current, const QStyleSheetItem* cursty, QtBox* dummy,
		QtTextCharFormat fmt, const QString& doc, int& pos);
    bool eatSpace(const QString& doc, int& pos, bool includeNbsp = FALSE );
    bool eat(const QString& doc, int& pos, QChar c);
    bool lookAhead(const QString& doc, int& pos, QChar c);
    QString parseOpenTag(const QString& doc, int& pos, QMap<QString, QString> &attr, bool& emptyTag);
    bool eatCloseTag(const QString& doc, int& pos, const QString& open);
    QChar parseHTMLSpecialChar(const QString& doc, int& pos);
    QString parseWord(const QString& doc, int& pos, bool insideTag = FALSE, bool lower = FALSE);
    QString parsePlainText(const QString& doc, int& pos, bool pre, bool justOneWord);
    bool hasPrefix(const QString& doc, int pos, QChar c);
    bool hasPrefix(const QString& doc, int pos, const QString& s);
    bool valid;
    QString contxt;
    const QtStyleSheet* sheet_;
    const QMimeSourceFactory* factory_;
    QStyleSheetItem* base;
    QStyleSheetItem* nullstyle;

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


inline bool QtTextRichString::haveSameFormat( int index1, int index2 ) const
{
    return items[index1].format == items[index2].format;
}

