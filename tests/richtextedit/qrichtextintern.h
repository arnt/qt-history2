/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qrichtextintern.h#5 $
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


class QtTextRichString : public QString
{
public:
    QtTextRichString( QtTextFormatCollection* fmt );
    ~QtTextRichString();

    int length() const;
    void remove( int index, int len );
    void insert( int index, const QChar& c, QtTextCharFormat fmt );
    inline void append( const QChar& c, QtTextCharFormat fmt )
    {
	insert( length(), c, fmt);
    }
    void clear() {
	QString::operator=(QString::null);
    }

    QChar charAt( int index ) const;
    QtTextCharFormat formatAt( int index ) const;

    bool haveSameFormat( int index1, int index2 ) const;

    bool isCustomItem( int index ) const;
    QtTextCustomItem* customItemAt( int index ) const;

private:
    ushort formatIndexAt( int index ) const;
    QtTextFormatCollection* format;
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
    QtBox( QtBox* p, QtTextFormatCollection* formatCol,
	   const QStyleSheetItem *stl, const QMap<QString, QString> &attr )
    : parent( p ), formats( formatCol ), text( formats ), style ( stl ), attributes_( attr )
    {
	boxes.setAutoDelete( TRUE );
	rows.setAutoDelete( TRUE );
	width = widthUsed = height = 0;
    };

    QtBox( QtBox* p, QtTextFormatCollection* formatCol,
	   const QStyleSheetItem *stl )
    : parent( p ), formats( formatCol ), text( formats ), style ( stl )
    {
	boxes.setAutoDelete( TRUE );
	rows.setAutoDelete( TRUE );
	width = widthUsed = height = 0;
    };

    void draw(QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion,
	      const QColorGroup& cg, const QtTextOptions& ,
	      bool onlyDirty = FALSE, bool onlySelection = FALSE);
    void setWidth (QPainter* p, int newWidth, bool forceResize = FALSE);

    QtBox* parent;
    QtTextFormatCollection* formats;
    QtTextRichString text;
    const QStyleSheetItem* style;
    QMap<QString, QString> attributes_;

    QList<QtBox> boxes;
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

    int numberOfSubBox( QtBox* subbox, bool onlyListItems)
    {
	return 1;
    }
    QStyleSheetItem::ListStyle listStyle();
    inline int alignment() const
    {
	if ( style->alignment() != QStyleSheetItem::Undefined )
	    return style->alignment();
	return parent?parent->alignment():QStyleSheetItem::AlignLeft;
    }

};


class QtTextNode
{
public:
    QtTextNode();
    ~QtTextNode();
    QtTextNode* next;

    inline QtTextNode* depthFirstSearch(QtTextNode* tag, QtTextContainer* &parent, bool down = TRUE);
    inline QtTextNode* nextLayout(QtTextNode* tag, QtTextContainer* &parent);
    inline QtTextNode* nextLeaf(QtTextNode* tag, QtTextContainer* &parent);

    QString text;

    inline bool isSpace() const {return text[0] == ' ';}
    inline bool isNewline() const {return text[0] == '\n';}
    inline bool isNull() const {return text.isNull();}


    inline QtRichText* root() const;
    inline QtTextContainer* parent() const;
    inline QtTextBox* box() const;
    inline QtTextNode* previousSibling() const;
    inline QtTextNode* lastSibling() const;
    inline QtTextNode* nextSibling() const;
    inline QtTextNode* nextNode() const;

    uint isSimpleNode: 1;
    uint isLastSibling:1;
    uint isContainer:1;
    uint isBox:1;
    uint isRoot:1;
    uint isSelected: 1;
    uint isSelectionDirty: 1;

    inline bool isCustomNode() const { return !isSimpleNode && !isContainer; }
};


class QtTextCustomNode : public QtTextNode
{
public:
    QtTextCustomNode();
    virtual ~QtTextCustomNode();

