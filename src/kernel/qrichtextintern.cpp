/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrichtextintern.cpp#16 $
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

class QTextContainer;
class QTextBox;
class QTextIterator;
class QRichText;

class QTextOptions {
public:
    QTextOptions( const QBrush* p = 0, QColor lc = Qt::blue, bool lu = TRUE )
	:paper( p ), linkColor( lc ), linkUnderline( lu )
    {
    };
    const QBrush* paper;
    QColor linkColor;
    bool linkUnderline;
};

class QTextNode
{
public:
    QTextNode();
    ~QTextNode();
    QTextNode* next;

    inline QTextNode* depthFirstSearch(QTextNode* tag, QTextContainer* &parent, bool down = TRUE);
    inline QTextNode* nextLayout(QTextNode* tag, QTextContainer* &parent);
    inline QTextNode* nextLeaf(QTextNode* tag, QTextContainer* &parent);

    QString text;

    inline bool isSpace() const {return text[0] == ' ';}
    inline bool isNewline() const {return text[0] == '\n';}
    inline bool isNull() const {return text.isNull();}


    inline QRichText* root() const;
    inline QTextContainer* parent() const;
    inline QTextBox* box() const;
    inline QTextNode* previousSibling() const;
    inline QTextNode* lastSibling() const;
    inline QTextNode* nextSibling() const;
    inline QTextNode* nextNode() const;

    uint isSimpleNode: 1;
    uint isLastSibling:1;
    uint isContainer:1;
    uint isBox:1;
    uint isRoot:1;
    uint isSelected: 1;
    uint isSelectionDirty: 1;

    inline bool isCustomNode() const { return !isSimpleNode && !isContainer; }
};


class QTextCustomNode : public QTextNode
{
public:
    QTextCustomNode();
    virtual ~QTextCustomNode();

    virtual void draw(QPainter* p, int x, int y,
		      int ox, int oy, int cx, int cy, int cw, int ch,
		      QRegion& backgroundRegion, const QColorGroup& cg, const QTextOptions& ) = 0;

    virtual bool expandsHorizontally();

    int width;
    int height;
};

class QTextHorizontalLine : public QTextCustomNode
{
public:
    QTextHorizontalLine(const QMap<QString, QString> &attr, const QMimeSourceFactory& factory);
    ~QTextHorizontalLine();

    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QTextOptions& );

    bool expandsHorizontally();

};



class QTextImage : public QTextCustomNode
{
public:
    QTextImage(const QMap<QString, QString> &attr, const QString& context,
	       const QMimeSourceFactory& factory);
    ~QTextImage();

    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QTextOptions& );
private:
    QPixmap pm;
    QRegion* reg;
};


// internal class for qmlbox, also used in qmlcursor.
class QTextRow
{
public:
    QTextRow();
    QTextRow(QPainter* p, QFontMetrics &fm,
	     QTextIterator& it, int w, int& min, int align = QStyleSheetItem::AlignLeft);
    ~QTextRow();
    int x;
    int y;
    int width;
    int height;
    int base;
    int fill;
    bool intersects(int xr, int yr, int wr, int hr);
    void draw(QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QTextOptions&,
	      bool onlyDirty = FALSE, bool onlySelection = FALSE);
    QTextNode* hitTest(QPainter* p, int obx, int oby, int xarg, int yarg);


    bool locate(QPainter* p, QTextNode* node, int &lx, int &ly, int &lh);

    bool dirty;

    QTextIterator begin() const;
    QTextIterator end() const;

    QTextNode* first;
    QTextNode* last;
    QTextContainer* parent;

};


inline bool QTextRow::intersects(int xr, int yr, int wr, int hr)
{
    return ( QMAX( x, xr ) <= QMIN( x+width, xr+wr ) &&
	     QMAX( y, yr ) <= QMIN( y+height, yr+hr ) );

}

class QTextContainer : public QTextNode
{
public:
    QTextContainer( const QStyleSheetItem *stl);
    QTextContainer( const QStyleSheetItem *stl, const QMap<QString, QString> &attr );
    virtual ~QTextContainer();
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

    virtual void setParent( QTextContainer* );
    QTextContainer* parent;
    const QStyleSheetItem* style;
    QTextNode* child;

    QTextBox* box() const;
    QTextBox* parentBox() const;

    QTextIterator begin() const;
    QTextIterator end() const;

    QTextNode* lastChild() const;

    void reparentSubtree();

    virtual QTextContainer* copy() const;

    void split(QTextNode* node);

    const QMap<QString, QString> *attributes() const;

    const QTextContainer* anchor() const;

    QTextContainer* findAnchor(const QString& name ) const;

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

class QTextIterator
{
public:
    QTextIterator() { node = 0; par = 0; }
    inline QTextIterator( const QTextNode* n );
    inline QTextIterator( const QTextNode* n, const QTextContainer* p );
    QTextIterator( const QTextIterator& it) { node = it.node; par = it.par; }
    ~QTextIterator();
    QTextIterator next() const;
    bool operator==( const QTextIterator& other ) const { return other.node == node; }
    bool operator!=( const QTextIterator& other ) const { return other.node != node; }
    QTextIterator operator++ (int);
    QTextIterator& operator++ ();

    inline QTextNode* operator*() const;
    inline QTextNode* operator->() const;
    inline QTextContainer* parentNode() const;
    inline QTextIterator parent() const;

protected:
    QTextContainer* par;
    QTextNode* node;
};





class QTextMulticol : public QTextContainer
{
private:
    int ncols;
public:
    QTextMulticol( const QStyleSheetItem *stl)
	: QTextContainer(stl)
	{
	    ncols = 1;
	}
    QTextMulticol( const QStyleSheetItem *stl, const QMap<QString, QString> &attr )
	: QTextContainer(stl, attr)
	{
	    if ( attr.contains("cols") )
		ncols =  attr["cols"].toInt();
	    ncols = QMAX( 1, ncols);
	}

    ~QTextMulticol()
	{
	}

    int numberOfColumns() const
	{
	    return ncols;
	}
};

class QTextFont : public QTextContainer
{
public:
    QTextFont( const QStyleSheetItem *stl);
    QTextFont( const QStyleSheetItem *stl, const QMap<QString, QString> &attr );
    ~QTextFont();

    void setParent( QTextContainer* );
};



class QTextBox : public QTextContainer
{
public:
    QTextBox( const QStyleSheetItem *stl);
    QTextBox( const QStyleSheetItem *stl, const QMap<QString, QString> &attr );
    ~QTextBox();

    void draw(QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion,
	      const QColorGroup& cg, const QTextOptions& ,
	      bool onlyDirty = FALSE, bool onlySelection = FALSE);
    void setWidth (QPainter* p, int newWidth, bool forceResize = FALSE);

    void update(QPainter* p, QTextRow* r = 0);

    QTextContainer* copy() const;

    QList<QTextRow> rows;

    int width;
    int widthUsed;
    int height;

    //    QTextNode* locate(int x, int y);
    QTextRow*  locate(QPainter* p, QTextNode* node, int &lx, int &ly, int &lh, int&lry, int &lrh);

    QTextNode* hitTest(QPainter* p, int obx, int oby, int xarg, int yarg);

    int numberOfSubBox( QTextBox* subbox, bool onlyListItems);
    QStyleSheetItem::ListStyle listStyle();

};


class QRichText : public QTextBox
{
public:
    QRichText( const QString &doc, const QFont& fnt = QApplication::font(),
	       const QString& context = QString::null,
	       int margin = 8, const QMimeSourceFactory* factory = 0, const QStyleSheet* sheet = 0 );
    ~QRichText();


    bool isValid() const;

    QString context() const;
    void dump();

private:
    void init( const QString& doc, const QFont& fnt, int margin = 8 );