    virtual void draw(QPainter* p, int x, int y,
		      int ox, int oy, int cx, int cy, int cw, int ch,
		      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& ) = 0;

    virtual bool expandsHorizontally();

    int width;
    int height;
};

class QtTextHorizontalLine : public QtTextCustomNode
{
public:
    QtTextHorizontalLine(const QMap<QString, QString> &attr, const QMimeSourceFactory& factory);
    ~QtTextHorizontalLine();

    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& );

    bool expandsHorizontally();

};



class QtTextImage : public QtTextCustomNode
{
public:
    QtTextImage(const QMap<QString, QString> &attr, const QString& context,
	       const QMimeSourceFactory& factory);
    ~QtTextImage();

    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& );
private:
    QPixmap pm;
    QRegion* reg;
};


// internal class for qmlbox, also used in qmlcursor.
class QtTextRow
{
public:
    QtTextRow();
    QtTextRow( QPainter* p, QFontMetrics &fm,
	       QtBox* b, int w, int& min, int align);
    QtTextRow( QPainter* p, QFontMetrics &fm,
	       QtTextRichString* t, int &index, int w, int& min, int align);
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
    QtTextRichString* text;
    int first;
    int last;
};


inline bool QtTextRow::intersects(int xr, int yr, int wr, int hr)
{
    return ( QMAX( x, xr ) <= QMIN( x+width, xr+wr ) &&
	     QMAX( y, yr ) <= QMIN( y+height, yr+hr ) );

}

class QtTextContainer : public QtTextNode
{
public:
    QtTextContainer( const QStyleSheetItem *stl);
    QtTextContainer( const QStyleSheetItem *stl, const QMap<QString, QString> &attr );
    virtual ~QtTextContainer();
    inline QFont font() const;
    void setFont( const QFont& );
    void setFontSize( int );
    int fontSize() const;
    inline QColor color(const QColor&) const;
    void setColor( const QColor& );
    inline int margin(QStyleSheetItem::Margin) const;
    inline QStyleSheetItem::WhiteSpaceMode  whiteSpaceMode() const;
    virtual int numberOfColumns() const;
    inline int alignment() const;

    virtual void setParent( QtTextContainer* );
    QtTextContainer* parent;
    const QStyleSheetItem* style;
    QtTextNode* child;

    QtTextBox* box() const;
    QtTextBox* parentBox() const;

    QtTextIterator begin() const;
    QtTextIterator end() const;

    QtTextNode* lastChild() const;

    void reparentSubtree();

    virtual QtTextContainer* copy() const;

    void split(QtTextNode* node);

    const QMap<QString, QString> *attributes() const;

    const QtTextContainer* anchor() const;

    QtTextContainer* findAnchor(const QString& name ) const;

protected:
    void setAttributes(const QMap<QString, QString> &attr );

private:
    int fontWeight() const;
    bool fontItalic() const;
    bool fontUnderline() const;
    QString fontFamily() const;

    void createFont();

    QFont* fnt;
    int fontsize;
    QColor col;
    QMap<QString, QString> * attributes_;
};

class QtTextIterator
{
public:
    QtTextIterator() { node = 0; par = 0; }
    inline QtTextIterator( const QtTextNode* n );
    inline QtTextIterator( const QtTextNode* n, const QtTextContainer* p );
    QtTextIterator( const QtTextIterator& it) { node = it.node; par = it.par; }
    ~QtTextIterator();
    QtTextIterator next() const;
    bool operator==( const QtTextIterator& other ) const { return other.node == node; }
    bool operator!=( const QtTextIterator& other ) const { return other.node != node; }
    QtTextIterator operator++ (int);
    QtTextIterator& operator++ ();

    inline QtTextNode* operator*() const;
    inline QtTextNode* operator->() const;
    inline QtTextContainer* parentNode() const;
    inline QtTextIterator parent() const;

protected:
    QtTextContainer* par;
    QtTextNode* node;
};





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