    bool parse (QTextContainer* current, QTextNode* lastChild, const QString& doc, int& pos);
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
    const QStyleSheet* sheet_;
    const QMimeSourceFactory* factory_;
    QStyleSheetItem* base;

};

inline QTextIterator::QTextIterator( const QTextNode* n )
{
    node = (QTextNode*)n;
    if ( node )
	par = node->parent();
}

inline QTextIterator::QTextIterator( const QTextNode* n, const QTextContainer* p )
{
    node = (QTextNode*) n;
    par = (QTextContainer*)p;
    if (!par && node)
	par = node->parent();
}

inline QTextNode* QTextIterator::operator*() const
{
    return node;
}

inline QTextNode* QTextIterator::operator->() const
{
    return node;
}


inline QTextContainer* QTextIterator::parentNode() const
{
    return par;
}

inline QTextIterator QTextIterator::parent() const
{
    return QTextIterator( par );
}



inline QFont QTextContainer::font() const
{
    if (!fnt) {
	QTextContainer* that = (QTextContainer*) this;
	that->createFont();
    }
    return *fnt;
}


inline QColor QTextContainer::color(const QColor& c) const
{
    if ( col.isValid() )
	return col;
    QColor sc = style->color();
    if ( sc.isValid() )
	return sc;
    return parent?parent->color(c):c;
}

inline int QTextContainer::margin(QStyleSheetItem::Margin m) const
{
    if (style->margin(m) != QStyleSheetItem::Undefined)
	return style->margin(m);
    return 0;
    //return parent?parent->margin(m):0;

}


inline QStyleSheetItem::WhiteSpaceMode  QTextContainer::whiteSpaceMode() const
{
    if ( style->whiteSpaceMode() != QStyleSheetItem::WhiteSpaceNormal )
	return style->whiteSpaceMode();
    return parent?parent->whiteSpaceMode():QStyleSheetItem::WhiteSpaceNormal;
}

inline int QTextContainer::numberOfColumns() const
{
    if (style->numberOfColumns() != QStyleSheetItem::Undefined)
	return style->numberOfColumns();
    return (parent && !parent->isBox)?parent->numberOfColumns():1;

}


inline int QTextContainer::alignment() const
{
    if ( style->alignment() != QStyleSheetItem::Undefined )
	return style->alignment();
    return parent?parent->alignment():QStyleSheetItem::AlignLeft;
}


/*!
  depthFirst traversal for the tag tree. Returns the next node
 */
inline QTextNode* QTextNode::nextNode() const
{
    if ( isContainer && ( ( const QTextContainer *) this)-> child )
	return ( ( const QTextContainer *) this)-> child;
    if ( next || !isLastSibling )
	return next;
    QTextNode* i = next;
    while ( i && i->isLastSibling )
	i = i->next;
    if ( i )
	return i->next;
    return 0;
}


/*!
  depthFirstSearch traversal for the tag tree
 */
inline QTextNode* QTextNode::depthFirstSearch(QTextNode* tag, QTextContainer* &parent, bool down)
{
    if (down) {
	if (tag->isContainer && ((QTextContainer*)tag)->child){
	    parent = (QTextContainer*)tag;
	    return ((QTextContainer*)tag)->child;
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
	QTextContainer* p = (QTextContainer*)tag->next;
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

inline QTextNode* QTextNode::nextLayout(QTextNode* tag, QTextContainer* &parent){
    QTextNode* t;

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


inline QTextNode* QTextNode::nextLeaf(QTextNode* tag, QTextContainer* &parent){
    do {
	tag = depthFirstSearch(tag, parent);

    } while (tag && tag->isContainer);

    return tag;
}



inline QTextNode* QTextNode::lastSibling() const
{
    QTextNode* n = (QTextNode*) this;

    while (n && !n->isLastSibling)
	n = n->next;
    return n;
}


inline QRichText* QTextNode::root() const
{
    if (isRoot)
	return ( QRichText* ) this;
    if (isContainer)
	return ((QTextContainer*)this)->parent->root();
    else {
	QTextNode* n = lastSibling();
	if (n) return n->next->root();
    }
    return 0;
}

inline QTextContainer* QTextNode::parent() const
{
    if (isContainer)
	return ((QTextContainer*)this)->parent;
    else {
	QTextNode* n = lastSibling();
	if (n) return (QTextContainer*)n->next;
    }
    return 0;
}

inline QTextBox* QTextNode::box() const
{
    QTextContainer* par = parent();
    if (!par)
	return 0;
    else
	return par->box();
}

inline QTextNode* QTextNode::previousSibling() const
{
    QTextContainer* par = parent();
    QTextNode* result = par->child;
    if (result == this)
	return 0;
    while (result->next && result->next != this)
	result = result->next;
    return result;
}


inline QTextNode* QTextNode::nextSibling() const
{
    if (isLastSibling)
	return 0;
    return next;
}