class QtTextFont : public QtTextContainer
{
public:
    QtTextFont( const QStyleSheetItem *stl);
    QtTextFont( const QStyleSheetItem *stl, const QMap<QString, QString> &attr );
    ~QtTextFont();

    void setParent( QtTextContainer* );
};





class QtRichText : public QtBox
{
public:
    QtRichText( const QString &doc, const QFont& fnt = QApplication::font(),
	       const QString& context = QString::null,
	       int margin = 8, const QMimeSourceFactory* factory = 0, const QtStyleSheet* sheet = 0 );
    ~QtRichText();


    bool isValid() const;

    QString context() const;
    void dump();

private:
    void init( const QString& doc, const QFont& fnt, int margin = 8 );

    bool parse (QtBox* current, QtBox* dummy, QtTextCharFormat fmt, const QString& doc, int& pos);
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

};

inline QtTextIterator::QtTextIterator( const QtTextNode* n )
{
    node = (QtTextNode*)n;
    if ( node )
	par = node->parent();
}

inline QtTextIterator::QtTextIterator( const QtTextNode* n, const QtTextContainer* p )
{
    node = (QtTextNode*) n;
    par = (QtTextContainer*)p;
    if (!par && node)
	par = node->parent();
}

inline QtTextNode* QtTextIterator::operator*() const
{
    return node;
}

inline QtTextNode* QtTextIterator::operator->() const
{
    return node;
}


inline QtTextContainer* QtTextIterator::parentNode() const
{
    return par;
}

inline QtTextIterator QtTextIterator::parent() const
{
    return QtTextIterator( par );
}



inline QFont QtTextContainer::font() const
{
    if (!fnt) {
	QtTextContainer* that = (QtTextContainer*) this;
	that->createFont();
    }
    return *fnt;
}


inline QColor QtTextContainer::color(const QColor& c) const
{
    if ( col.isValid() )
	return col;
    QColor sc = style->color();
    if ( sc.isValid() )
	return sc;
    return parent?parent->color(c):c;
}

inline int QtTextContainer::margin(QStyleSheetItem::Margin m) const
{
    if (style->margin(m) != QStyleSheetItem::Undefined)
	return style->margin(m);
    return 0;
    //return parent?parent->margin(m):0;

}


inline QStyleSheetItem::WhiteSpaceMode  QtTextContainer::whiteSpaceMode() const
{
    if ( style->whiteSpaceMode() != QStyleSheetItem::WhiteSpaceNormal )
	return style->whiteSpaceMode();
    return parent?parent->whiteSpaceMode():QStyleSheetItem::WhiteSpaceNormal;
}

inline int QtTextContainer::numberOfColumns() const
{
    if (style->numberOfColumns() != QStyleSheetItem::Undefined)
	return style->numberOfColumns();
    return (parent && !parent->isBox)?parent->numberOfColumns():1;

}


inline int QtTextContainer::alignment() const
{
    if ( style->alignment() != QStyleSheetItem::Undefined )
	return style->alignment();
    return parent?parent->alignment():QStyleSheetItem::AlignLeft;
}


/*!
  depthFirst traversal for the tag tree. Returns the next node
 */
inline QtTextNode* QtTextNode::nextNode() const
{
    if ( isContainer && ( ( const QtTextContainer *) this)-> child )
	return ( ( const QtTextContainer *) this)-> child;
    if ( next || !isLastSibling )
	return next;
    QtTextNode* i = next;
    while ( i && i->isLastSibling )
	i = i->next;
    if ( i )
	return i->next;
    return 0;
}


/*!
  depthFirstSearch traversal for the tag tree
 */
inline QtTextNode* QtTextNode::depthFirstSearch(QtTextNode* tag, QtTextContainer* &parent, bool down)
{
    if (down) {
	if (tag->isContainer && ((QtTextContainer*)tag)->child){
	    parent = (QtTextContainer*)tag;
	    return ((QtTextContainer*)tag)->child;
	}
	//  	return depthFirstSearch(tag, parent, FALSE);
    }
    //      else
    {
	if (tag == this){
	    return 0;
	}
	if (!tag->isLastSibling && tag->next){
	    return tag->next;
	}
	QtTextContainer* p = (QtTextContainer*)tag->next;
	if (p){
	    parent = p->parent;
	    return depthFirstSearch(p, parent, FALSE);
	}
    }
    return 0;
}


/*!
  extends the depthFirstSearch traversal so that only tags that include a layout are
  returned
*/

inline QtTextNode* QtTextNode::nextLayout(QtTextNode* tag, QtTextContainer* &parent){
    QtTextNode* t;

    if (tag != this && tag->isBox)
	t = depthFirstSearch(tag, parent, FALSE);
    else
	t = depthFirstSearch(tag, parent);
    if (t) {
	if (t->isContainer && !t->isBox)
	    return nextLayout(t, parent);
    }
    return t;
}


inline QtTextNode* QtTextNode::nextLeaf(QtTextNode* tag, QtTextContainer* &parent){
    do {
	tag = depthFirstSearch(tag, parent);

    } while (tag && tag->isContainer);

    return tag;
}



inline QtTextNode* QtTextNode::lastSibling() const
{
    QtTextNode* n = (QtTextNode*) this;

    while (n && !n->isLastSibling)
	n = n->next;
    return n;
}


inline QtRichText* QtTextNode::root() const
{
    if (isRoot)
	return ( QtRichText* ) this;
    if (isContainer)
	return ((QtTextContainer*)this)->parent->root();
    else {
	QtTextNode* n = lastSibling();
	if (n) return n->next->root();
    }
    return 0;
}

inline QtTextContainer* QtTextNode::parent() const
{
    if (isContainer)
	return ((QtTextContainer*)this)->parent;
    else {
	QtTextNode* n = lastSibling();
	if (n) return (QtTextContainer*)n->next;
    }
    return 0;
}

inline QtTextBox* QtTextNode::box() const
{
    QtTextContainer* par = parent();
    if (!par)
	return 0;
    else
	return par->box();
}

inline QtTextNode* QtTextNode::previousSibling() const
{
    QtTextContainer* par = parent();
    QtTextNode* result = par->child;
    if (result == this)
	return 0;
    while (result->next && result->next != this)
	result = result->next;
    return result;
}


inline QtTextNode* QtTextNode::nextSibling() const
{
    if (isLastSibling)
	return 0;
    return next;
}


class QtTextCursor{
public:
    QtTextCursor(QtRichText& doc);
    ~QtTextCursor();
    void draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch);

    QtRichText* document;

    int x;
    int y;
    int height;

    QtTextRow* row;
    int rowY;
    int rowHeight;

    int width() { return 1; }

    QtTextNode *node;
    QtTextContainer *nodeParent;

    bool hasSelection;
    bool selectionDirty;
    void clearSelection();

    void insert(QPainter* p, const QString& s);
    void enter(QPainter* p);
    void del(QPainter* p, int c = 1);
    void backSpace(QPainter* p, int c = 1);

    void right(QPainter* p, bool select = FALSE);
    void left(QPainter* p, bool select = FALSE);
    void up(QPainter* p, bool select = FALSE);
    void down(QPainter* p, bool select = FALSE);
    void home(QPainter* p, bool select = FALSE);
    void last(QPainter* p, bool select = FALSE);
    void goTo(QPainter* p, int xarg, int yarg, bool select = FALSE);


    void goTo(QtTextNode* n, QtTextContainer* par,  bool select = FALSE);
    void calculatePosition(QPainter* p);

    int xline;
    int yline;
    bool ylineOffsetClean;

private:
    void rightInternal(bool select = FALSE);
    void leftInternal(bool select = FALSE);
};
