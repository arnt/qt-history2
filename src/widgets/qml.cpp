/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qml.cpp#3 $
**
** Implementation of QML classes
**
** Created : 990101
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
#include "qml.h"
#include <qapplication.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qpainter.h>

#include <qstack.h>
#include <stdio.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qbitmap.h>
#include <qtimer.h>

struct QMLStyleData
{
    QMLStyle::DisplayMode disp;
    int fontitalic;
    int fontweight;
    int fontsize;
    QString fontfamily;
    QMLStyle *parentstyle;
    QString stylename;
    int ncolumns;
    QColor col;
    bool anchor;
    int align;
    int parsep;
    QMLStyle::ListStyle list;
};

/*!
  \class QMLStyle qml.h
  \brief The QMLStyle class encapsulates a QML style

  A style consists of a name and a set of
  font, color, and other display properties.
  When used in a
  \link QMLStyleSheet style sheet\endlink, styles define the
  name of a QML tag, and the display property changes associated
  with it.
*/

/*!
  Constructs a new style named \a name.

  All properties in QMLStyle are initially in the "do not change" state,
  except \link QMLStyle::DisplayMode display mode\endlink, which defaults
  to \c DisplayInline.
*/
QMLStyle::QMLStyle( const QString& name )
{
    d = new QMLStyleData;
    d->stylename = name.lower();
    init();
}


/*!
  Destroys the style.  Note that QMLStyle objects become owned
  by QMLStyleSheet when you \link QMLStyleSheet::insert() add them\endlink.
 */
QMLStyle::~QMLStyle()
{
    delete d;
}


/*!
  \internal
  Internal initialization
 */
void QMLStyle::init()
{
    d->disp = DisplayInline;

    d->fontitalic = Undefined;
    d->fontweight = Undefined;
    d->fontsize = Undefined;
    d->ncolumns = Undefined;
    d->col = QColor(); // !isValid()
    d->anchor = FALSE;
    d->align = Undefined;
    d->parsep = Undefined;
    d->list = QMLStyle::ListDisc;

}

/*!
  Returns the name of style.
*/
QString QMLStyle::name() const
{
    return d->stylename;
}

/*!
  Returns the \link QMLStyle::DisplayMode display mode\endlink of the style.

  \sa setDisplayMode()
 */
QMLStyle::DisplayMode QMLStyle::displayMode() const
{
    return d->disp;
}

/*!
  Sets the display mode of the style to \a m.

  \define QMLStyle::DisplayMode

  The affect of the available values are:
  <ul>
   <li> \c DisplayBlock
		- elements will be displayed as a rectangular block.
		    (eg. &lt;P&gt; ... &lt;/P&gt;)
   <li> \c DisplayInline
		- elements will be displayed in a horizontally flowing sequence.
		    (eg. &lt;EM&gt; ... &lt;/EM&gt;)
   <li> \c DisplayListItem
		- elements will be displayed vertically sequenced.
		    (eg. &lt;EM&gt; ... &lt;/EM&gt;)
   <li> \c DisplayNone
		- elements which are not displayed at all.
  </ul>

  \sa displayMode()
 */
void QMLStyle::setDisplayMode(DisplayMode m)
{
    d->disp=m;
}


/*!
  Returns the alignment of this style. Possible values are AlignLeft,
  AlignRight and AlignCenter.

  \sa setAlignment()
 */
int QMLStyle::alignment() const
{
    return d->align;
}

/*!
  Sets the alignment. This only makes sense for styles with
  \link QMLStyle::DisplayMode display mode\endlink
  DisplayBlock. Possible values are AlignLeft, AlignRight and
  AlignCenter.

  \sa alignment(), displayMode()
 */
void QMLStyle::setAlignment( int f )
{
    d->align = f;
}


/*!
  Returns whether the styles sets an italic or upright font.

 \sa setFontItalic(), definesFontItalic()
 */
bool QMLStyle::fontItalic() const
{
    return d->fontitalic > 0;
}

/*!
  Sets italic or upright shape for the style.

  \sa fontItalic(), definesFontItalic()
 */
void QMLStyle::setFontItalic(bool italic)
{
    d->fontitalic = italic?1:0;
}

/*!
  Returns whether the style defines a font shape.  A style
  does not define any shape until setFontItalic() is called.

  \sa setFontItalic(), fontitalic()
 */
bool QMLStyle::definesFontItalic() const
{
    return d->fontitalic != Undefined;
}


/*!
  Returns the font weight setting of the style. This is either a
  valid QFont::Weight or the value QMLStyle::Undefined.

 \sa setFontWeight, QFont
 */
int QMLStyle::fontWeight() const
{
    return d->fontweight;
}

/*!
  Sets the font weight setting of the style.  Valid values are
  those defined by QFont::Weight.

  \sa QFont, fontWeight()
 */
void QMLStyle::setFontWeight(int w)
{
    d->fontweight = w;
}

/*!
  Returns the font size setting of the style. This is either a valid
  pointsize or QMLStyle::Undefined.

 \sa setFontSize(), QFont::pointSize(), QFont::setPointSize()
 */
int QMLStyle::fontSize() const
{
    return d->fontsize;
}

/*!
  Sets the font size setting of the style, in point measures.

 \sa fontSize(), QFont::pointSize(), QFont::setPointSize()
 */
void QMLStyle::setFontSize(int s)
{
    d->fontsize = s;
}


/*!
  Returns the font family setting of the style. This is either a valid
  font family or QString::null if no family has been set.

 \sa setFontFamily(), QFont::family(), QFont::setFamily()
 */
QString QMLStyle::fontFamily() const
{
    return d->fontfamily;
}

/*!
  Sets the font family setting of the style.

 \sa fontFamily(), QFont::family(), QFont::setFamily()
 */
void QMLStyle::setFontFamily( const QString& fam)
{
    d->fontfamily = fam;
}


/*!
  Returns the number of columns for this style.

  \sa setNumberOfColumns(), displayMode(), setDisplayMode()

 */
int QMLStyle::numberOfColumns() const
{
    return d->ncolumns;
}


/*!
  Sets the number of columns for this style.  Elements in the style
  are divided into columns.

  This only makes sense
  if the style uses a \link QMLStyle::DisplayMode block display mode\endlink.

  \sa numberOfColumns()
 */
void QMLStyle::setNumberOfColumns(int ncols)
{
    if (ncols > 0)
	d->ncolumns = ncols;
}


/*!
  Returns the text color of this style, or 
  \link QColor::QColor() an invalid color\endlink
  if no color has been set yet.

  \sa setColor()
 */
QColor QMLStyle::color() const
{
    return d->col;
}

/*!
  Sets the text color of this style.

  \sa color()
 */
void QMLStyle::setColor( const QColor &c)
{
    d->col = c;
}

/*!
  Returns whether this style is an anchor.

  \sa setAnchor()
 */
bool QMLStyle::isAnchor() const
{
    return d->anchor;
}

/*!
  Sets whether the style is an anchor (link).  Elements in this style
  have connections to other documents or anchors.

  \sa isAnchor()
 */
void QMLStyle::setAnchor(bool anc)
{
    d->anchor = anc;
}



/*!
  Returns the separation of subparagraphs in pixel.
  
  \sa setParagraphSeparation()
 */
int QMLStyle::paragraphSeparation() const
{
    return d->parsep;
}


/*!
  Sets the separation of subparagraphs in pixels.
  The value must be >= 0.
  
  \sa paragraphSeparation()
 */
void QMLStyle::setParagraphSeparation(int v)
{
    d->parsep = v;
}


/*!
  Returns the list style of the style.

  \sa setListStyle()
 */
QMLStyle::ListStyle QMLStyle::listStyle() const
{
    return d->list;
}

/*!
  Sets the list style of the style

  This is used by nested elements which have a
  \link QMLStyle::DisplayMode display mode\endlink of DisplayListItem.

  \define QMLStyle::ListStyle

  The affect of the available values are:
  <ul>
   <li> \c ListDisc
		- a filled circle
   <li> \c ListCircle
		- an unfilled circle
   <li> \c ListSquare
		- a filled circle
   <li> \c ListDecimal
		- an integer in base 10: \e 1, \e 2, \e 3, ...
   <li> \c ListLowerAlpha
		- a lowercase letter: \e a, \e b, \e c, ...
   <li> \c ListUpperAlpha
		- an uppercase letter: \e A, \e B, \e C, ...
  </ul>

  \sa listStyle()
 */
void QMLStyle::setListStyle( QMLStyle::ListStyle s)
{
    d->list=s;
}


//************************************************************************


class QMLBox;
class QMLNode
{
public:
    QMLNode();
    ~QMLNode();
    QMLNode* next;

    inline QMLNode* depthFirstSearch(QMLNode* tag, QMLContainer* &parent, bool down = TRUE);
    inline QMLNode* nextLayout(QMLNode* tag, QMLContainer* &parent);
    inline QMLNode* nextLeaf(QMLNode* tag, QMLContainer* &parent);

    QChar c;

    inline bool isSpace() const {return c.isSpace();}
    inline bool isNull() const {return c == QChar::null;}

    inline QMLContainer* parent() const;
    inline QMLBox* box() const;
    inline QMLNode* previousSibling() const;
    inline QMLNode* lastSibling() const;
    inline QMLNode* nextSibling() const;

    uint isSimpleNode: 1;
    uint isLastSibling:1;
    uint isContainer:1;
    uint isBox:1;
    uint isSelected: 1;
    uint isSelectionDirty: 1;
};




class QMLCustomNode : public QMLNode
{
public:
    QMLCustomNode();
    virtual ~QMLCustomNode();

    virtual void draw(QPainter* p, int x, int y,
		      int ox, int oy, int cx, int cy, int cw, int ch,
		      QRegion& backgroundRegion, const QColorGroup& cg, const QBrush* paper = 0) = 0;

    int width;
    int height;
};

class QMLImage : public QMLCustomNode
{
public:
    QMLImage(const QDict<QString>&attr, const QMLProvider& provider);
    ~QMLImage();

    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QBrush* paper = 0);
private:
    QPixmap pm;
    QRegion* reg;
};


// internal class for qmlbox, also used in qmlcursor.
class QMLRow
{
public:
    QMLRow();
    QMLRow(QMLContainer* box, QPainter* p, QMLNode* &t, QMLContainer* &par, int w, int align = QMLStyle::AlignLeft);
    ~QMLRow();
    int x;
    int y;
    int width;
    int height;
    int base;
    int fill;
    bool intersects(int xr, int yr, int wr, int hr);
    void draw(QMLContainer* box, QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, const QBrush* paper = 0,
	      bool onlyDirty = FALSE, bool onlySelection = FALSE);
    QMLNode* hitTest(QMLContainer* box, QPainter* p, int obx, int oby, int xarg, int yarg);


    bool locate(QMLContainer* box, QPainter* p, QMLNode* node, int &lx, int &ly, int &lh);

    bool dirty;

    QMLNode* start;
    QMLNode* end;
    QMLContainer* parent;

};


inline bool QMLRow::intersects(int xr, int yr, int wr, int hr)
{
    return ( QMAX( x, xr ) <= QMIN( x+width, xr+wr ) &&
	     QMAX( y, yr ) <= QMIN( y+height, yr+hr ) );

}

class QMLContainer : public QMLNode
{
public:
    QMLContainer( const QMLStyle *stl);
    QMLContainer( const QMLStyle *stl, const QDict<QString>& attr );
    virtual ~QMLContainer();
    inline QFont font() const;
    inline QColor color(const QColor&) const;
    inline int paragraphSeparation() const;
    inline int numberOfColumns() const;
    inline int alignment() const;

    QMLContainer* parent;
    const QMLStyle* style;
    QMLNode* child;

    QMLBox* box() const;
    QMLBox* parentBox() const;

    QMLNode* lastChild() const;

    void reparentSubtree();

    virtual QMLContainer* copy() const;

    void split(QMLNode* node);

    const QDict<QString>* attributes() const;

    const QMLContainer* anchor() const;

protected:
    void setAttributes(const QDict<QString>& attr );

private:
    int fontWeight() const;
    int fontItalic() const;
    QString fontFamily() const;
    int fontSize() const;

    void createFont();

    QFont* fnt;
    QDict<QString>* attributes_;
};


inline QFont QMLContainer::font() const
{
    if (!fnt) {
	QMLContainer* that = (QMLContainer*) this;
	that->createFont();
    }
    return *fnt;
}


inline QColor QMLContainer::color(const QColor& c) const
{
    QColor sc = style->color();
    if ( sc.isValid() )
	return sc;
    return parent?parent->color(c):c;
}

inline int QMLContainer::paragraphSeparation() const
{
    if (style->paragraphSeparation() != QMLStyle::Undefined)
	return style->paragraphSeparation();
    return parent?parent->paragraphSeparation():0;

}


inline int QMLContainer::numberOfColumns() const
{
    if (style->numberOfColumns() != QMLStyle::Undefined)
	return style->numberOfColumns();
    return (parent && !parent->isBox)?parent->numberOfColumns():1;

}


inline int QMLContainer::alignment() const
{
    if ( style->alignment() != QMLStyle::Undefined )
	return style->alignment();
    return parent?parent->alignment():QMLStyle::AlignLeft;
}


class QMLBox : public QMLContainer
{
public:
    QMLBox( const QMLStyle *stl);
    QMLBox( const QMLStyle *stl, const QDict<QString>& attr );
    ~QMLBox();

    void draw(QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion,
	      const QColorGroup& cg, const QBrush* paper,
	      bool onlyDirty = FALSE, bool onlySelection = FALSE);
    void setWidth (QPainter* p, int newWidth, bool forceResize = FALSE);

    void update(QPainter* p, QMLRow* r = 0);

    QMLContainer* copy() const;

    QList<QMLRow> rows;

    int width;
    int height;

    int border;

    //    QMLNode* locate(int x, int y);
    QMLRow*  locate(QPainter* p, QMLNode* node, int &lx, int &ly, int &lh, int&lry, int &lrh);

    QMLNode* hitTest(QPainter* p, int obx, int oby, int xarg, int yarg);


    int numberOfSubBox( QMLBox* subbox);
    QMLStyle::ListStyle listStyle();

};


//************************************************************************


/*!
  \class QMLStyleSheet qml.h
  \brief A collection of styles and a generator of tags.

  By \link QMLStyleSheet::insert() inserting\endlink QStyle objects
  into a style sheet, you build a definition of a set of tags.  This
  definition will be used by the internal QML features to parse
  and display QML documents to which the style sheet applies.

  The default QMLStyleSheet object has the following style bindings:

  <ul>
    <li>&lt;qml&gt;...&lt;/qml&gt;
	A QML document.

    <li>&lt;a&gt;...&lt;/a&gt;
	An anchor or link.

    <li>&lt;em&gt;...&lt;/em&gt;
	Emphasized.

    <li>&lt;large&gt;...&lt;/large&gt;
	Large font size.

    <li>&lt;b&gt;...&lt;/b&gt;
    style->setFontWeight( QFont::Bold);
	Bold font style.

    <li>&lt;h1&gt;...&lt;/h1&gt;
	A top-level heading.

    <li>&lt;p&gt;...&lt;/p&gt;
	A paragraph.

    <li>&lt;center&gt;...&lt;/center&gt;
	A centered paragraph.

    <li>&lt;twocolumn&gt;...&lt;/twocolumn&gt;
	Two-column display.

    <li>&lt;ul&gt;...&lt;/ul&gt;
	An un-ordered list.

    <li>&lt;ol&gt;...&lt;/ol&gt;
	An ordered list.

    <li>&lt;li&gt;...&lt;/li&gt;
	A list item.

    <li>&lt;img&gt;
	An image.
  </ul>
*/

/*!
  Create a style sheet.  Like any QObject, the created object will be
  deleted when its parent destructs (if the child still exists then).

  By default, the style sheet has the QML definitions defined above.
*/
QMLStyleSheet::QMLStyleSheet( QObject *parent, const char *name )
    : QObject( parent, name )
{
    init();
}

/*!
  Destroys the style sheet.  All styles inserted into the style sheet
  will be deleted.
*/
QMLStyleSheet::~QMLStyleSheet()
{
    delete nullstyle;
}

/*!
  \internal
  Initialized the style sheet to the QML style.
*/
void QMLStyleSheet::init()
{
    styles.setAutoDelete( TRUE );

    nullstyle  = new QMLStyle("");

    QMLStyle*  style;

    style = new QMLStyle( "qml" );
    style->setDisplayMode( QMLStyle::DisplayBlock );
    insert(style);

    style = new QMLStyle( "a" );
    style->setColor( Qt::blue );
    style->setAnchor( TRUE );
    insert( style );

    style = new QMLStyle( "em" );
    style->setFontItalic( TRUE );
    insert(style);

    style = new QMLStyle( "large" );
    style->setFontSize( 24 );
    insert(style);

    style = new QMLStyle( "b" );
    style->setFontWeight( QFont::Bold);
    insert(style);

    style = new QMLStyle( "h1" );
    style->setFontWeight( QFont::Bold);
    style->setFontSize(24);
    style->setDisplayMode(QMLStyle::DisplayBlock);
    insert(style);

    style = new QMLStyle( "p" );
    style->setDisplayMode(QMLStyle::DisplayBlock);
    insert(style);

    style = new QMLStyle( "center" );
    style->setDisplayMode(QMLStyle::DisplayBlock);
    style->setAlignment( AlignCenter );
    insert(style);

    style = new QMLStyle( "twocolumn" );
    style->setNumberOfColumns( 2 );
    insert(style);

    style = new QMLStyle( "ul" );
    style->setDisplayMode(QMLStyle::DisplayBlock);
    style->setParagraphSeparation( 4 );
    insert(style);

    style = new QMLStyle( "ol" );
    style->setDisplayMode(QMLStyle::DisplayBlock);
    style->setParagraphSeparation( 4 );
    style->setListStyle( QMLStyle::ListDecimal );
    insert(style);

    style = new QMLStyle( "li" );
    style->setDisplayMode(QMLStyle::DisplayListItem);
    insert(style);

    insert(new QMLStyle("img"));

}



static QMLStyleSheet* defaultsheet = 0;

/*!
  Returns the application-wide default style sheet.
*/
QMLStyleSheet& QMLStyleSheet::defaultSheet()
{
    if (!defaultsheet)
	defaultsheet = new QMLStyleSheet();
    return *defaultsheet;
}

/*!
  Sets the application-wide default style sheet, deleting any style
  sheet previously set.
*/
void QMLStyleSheet::setDefaultSheet( QMLStyleSheet* sheet)
{
    if (defaultsheet)
	delete defaultsheet;
    defaultsheet = sheet;
}

/*!
  Inserts \a style.  Any tags generated after this time will be
  bound to this style.  Note that \a style becomes owned by the
  style sheet and will be deleted when the style sheet destructs.
*/
void QMLStyleSheet::insert( QMLStyle* style )
{
    styles.insert(style->name(), style);
}


/*!
  Generates an internal object for tag named \a name, given the
  attributes \a attr, and using additional information provided
  by \a provider.

  This function should not (yet) be used in application code.
*/
QMLNode* QMLStyleSheet::tag( const QString& name,
			     const QDict<QString> &attr,
			     const QMLProvider& provider ) const
{
    QMLStyle* style = styles[name];
    if ( !style )
	style = nullstyle;

    // first the empty tags
    if (style->name() == "img")
	return new QMLImage(attr, provider);

    // process containers
    switch ( style->displayMode() ) {
    case QMLStyle::DisplayBlock:
    case QMLStyle::DisplayListItem:
	return new QMLBox( style, attr );
    default: // inline, none
	return new QMLContainer( style, attr );
    }
}




class QMLDocument : public QMLBox
{
public:
    QMLDocument( const QString &doc, const QWidget* w = 0);
    QMLDocument( const QString &doc, const QMLProvider& provider, const QWidget* w = 0);
    QMLDocument( const QString &doc,  const QMLProvider& provider, const QMLStyleSheet& sheet, const QWidget* w = 0);
    ~QMLDocument();


    bool isValid() const;

    void dump();

private:
    void init( const QString& doc, const QWidget* w = 0 );

    void parse (QMLContainer* current, QMLNode* lastChild, const QString& doc, int& pos);
    bool eatSpace(const QString& doc, int& pos);
    bool eat(const QString& doc, int& pos, const QChar& c);
    bool lookAhead(const QString& doc, int& pos, const QChar& c);
    QString parseOpenTag(const QString& doc, int& pos, QDict<QString> &attr);
    bool eatCloseTag(const QString& doc, int& pos, const QString& open);
    QString parseWord(const QString& doc, int& pos, bool lower = FALSE);
    QString parsePlainText(const QString& doc, int& pos);
    bool hasPrefix(const QString& doc, int pos, const QChar& c);
    bool valid;
    QChar* openChar;
    QChar* closeChar;
    QChar* slashChar;
    QChar* quoteChar;
    QChar* equalChar;
    const QMLStyleSheet* sheet_;
    const QMLProvider* provider_;
    QMLStyle* base;

};


//************************************************************************



QMLNode::QMLNode()
{
    next = 0;
    isSimpleNode = 1;
    isLastSibling = 0;
    isContainer = 0;
    isBox = 0;
    isSelected = 0;
    isSelectionDirty = 0;
}


QMLNode::~QMLNode()
{
}




//************************************************************************

QMLCustomNode::QMLCustomNode()
    :QMLNode()
{
    isSimpleNode = 0;
    width = height = 0;
}

QMLCustomNode::~QMLCustomNode()
{
}


QMLImage::QMLImage(const QDict<QString> &attr, const QMLProvider &provider)
    : QMLCustomNode()
{
    reg = 0;
    QString* imageName = attr["source"];
    if (imageName) {
	pm = provider.image( *imageName );
	pm.setMask( pm.createHeuristicMask() );
	width = pm.width();
	height = pm.height();
	if ( pm.mask() ) {
	    QRegion mask( *pm.mask() );
	    QRegion all( 0, 0, pm.width(), pm.height() );
	    reg = new QRegion( all.subtract( mask ) );
	}
	if (pm.isNull()) {
	    width = height = 10;
	}
    }
}

QMLImage::~QMLImage()
{
}

void QMLImage::draw(QPainter* p, int x, int y,
		    int ox, int oy, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/,
		    QRegion& backgroundRegion, const QColorGroup& /* cg */, const QBrush* /*bg*/)
{
    if ( reg ){
	QRegion tmp( *reg );
	tmp.translate( x-ox, y-oy );
	backgroundRegion = backgroundRegion.unite( tmp );
    }
    p->drawPixmap( x-ox , y-oy, pm );
}


//************************************************************************

QMLRow::QMLRow()
{
    x = y = width = height = base = 0;
    start = end = 0;
    parent = 0;
    fill = 0;

    dirty = TRUE;
}

QMLRow::QMLRow( QMLContainer* box, QPainter* p, QMLNode* &t, QMLContainer* &par, int w, int align)
{
    x = y = width = height = base = 0;
    start = end = 0;
    dirty = TRUE;

    width = w;

    start = t;
    parent = par;

    int tx = 0;
    int rh = 0;
    int rasc = 0;
    int rdesc = 0;

    if (t->isBox) {
	QMLBox* b = (QMLBox*)t;
	height = b->height;
	base = height;
	end = t;
	t = box->nextLayout(t, par);
	return;
    }

    QMLNode* i = t;

    // do word wrap
    QMLContainer* lastPar = par;
    QMLNode* lastSpace = t;
    int lastHeight = rh;
    int lastWidth = 0;
    int lastAsc = rasc;
    int lastDesc = rdesc;
    bool noSpaceFound = TRUE;

    while (i && !i->isBox) {
	p->setFont( par->font() );
	QFontMetrics fm = p->fontMetrics();

	int h,a,d;
	if (i->isSimpleNode) {
	    if (!i->isNull())
		tx += fm.width(i->c);
	    h = fm.height();
	    a = fm.ascent();
	    d = h-a;
	}
	else {
	    tx += ((QMLCustomNode*)i)->width;
	    h = ((QMLCustomNode*)i)->height;
	    a = h;
	    d = 0;
	}
	if (tx > width && i != t)
	    break;

	rh = QMAX( rh, h );
	rasc = QMAX( rasc, a );
	rdesc = QMAX( rdesc, d );

	QMLNode* cur = i;
	i = box->nextLayout(i, par);
	
	// break (a) after a space, (b) before a box, (c) if we have
	// to or (d) at the end of a box.
	if (cur->isSpace() || (i&&i->isBox) || noSpaceFound || !i || i == t){
	    lastPar = par;
	    lastSpace = cur;
	    lastHeight = rh;
	    lastAsc = rasc;
	    lastDesc = rdesc;
	    lastWidth = tx;
	    if (noSpaceFound && cur->isSpace())
		noSpaceFound = FALSE;
	}	
    }
    end = lastSpace;
    i = box->nextLayout(lastSpace, lastPar);
    rh = lastHeight;
    rasc = lastAsc;
    rdesc = lastDesc;

    par = lastPar;

    height = QMAX(rh, rasc+rdesc);
    base = rasc;

    if (align == Qt::AlignCenter)
	fill = (width - lastWidth) / 2;
    else if (align == Qt::AlignRight)
	fill = width - lastWidth;
    else
	fill = 0;

    t = i;
}



QMLRow::~QMLRow()
{
}


void QMLRow::draw(QMLContainer* box, QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, const QBrush* paper,
		  bool onlyDirty, bool onlySelection)
{
    static QString s;

    if (!intersects(cx-obx, cy-oby, cw,ch)) {
	dirty = FALSE;
	return;
    }


    if (start->isBox) {
	//we have to draw the box
	((QMLBox*)start)->draw(p, obx+x, oby+y, ox, oy, cx, cy, cw, ch,
			       backgroundRegion, cg, paper, dirty?FALSE:onlyDirty, onlySelection);
	dirty = FALSE;
	return;
    }

    QRegion r(x+obx-ox, y+oby-oy, width, height);

    backgroundRegion = backgroundRegion.subtract(r);

    if (onlyDirty) {
	if (!dirty)
	    return;
    }

    dirty = FALSE;
    QMLNode* t = start;
    QMLContainer* par = parent;

    bool reducedFlickerMode = FALSE;
    do {
	if ( !t->isSimpleNode) {
	    reducedFlickerMode = TRUE;
	    break;
	}
	if (t == end)
	    break;
	t = box->nextLayout(t, par);
    } while ( t );


    if ( !reducedFlickerMode ) {
	if (!onlyDirty && !onlySelection && paper) {
	    if ( paper->pixmap() )
		p->drawTiledPixmap(x+obx-ox, y+oby-oy, width, height, *paper->pixmap(), x+obx, y+oby);
	    else
		p->fillRect(x+obx-ox, y+oby-oy, width, height, *paper);
	}
    }




    t = start;
    bool inStart = TRUE;
    par = parent;

    int tx = x + fill;

    do {
	s.truncate(0);
	p->setFont( par->font() );
	QFontMetrics fm = p->fontMetrics();
	int tw = 0;
	QMLNode* tmp = 0;
	bool select = t->isSelected;
	bool selectionDirty = t->isSelectionDirty;
	t->isSelectionDirty = 0;
	if (t->isSimpleNode) {
	    if (!t->isNull()){
		s += t->c;
		tw += fm.width( t->c );
	    }
	    // special optimized code for simple nodes (characters)
	    while ( t != end && (tmp = t->nextSibling() ) && tmp->isSimpleNode
		    && ((bool)tmp->isSelected) == select
		    && ((bool) tmp->isSelectionDirty) == selectionDirty
		    && t->isSimpleNode
		    ) {
		t = tmp;
		tmp->isSelectionDirty = 0;
		if (!t->isNull()) {
		    s += t->c;
		    tw += fm.width( t->c );
		}
		// 	    if (t->isSpace())
		// 	      break;
	    }
	}
	else {
	    // custom nodes
	    tw += ((QMLCustomNode*)t)->width;
	}
	

	if (!onlySelection || selectionDirty) {
	    p->setPen( par->color(cg.text()) );
	
	    if (select) {
		if ( inStart )
		    p->fillRect(x+obx-ox, y+oby-oy, tw+(tx-x), height, cg.highlight() );
		if (t ==end)
		    p->fillRect(tx+obx-ox, y+oby-oy, width-(tx-x), height, cg.highlight());
		else
		    p->fillRect(tx+obx-ox, y+oby-oy, tw, height, cg.highlight());
		p->setPen( cg.highlightedText() );
	    }
	    else if ( (onlyDirty || onlySelection || (reducedFlickerMode && t->isSimpleNode)) && paper ) {
		int txo = 0;
		if ( inStart ) {
		    if ( paper->pixmap() )
			p->drawTiledPixmap(x+obx-ox, y+oby-oy, tw+(tx-x), height,
					   *paper->pixmap(), x+obx, y+oby);
		    else
			p->fillRect(x+obx-ox, y+oby-oy, tw+(tx-x), height, *paper );
		    txo = tw;
		}
		if (t==end){
		    if ( paper->pixmap() )
			p->drawTiledPixmap(tx+obx-ox+txo, y+oby-oy, width-(tx-x)-txo, height,
					   *paper->pixmap(), tx+obx+txo, y+oby);
		    else
			p->fillRect(tx+obx-ox+txo, y+oby-oy, width-(tx-x)-txo, height, *paper);
		}
		else {
		    if ( paper->pixmap() )
			p->drawTiledPixmap(tx+obx-ox, y+oby-oy, tw, height,
					   *paper->pixmap(), tx+obx, y+oby);
		    else
			p->fillRect(tx+obx-ox, y+oby-oy, tw, height, *paper);
		}
	    }
	
	    if (t->isSimpleNode) {
		p->drawText(tx+obx-ox, y+oby-oy+base, s);
	    }
	    else {
		if ( reducedFlickerMode ) {
		    if (!t->isSelected) {
			if ( inStart ) {
			    if ( paper->pixmap() )
				p->drawTiledPixmap(x+obx-ox, y+oby-oy, (tx-x), height,
						   *paper->pixmap(), x+obx, y+oby);
			    else
				p->fillRect(x+obx-ox, y+oby-oy, (tx-x), height, *paper);
			}
			if (t==end){
			    if ( paper->pixmap() )
				p->drawTiledPixmap(tx+tw+obx-ox, y+oby-oy, width-(tx-x)-tw, height,
						   *paper->pixmap(), tx+obx, y+oby);
			    else
				p->fillRect(tx+tw+obx-ox, y+oby-oy, width-(tx-x)-tw, height, *paper);
			}
		    }
		    int h = ((QMLCustomNode*)t)->height;
		    if ( h < height && !t->isSelected ) {
			if ( paper->pixmap() )
			    p->drawTiledPixmap(tx+obx-ox, y+oby-oy, tw, base-h,
					       *paper->pixmap(), tx+obx, y+oby);
			else
			    p->fillRect(tx+obx-ox, y+oby-oy, tw, base-h, *paper);
			if ( paper->pixmap() )
			    p->drawTiledPixmap(tx+obx-ox, y+oby-oy+base, tw, height-base,
					       *paper->pixmap(), tx+obx, y+oby);
			else
			    p->fillRect(tx+obx-ox, y+oby-oy+base, tw, height-base, *paper);
		    }
		    ((QMLCustomNode*)t)->draw(p,tx+obx,y+oby+base-h,
					      ox, oy, cx, cy, cw, ch, backgroundRegion, cg, paper);
		
		    if ( t->isSelected ) {
			QRect tr( tx+obx-ox, y+oby-oy+base-h, tw, h );
			backgroundRegion = backgroundRegion.subtract( tr );
		    }
		}
	    }
	}
	tx += tw;
	if (t == end)
	    break;
	t = box->nextLayout(t, par);
	inStart = FALSE;
    } while ( t );

}

QMLNode* QMLRow::hitTest(QMLContainer* box, QPainter* p, int obx, int oby, int xarg, int yarg)
{
    if (!intersects(xarg-obx, yarg-oby, 0,0))
	return 0;

    if (start->isBox) {
	return ((QMLBox*)start)->hitTest(p, obx+x, oby+y, xarg, yarg);
    }

    QMLNode* t = start;
    QMLContainer* par = parent;
    int tx = fill;
    QMLNode* result = t;
    do {
	p->setFont( par->font() );
	QFontMetrics fm = p->fontMetrics();
	if (t->isSimpleNode)
	    tx += fm.width( t->c );
	else
	    tx += ((QMLCustomNode*)t)->width;
	result = t;
	t = box->nextLayout(t, par);
    } while (result != end && obx + x + tx <= xarg);
	
    return result;

}

bool QMLRow::locate(QMLContainer* box, QPainter* p, QMLNode* node, int &lx, int &ly, int &lh)
{
    if (start->isBox) { // a box row
	if (node == start) {
	    lx = x;
	    ly = y;
	    lh = height;
	    return TRUE;
	}
	return FALSE;
    }



    QMLNode* t = start;
    QMLContainer* par = parent;

    while (t && t != node && t != end)
	t = box->nextLayout(t, par);
    if (t != node ) {
	return FALSE; // nothing found
    }

    t = start;
    par = parent;
    lx = x + fill;
    QFontMetrics fm = p->fontMetrics();
    while (t != node) {
	p->setFont( par->font() );
	fm = p->fontMetrics();
	if (t->isSimpleNode)
	    lx += fm.width( t->c );
	else
	    lx += ((QMLCustomNode*)t)->width;
	t = box->nextLayout(t, par);
    };
    p->setFont( par->font() );
    fm = p->fontMetrics();
    ly = y + base - fm.ascent();
    lh = fm.height();

    return TRUE;
}



//************************************************************************
QMLContainer::QMLContainer( const QMLStyle *stl)
    : style(stl)
{
    isSimpleNode = 0;
    isContainer = 1;
    fnt = 0;
    parent = 0;
    child = 0;
    attributes_ = 0;
}

QMLContainer::QMLContainer( const QMLStyle *stl, const QDict<QString>& attr )
    : style(stl)
{
    isSimpleNode = 0;
    isContainer = 1;
    fnt = 0;
    parent = 0;
    child = 0;
    attributes_ = 0;
    if (!attr.isEmpty() )
	setAttributes( attr );
}

void QMLContainer::setAttributes(const QDict<QString>& attr )
{
    delete attributes_;
    attributes_ = new QDict<QString>;
    attributes_->setAutoDelete( TRUE );
    //#### we really need a QStringDict!
    QDictIterator<QString> it(attr);
    while ( it.current() ) {
	attributes_->insert( it.currentKey(), new QString( *it.current() ) );
	++it;
    }
}

QMLContainer::~QMLContainer()
{
    delete fnt;
    delete attributes_;
    QMLNode* nx = 0;
    for (QMLNode* n = child; n; n = nx) {
	if (n->isLastSibling)
	    nx = 0;
	else
	    nx = n->next;
	
	if (n->isBox)
	    delete (QMLBox*)n;
	else if (n->isContainer)
	    delete (QMLContainer*)n;
	else if (n->isSimpleNode)
	    delete n;
	else
	    delete (QMLCustomNode*)n;
    }
}


QMLContainer* QMLContainer::copy() const
{
    QMLContainer* result = new QMLContainer( style );
    if (attributes_)
	result->setAttributes( *attributes_ );
    return result;
}

void QMLContainer::split(QMLNode* node)
{
    QMLContainer* c2 = copy();

    QMLNode* prev = node->previousSibling(); // slow!
    if (!node->isContainer) {
	QMLNode* n = new QMLNode;
	n->c = QChar::null;
	n->isLastSibling = 1;
	n->next = this;
	if (prev)
	    prev->next = n;
	else
	    child = n;
    }
    else {
	if (prev){
	    prev->isLastSibling = 1;
	    prev->next = this;
	}
	else
	    child = 0;
    }

    c2->child = node;
    c2->parent = parent;

    c2->next = next;
    next = c2;
    c2->isLastSibling = isLastSibling;
    isLastSibling = 0;

    if (!isBox)
	parent->split(c2);
    else
	c2->reparentSubtree();
}


const QDict<QString>* QMLContainer::attributes() const
{
    return attributes_;
}

const QMLContainer* QMLContainer::anchor() const
{
    if ( style->isAnchor() )
	return this;
    if ( parent )
	return parent->anchor();
    return 0;
}


QMLBox* QMLContainer::box() const
{
    QMLContainer* result = (QMLContainer*) this;
    while (result && !result->isBox)
	result = result->parent;
    return (QMLBox*)result;
}

QMLBox* QMLContainer::parentBox() const
{
    QMLContainer* result = (QMLContainer*) parent;
    while (result && !result->isBox)
	result = result->parent;
    return (QMLBox*)result;
}


QMLNode* QMLContainer::lastChild() const
{
    if (!child)
	return 0;
    return child->lastSibling();
}


void QMLContainer::reparentSubtree()
{
    QMLNode* n = child;
    while (n) {
	if (n->isContainer) {
	    delete  ((QMLContainer*)n)->fnt;
	    ((QMLContainer*)n)->fnt = 0;
	    ((QMLContainer*)n)->parent = this;
	    ((QMLContainer*)n)->reparentSubtree();
	}
	if (n->isLastSibling) {
	    n->next = this;
	    break;
	}
	n = n->next;
    }
}




void QMLContainer::createFont()
{
    fnt = parent?new QFont(parent->font()) : new QFont( fontFamily() );
    fnt->setFamily( fontFamily() );
    fnt->setPointSize( fontSize() );
    fnt->setWeight( fontWeight() );
    if (style-> definesFontItalic() )
	fnt->setItalic( style->fontItalic() );
}



int QMLContainer::fontWeight() const
{
    int w = style->fontWeight();
    if ( w == QMLStyle::Undefined && parent )
	w = parent->fontWeight();
    return w;
}

int QMLContainer::fontItalic() const
{
    int fi = style->fontItalic();
    if ( fi == QMLStyle::Undefined && parent )
	fi = parent->fontItalic();
    return fi;
}

int QMLContainer::fontSize() const
{
    int w = style->fontSize();
    if ( w == -1 && parent )
	w = parent->fontSize();
    return w;
}

QString QMLContainer::fontFamily() const
{
    QString f = style->fontFamily();
    if ( f.isNull() && parent )
	f = parent->fontFamily();
    return f;
}

//************************************************************************

QMLBox::QMLBox( const QMLStyle *stl)
    :QMLContainer(stl)
{
    rows.setAutoDelete(true);
    isSimpleNode = 0;
    isBox = 1;
    border = width = height = 0;
}

QMLBox::QMLBox( const QMLStyle *stl, const QDict<QString>& attr )
    :QMLContainer(stl, attr)
{
    rows.setAutoDelete(true);
    isSimpleNode = 0;
    isBox = 1;
    border = width = height = 0;
}

QMLContainer* QMLBox::copy() const
{
    QMLBox* result = new QMLBox( style );
    return result;
}

QMLBox::~QMLBox()
{
}



#define IN16BIT(x) QMAX( (2<<15)-1, x)

void QMLBox::draw(QPainter *p,  int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, const QBrush* paper,
		  bool onlyDirty, bool onlySelection)
{
    if (onlySelection && !isSelectionDirty)
	return;
    isSelectionDirty = 0;

    if ( !onlySelection && style->displayMode() == QMLStyle::DisplayListItem && rows.first()) {
	QMLRow* row = rows.first();
	QRect r (obx-ox + row->x - 25, oby-oy + row->y, 25, row->height); //#### label width
	if (paper) {
	    if ( paper->pixmap() )
		p->drawTiledPixmap( r, *paper->pixmap(), QPoint(ox, oy) );
	    else
		p->fillRect(r, *paper );
	}
	
	QMLBox* b = parentBox();
	QMLStyle::ListStyle s = b?b->listStyle():QMLStyle::ListDisc;
	
	switch ( s ) {
	case QMLStyle::ListDecimal:
	case QMLStyle::ListLowerAlpha:
	case QMLStyle::ListUpperAlpha:
	    {
		int n = 1;
		if ( b )
		    n = b->numberOfSubBox( this );
		QString l;
		switch ( s ) {
		case QMLStyle::ListLowerAlpha:
		    if ( n < 27 ) {
			l = QChar( ('a' + (char) (n-1)));
			break;
		    }
		case QMLStyle::ListUpperAlpha:
		    if ( n < 27 ) {
			l = QChar( ('A' + (char) (n-1)));
			break;
		    }
		    break;
		default:  //QMLStyle::ListDecimal:
		    l.setNum( n );
		    break;
		}
		l += ". ";
		p->setFont( font() );
		p->drawText( r, Qt::AlignRight|Qt::AlignVCenter, l);
	    }
	    break;
	case QMLStyle::ListSquare:
	    {
		QRect er( r.right()-10, r.center().y()-1, 6, 6);
		p->fillRect( er , cg.text() );
	    }
	    break;
	case QMLStyle::ListCircle:
	    {
		QRect er( r.right()-10, r.center().y()-1, 6, 6);
		p->drawEllipse( er );
	    }
	    break;
	case QMLStyle::ListDisc:
	default:
	    {
		p->setBrush( Qt::SolidPattern );
		QRect er( r.right()-10, r.center().y()-1, 6, 6);
		p->drawEllipse( er );
		p->setBrush( Qt::NoBrush );
	    }
	    break;
	}
	
	backgroundRegion = backgroundRegion.subtract( r );
    }



    for (QMLRow* row = rows.first(); row; row = rows.next()) {
	row->draw(this, p, obx, oby, ox, oy, cx, cy, cw, ch, backgroundRegion, cg, paper, onlyDirty, onlySelection);
    }

}


void QMLBox::setWidth(QPainter* p, int newWidth, bool forceResize)
{
    if (newWidth == width && !forceResize) // no need to resize
	return;


    QList<QMLRow> oldRows;
    if ( newWidth == width ){
	// reduce flicker by storing the old rows.
	oldRows = rows;
	rows.setAutoDelete( FALSE );
	oldRows.setAutoDelete( TRUE );
    }
    rows.clear();
    rows.setAutoDelete( TRUE );

    width = newWidth;
    height = 0;

    int label_offset = 0;
    if ( style->displayMode() == QMLStyle::DisplayListItem )
	label_offset = 25; //### hardcoded

    int ncols = numberOfColumns();
    int colwidth = newWidth / ncols;
    if (colwidth < 10)
	colwidth = 10;

    QMLContainer* par = this;
    QMLNode* n = nextLayout( this, par);
    QMLRow* row = 0;


    int parsep = paragraphSeparation();
    int h = parsep;

    while (n) {
	if (n->isBox){
	    ((QMLBox*)n)->setWidth(p, colwidth-2*border); // todo this can be done in word wrap?!
	}
	row = new QMLRow(this, p, n, par, colwidth-2*border - label_offset, alignment() );
	rows.append(row);
	h += row->height;
	if ( parsep > 0 && row->start->isBox )
	    h += parsep;
    }

    // do multi columns if required. Also check with the old rows to
    // optimize the refresh

    row = rows.first();
    QMLRow* old = oldRows.first();
    height = 0;
    h /= ncols;
    for (int col = 0; col < ncols; col++) {
	int colheight = parsep;
	for (; row && colheight < h; row = rows.next()) {
	    row->x = col  * colwidth + border + label_offset;
	    row->y = colheight;
	
	    colheight += row->height;
	    if ( parsep > 0 && row->start->isBox ) {
		colheight += parsep;
	    }
	
	
	    if ( old) {
		if ( row->start->isBox ) {
		    // do not check a height changes of box rows!
		    if (old->start == row->start && old->end == row->end
			&& old->width == old->width
			&& old->x == row->x && old->y == row->y)
			{
			    row->dirty = old->dirty;
			}
		}
		else if (old->start == row->start && old->end == row->end
			 && old->height == row->height && old->width == old->width
			 && old->x == row->x && old->y == row->y)
		    {
			row->dirty = old->dirty;
		    }

		old = oldRows.next();
	    }
	}
	height = QMAX( height, colheight );
    }
}


void QMLBox::update(QPainter* p, QMLRow* r)
{
    if ( r ) { // optimization
	QMLRow* row;
	QMLRow* prev = 0;
	
	//todo drop QList and connect the rows directly *sigh*
	for ( row = rows.first(); row && row != r; row = rows.next()) {
	    prev = row;
	}
	bool fast_exit = TRUE;
	if (prev) {
	    QMLContainer* par = prev->parent;
	    QMLNode* n = prev->start;
	    QMLRow tr (this, p, n, par, prev->width);
	    fast_exit &= prev->end == tr.end;
	}
	if (fast_exit) {
	    QMLContainer* par = r->parent;
	    QMLNode* n = r->start;
	    QMLRow tr (this, p, n, par, r->width, alignment() );
	    fast_exit &= r->end == tr.end && r->height == tr.height;
	    if (fast_exit) {
		r->dirty = TRUE;
		r->fill = tr.fill;
		return;
	    }
	}
    }

    int oldHeight = height;
    setWidth(p, width, TRUE);

    if (height != oldHeight) { // we have to inform our parent
	QMLBox* b = parentBox();
	if (b){
	    b->update( p ); // TODO SLOW
	}
    }
}

QMLRow* QMLBox::locate(QPainter* p, QMLNode* node, int &lx, int &ly, int &lh, int&lry, int &lrh)
{
	
    QMLRow* row;
    for ( row = rows.first(); row; row = rows.next()) {
	if (row->locate(this, p, node, lx, ly, lh) ) {
	    lry = row->y;
	    lrh = row->height;
	    break;
	}
    }
    if (row) {
	QMLBox* b = parentBox();
	if (b) {
	    int mx, my, mh, mry, mrh;
	    mx = my = mh = mry = mrh = 0;
	    (void) b->locate(p, this, mx, my, mh, mry, mrh);
	    lx += mx;
	    ly += my;
	    lry += my;
	}
    }
    return row;
}

QMLNode* QMLBox::hitTest(QPainter* p, int obx, int oby, int xarg, int yarg)
{
    QMLRow* row;
    QMLNode* result = 0;
    for ( row = rows.first(); row; row = rows.next()) {
	result = row->hitTest(this, p, obx, oby, xarg, yarg);
	if (result)
	    break;
    }
    return result;
}



int QMLBox::numberOfSubBox( QMLBox* subbox)
{
    QMLNode* i = child;
    int n = 1;
    while (i && i != subbox ) {
	i = i->nextSibling();
	n++;
    }
    return n;
}


QMLStyle::ListStyle QMLBox::listStyle()
{
    if ( attributes() ) {
	debug("have attributes");
	QString* s =  attributes()->find("type");
	
	//#### use a nice and fast dict for that
	if ( !s )
	    return style->listStyle();
	else if ( *s == "square" )
	    return QMLStyle::ListSquare;
	else if ( *s == "disc" )
	    return QMLStyle::ListDisc;
	else if ( *s == "circle" )
	    return QMLStyle::ListCircle;
	else if ( *s == "1" )
	    return QMLStyle::ListDecimal;
	else if ( *s == "a" )
	    return QMLStyle::ListLowerAlpha;
	else if ( *s == "A" )
	    return QMLStyle::ListUpperAlpha;
    }
    return style->listStyle();
}


//************************************************************************

class QMLCursor{
public:
    QMLCursor(QMLDocument& doc);
    ~QMLCursor();
    void draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch);

    QMLDocument* document;

    int x;
    int y;
    int height;

    QMLRow* row;
    int rowY;
    int rowHeight;

    int width() { return 1; }

    QMLNode *node;
    QMLContainer *nodeParent;

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
    void end(QPainter* p, bool select = FALSE);
    void goTo(QPainter* p, int xarg, int yarg, bool select = FALSE);


    void goTo(QMLNode* n, QMLContainer* par,  bool select = FALSE);
    void calculatePosition(QPainter* p);

    int xline;
    int yline;
    bool ylineOffsetClean;

private:
    void rightInternal(bool select = FALSE);
    void leftInternal(bool select = FALSE);
};


QMLCursor::QMLCursor(QMLDocument& doc)
{
    document = &doc;
    node = document;
    nodeParent = 0;
    hasSelection = FALSE;
    selectionDirty = FALSE;

    while (node && node->isContainer)
	node = document->depthFirstSearch( node, nodeParent);


    x = y = height = rowY = rowHeight = 0;
    row = 0;
    xline = 0;
    yline = 0;
    ylineOffsetClean = FALSE;
}

QMLCursor::~QMLCursor()
{
}

void QMLCursor::draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch)
{
    if ( QMAX( x, cx ) <= QMIN( x+width(), cx+cw ) &&
	 QMAX( y, cy ) <= QMIN( y+height, cy+ch ) ) {

	p->drawLine(x-ox, y-oy, x-ox, y-oy+height-1);
	// warwick says two-pixels cursor are ugly
	//	p->drawLine(x+1-ox, y-oy, x+1-ox, y-oy+height-1);
    }
}


void QMLCursor::clearSelection()
{
    if (!hasSelection)
	return;

    QMLNode* i = document;
    QMLContainer* ip = 0;
    while (i && i->isContainer)
	i = document->depthFirstSearch( i, ip);

    while ( i ) {
	if (i->isSelected) {
	    i->isSelected = 0;
	    i->isSelectionDirty = 1;
	    QMLBox* b = ip->box();
	    do {
		b->isSelectionDirty = 1;
	    }while ( ( b = b->parentBox() ) );
	}
	i = document->nextLeaf( i, ip );
    }

    selectionDirty = TRUE;
    hasSelection = FALSE;
}

void QMLCursor::goTo(QMLNode* n, QMLContainer* par, bool select)
{
    if (select && node != n){
	selectionDirty = TRUE;
	hasSelection = TRUE;

	QMLNode* other = n;
	QMLContainer* otherParent = par;

	QMLNode* i1 = node;
	QMLContainer* i1p = nodeParent;
	QMLNode* i2 = other;
	QMLContainer* i2p = otherParent;
	
	while (i1 != other && i2 != node){
	    if (i1) i1 = document->nextLeaf(i1, i1p);
	    if (i2) i2 = document->nextLeaf(i2, i2p);
	}
	QMLNode* start = 0;
	QMLContainer* startParent = 0;
	QMLNode* end = 0;
	if (i1 == other) {
	    start = node;
	    startParent = nodeParent;
	    end = other;
	}
	else {
	    start = other;
	    startParent = otherParent;
	    end = node;
	}

	while (start != end ) {
	    start->isSelected = start->isSelected?0:1;
	    QMLBox* b = startParent->box();
	    do {
		b->isSelectionDirty = 1;
	    }while ( ( b = b->parentBox() ) );
	    startParent->box()->isSelectionDirty = 1;
	    start->isSelectionDirty = 1;
	    start = document->nextLeaf( start, startParent );
	}
    }

    node = n;
    nodeParent = par;
}


/*! Set x, y, xline, yline, row, rowY and rowHeight according to the
  current cursor position.  */
void QMLCursor::calculatePosition(QPainter* p)
{
    if (!node || !nodeParent) {
	xline = yline = x = y = 0;
	ylineOffsetClean = FALSE;
	return;
    }
    row = nodeParent->box()->locate(p, node, x, y, height, rowY, rowHeight);
    xline = x;
    yline = y;
    ylineOffsetClean = FALSE;
}

/*!  Move the cursor to the node near the given coordinates. If select
  is TRUE, the nodes in between toggle their selection state.  */
void QMLCursor::goTo(QPainter* p, int xarg, int yarg, bool select)
{
    QMLNode* n = document->hitTest(p, 0, 0, xarg, yarg);
    if (n)
	goTo(n, n->parent(), select);
    calculatePosition(p);
}


/*!
  Insert the given string at the current cursor position.
 */
void QMLCursor::insert(QPainter* p, const QString& s)
{
    if (s.isEmpty())
	return;

    QMLNode* n = new QMLNode;
    n->c = s[0];

    QMLNode* last = n;
    for (unsigned int i = 1; i < s.length(); i++) {
	last->next = new QMLNode;
	last = last->next;
	last->c = s[int(i)];
    }

    if (nodeParent->child == node) {
	last->next = node;
	nodeParent->child = n;
	//	row = 0;
    } else {
	QMLNode* prev = node->previousSibling(); // slow!
	last->next = node;
	prev->next = n;
    }
    QMLBox* b = node->box();
    if (b) {
	if (row && row->start == node){
	    row->start = n;
	}
	b->update(p, row);
    }
    calculatePosition(p);
}


/*!
  Enter key, splits the paragraph.
 */
void QMLCursor::enter(QPainter* p)
{

    nodeParent->split(node);

    QMLBox* b = nodeParent->box();
    b->update(p);
    b->next->box()->update( p );

    nodeParent = node->parent();
    calculatePosition(p);
}


/*!
  Delete c items behind the cursor.
 */
void QMLCursor::del(QPainter* p, int c)
{
    QMLNode* curNode = node;
    QMLContainer* curParent = nodeParent;
    QMLRow* curRow = row;
    bool useCurRow = (curRow->start!=curNode && curRow->end != curNode);

    curRow->dirty = TRUE;

    if (c < 1)
	c = 1;

    QMLBox* nodeBox = 0;
    QMLBox* curBox = 0;

    bool bigUpdate = FALSE;

    for (int i = 0; i < c; i++) {
	curNode = node;
	curParent = nodeParent;

	rightInternal();

	// nothing to do
	if ( node == curNode )
	    return;

	nodeBox = node->box();
	curBox = curNode->box();

	QMLNode* prev = curNode->previousSibling();

	// workaround for empty containers at the end
	if (!prev && curParent != curBox)
	    prev = curParent;

	if (prev && prev->next == curNode) {
	    prev->next = curNode->next;
	    prev->isLastSibling = curNode->isLastSibling;
	}
	if (curParent->child == curNode){
	    if (curNode->isLastSibling)
		curParent->child = 0;
	    else
		curParent->child = curNode->next;
	}

	if ( nodeBox != curBox) {
	    QMLNode* hook = prev;
	    if (hook) {
		// climb up the tree if we are at the end of usual containers
		while (hook->isLastSibling && !hook->next->isBox )
		    hook = hook->next;
	    }

	    // first, disconnect the moving items, then reconnect them
	    // behind the hook

	    // find the target and the lastTarget
	    bool curBoxInNodeBox = curBox->parentBox()==nodeBox;
	    QMLNode* target = curBoxInNodeBox?curBox->next:nodeBox->child;

	    while (target->isBox && ((QMLContainer*)target)->child)
		target = ((QMLContainer*)target)->child;

	    QMLNode* lastTarget;
	    if (!curBoxInNodeBox) {
		lastTarget = target->lastSibling();
	    }
	    else {
		QMLNode* i = target;
		while ( !i->isLastSibling && (i->next && !i->next->isBox ))
		    i = i->next;
		lastTarget = i;
	    }

	    // disconnect
	    QMLNode* prevTarget = target->previousSibling();
	    if (prevTarget) {
		prevTarget->isLastSibling = lastTarget->isLastSibling;
		prevTarget->next = lastTarget->next;
	    } else {
		QMLContainer* targetParent = target->parent();
		targetParent->child = 0;
		if (lastTarget->next != targetParent)
		    targetParent->child = lastTarget->next;
		else {
		    // targetParent is empty => remove
		    QMLNode* targetParentPrev = targetParent->previousSibling();
		    if (targetParentPrev) {
			targetParentPrev->isLastSibling = targetParent->isLastSibling;
			targetParentPrev->next = targetParent->next;
		    }
		    else {
			QMLContainer* targetParentParent = targetParent->parent;
			if (targetParentParent) {
			    if (targetParent->isLastSibling)
				targetParentParent->child = 0;
			    else {
				targetParentParent->child = targetParent->next;
			    }
			}
		    }

		    // TODO das muss doch immmer eine box sein (?)
		    if (targetParent->isBox)
			delete (QMLBox*)targetParent;
		    else
			delete targetParent;
		    nodeBox = 0;
		}
	    }


	    // reconnect
	    if (hook) {
		lastTarget->isLastSibling = hook->isLastSibling;
		lastTarget->next = hook->next;
		hook->next = target;
		hook->isLastSibling = 0;
	    }
	    else { // empty curbox, make the target the first child
		curBox->child = target;
		lastTarget->isLastSibling = 1;
	    }


	    QMLBox* b = curBox->parentBox();
	    if (b)
		b->reparentSubtree();
	    else
		curBox->reparentSubtree();

	    if (nodeBox){
		nodeBox->reparentSubtree();
	    }

	    bigUpdate = TRUE;

	    // recalculate the nodeParent
	    nodeParent = node->parent();
	}
	delete curNode;

	curNode = node;
	curParent = nodeParent;
    }

    if ( !bigUpdate ) {
	curBox->update(p, useCurRow?curRow:0);
    }
    else {
	// big update

	if ( nodeBox )
	    nodeBox->update( p );

	int oldCurBoxHeight = curBox->height;
	curBox->update( p );

	// if the curbox changes height, the parent box gets updated
	// automatically. Otherwise we do it here for safety.
	QMLBox* b = curBox->parentBox();
	if (b && oldCurBoxHeight == curBox->height) {
	    b->update( p );
	}
    }

    calculatePosition(p);

}



/*!
  Delete c items before the cursor.
 */
void QMLCursor::backSpace(QPainter* p, int c)
{
    QMLNode* curNode = node;
    for (int i = 0; i < c; i++)
	leftInternal();
    if ( node == curNode )
	return;
    del(p, c);
}

/*!
  Move the cursor one item to the right
 */
void QMLCursor::right(QPainter* p,bool select)
{
    rightInternal(select);
    calculatePosition(p);
}

/*!
  internal
 */
void QMLCursor::rightInternal(bool select)
{
    QMLContainer* np = nodeParent;
    QMLNode* n = document->nextLeaf(node, np);
    if (n)
	goTo(n, np, select);
}

/*!
  Move the cursor one item to the left
 */
void QMLCursor::left(QPainter* p, bool select)
{
    leftInternal(select);
    calculatePosition(p);
}

/*!
  internal
 */
void QMLCursor::leftInternal(bool select)
{
    QMLContainer* tmpParent = 0;

    QMLContainer* np = nodeParent;
    while (np->parent && document->nextLeaf(np, tmpParent) == node)
	np = np->parent;


    QMLNode* n = 0;
    QMLNode* tmp = np->nextLeaf(np, tmpParent);

    while (tmp != node) {
	n = tmp;
	np = tmpParent;
	tmp = document->nextLeaf(tmp, tmpParent);
    }
    if (n)
	goTo(n, np, select);
}

/*!
  Move the cursor one row up
 */
void QMLCursor::up(QPainter* p, bool select)
{
    QMLNode* tmp = node;
    int ty = rowY - 1;
    while (ty > 0 && (!tmp || tmp == node)) {
	tmp = document->hitTest(p, 0, 0, xline, ty--);
    }
    if (tmp)
	goTo(tmp, tmp->parent(), select );
    int oldXline = xline;
    calculatePosition(p);
    xline = oldXline;
}

/*!
  Move the cursor one row down
 */
void QMLCursor::down(QPainter* p, bool select)
{
    QMLNode* tmp = node;
    int ty = rowY + rowHeight + 1;
    while (ty < document->height && (!tmp || tmp == node)) {
	tmp = document->hitTest(p, 0, 0, xline, ty++);
    }
    if (tmp)
	goTo(tmp, tmp->parent(), select );
    int oldXline = xline;
    calculatePosition(p);
    xline = oldXline;
}

/*!
  Home key
 */
void QMLCursor::home(QPainter* p, bool select)
{
    goTo(row->start, row->parent, select );
    calculatePosition(p);
}

/*!
  End key
 */
void QMLCursor::end(QPainter* p, bool select)
{
    goTo(row->end, row->end->parent(), select );
    calculatePosition(p);
}


//************************************************************************


/*!
  \class QMLProvider qml.h
  \brief Provides a collection of QML documents

  Since QML documents link to each other, a view which displays QML will
  often need access to other documents so as to follow links, etc.
  A QMLProvider actually knows nothing about QML - it is just a repository
  of documents and images which QML documents happen to use.

  The default implementation gives no meaning to the names of documents
  it provides, beyond that provided by setPath().  However, the image()
  and document() functions are virtual, so a subclass could interpret
  the names as filenames, URLs, or whatever.  

  \sa QMLView::setProvider()
*/

/*!
  Create a provider.  Like any QObject, the created object will be
  deleted when its parent destructs (if the child still exists then).
 */
QMLProvider::QMLProvider(  QObject *parent, const char *name )
    : QObject( parent, name )
{
    images.setAutoDelete( TRUE );
    documents.setAutoDelete( TRUE );
}

/*!
  Destroys the provider.
 */
QMLProvider::~QMLProvider()
{
}


static QMLProvider* defaultprovider = 0;

static void cleanup_provider()
{
    delete defaultprovider;
}

/*!
  Returns the application-wide default provider.
 */
QMLProvider& QMLProvider::defaultProvider()
{
    if (!defaultprovider)
	defaultprovider = new QMLProvider;
    qAddPostRoutine(cleanup_provider);
    return *defaultprovider;
}

/*!
  Sets the default provider, destroying any previously set provider.
 */
void QMLProvider::setDefaultProvider( QMLProvider* provider)
{
    delete defaultprovider;
    defaultprovider = provider;
}


/*!
  Binds the given name to a pixmap. The pixmap can be accessed with
  image().
*/
void QMLProvider::setImage(const QString& name, const QPixmap& pm)
{
    images.insert(name, new QPixmap(pm));
}

/*!
  Returns the image corresponding to \a name.

  \sa setImage()
*/
QPixmap QMLProvider::image(const QString &name) const
{
    QPixmap* p = images[name];
    if (p)
	return *p;
    else
	return QPixmap( searchPath + "/" + name );
}

/*!
  Binds the \a name to the document contents \a doc.
  The document can then be accessed with document().
*/
void QMLProvider::setDocument(const QString &name, const QString& doc)
{
    documents.insert(name, new QString(doc));
}

/*!
  Returns the document contents corresponding to \a name.
  \sa setDocument()
*/
QString QMLProvider::document(const QString &name) const
{
    QString* s = documents[name];
    if (s)
	return *s;
    {
	QFile f (searchPath + "/" + name);
	QString d;
	if ( f.open( IO_ReadOnly ) ) {
	    QTextStream t( &f );
	    d = t.read();
	    f.close();
	}
	return d;
    }
}


/*! 
  If an item cannot be found in the definitions set by setDocument()
  and setImage(), the default implementations of image() and document()
  will try to load it from the local filesystem in the single directory
  specified with setPath().  

  \sa path()
 */
void QMLProvider::setPath( const QString &path )
{
     searchPath = path;
}

/*!
  Returns the current search path.

  \sa setPath()
 */
QString QMLProvider::path() const
{
    return searchPath;
}



//************************************************************************


QMLDocument::QMLDocument( const QString &doc, const QWidget* w)
    :QMLBox( (base = new QMLStyle("")) )
{
    provider_ = &QMLProvider::defaultProvider(); // for access during parsing only
    sheet_ = &QMLStyleSheet::defaultSheet();// for access during parsing only
    init( doc, w );
    provider_ = 0;
}

QMLDocument::QMLDocument( const QString &doc, const QMLProvider& provider, const QWidget* w)
    :QMLBox( (base = new QMLStyle("")) )
{
    provider_ = &provider; // for access during parsing only
    sheet_ = &QMLStyleSheet::defaultSheet();// for access during parsing only
    init( doc, w );
}

QMLDocument::QMLDocument(const QString &doc,  const QMLProvider& provider,
			 const QMLStyleSheet& sheet, const QWidget* w )
    :QMLBox( (base = new QMLStyle("")) )
{

    provider_ = &provider; // for access during parsing only
    sheet_ = &sheet; // for access during parsing only
    init( doc, w );
}

void QMLDocument::init( const QString& doc, const QWidget* w )
{
    //set up base style
    base->setDisplayMode(QMLStyle::DisplayInline);
    QFont f = w?w->font(): *QApplication::font();
    base->setFontFamily( f.family() );
    base->setFontItalic( f.italic() );
    base->setFontWeight( f.weight() );
    base->setFontSize( f.pointSize() );

    valid = TRUE;
    openChar = new QChar('<');
    closeChar = new QChar('>');
    slashChar = new QChar('/');
    quoteChar = new QChar('"');
    equalChar = new QChar('=');
    int pos = 0;
    parse(this, 0, doc, pos);
}

QMLDocument::~QMLDocument()
{
    delete openChar;
    delete closeChar;
    delete slashChar;
    delete quoteChar;
}



void QMLDocument::dump()
{
}



bool QMLDocument::isValid() const
{
    return valid;
}


void QMLDocument::parse (QMLContainer* current, QMLNode* lastChild, const QString &doc, int& pos)
{
    //     eatSpace(doc, pos);
    while ( valid && pos < int(doc.length() )) {
	bool sep = FALSE;
	if (hasPrefix(doc, pos, *openChar) ){
	    if (hasPrefix(doc, pos+1, *slashChar)) {
		if (current->isBox){ // todo this inserts a hitable null character
		    QMLNode* n = new QMLNode;
		    n->c = QChar::null;
		    QMLNode* l = lastChild;
		    if (!l)
			current->child = n;
		    else {
			l->isLastSibling = 0;
			l->next = n;
		    }
		    n->next = current;
		    n->isLastSibling = 1;
		    lastChild = n;
		    l = n;
		}
		return;
	    }
	    QDict<QString> attr;
	    attr.setAutoDelete( TRUE );
	    QString tagname = parseOpenTag(doc, pos, attr);
	    sep = eatSpace(doc, pos);
	    QMLNode* tag = sheet_->tag(tagname, attr, *provider_);
	    if (tag->isContainer ) {
		if (current == this && !attr.isEmpty() ) {
		    setAttributes( attr );
		}
		QMLContainer* ctag = (QMLContainer*) tag;
		valid &= ctag != 0;
		if (valid) {
		    QMLNode* l = lastChild;
		    if (!l){
			current->child  = ctag;
			ctag->isLastSibling = 1;
		    }
		    else {
			l->next = ctag;
			l->isLastSibling = 0;
		    }
		
		    ctag->parent = current; //TODO
		    ctag ->next = current;
		    ctag->isLastSibling = 1;
		    lastChild = ctag;
		
		    parse(ctag, 0, doc, pos);
		    sep |= eatSpace(doc, pos);
		    valid = (hasPrefix(doc, pos, *openChar)
			     && hasPrefix(doc, pos+1, *slashChar)
			     && eatCloseTag(doc, pos, tagname) );
		    if (!valid)
			return;
		    if ( ctag->isBox ) // no whitespace between boxes
			sep |= eatSpace(doc, pos);
		}
	    }
	    else { // empty tags
		if (valid) {
		    sep |= eatSpace(doc, pos);
		    QMLNode* l = lastChild;
		    if (!l){
			current->child  = tag;
			tag->isLastSibling = 1;
		    }
		    else {
			l->next = tag;
			l->isLastSibling = 0;
		    }
		    tag ->next = current;
		    tag->isLastSibling = 1;
		    lastChild = tag;
		}
	    }
	}
	else {
	    QString word = parsePlainText(doc, pos);
	    if (valid){
		QMLNode* l = lastChild;
		for (int i = 0; i < int(word.length()); i++){
		    QMLNode* n = new QMLNode;
		    n->c = word[i];
		    if (!l)
			current->child = n;
		    else {
			l->isLastSibling = 0;
			l->next = n;
		    }
		    n->next = current;
		    n->isLastSibling = 1;
		    lastChild = n;
		    l = n;
		}
		sep |= eatSpace(doc, pos);
	    }
	}
    }

}

bool QMLDocument::eatSpace(const QString& doc, int& pos)
{
    int old_pos = pos;
    while (pos < int(doc.length()) && doc[pos].isSpace())
	pos++;
    return old_pos < pos;
}

bool QMLDocument::eat(const QString& doc, int& pos, const QChar& c)
{
    valid &= (bool) (doc[pos] == c);
    if (valid)
	pos++;
    return valid;
}

bool QMLDocument::lookAhead(const QString& doc, int& pos, const QChar& c)
{
    return (doc[pos] == c);
}


QString QMLDocument::parseWord(const QString& doc, int& pos, bool lower)
{
    QString s;

    if (doc[pos] == *quoteChar) {
	pos++;
	while ( pos < int(doc.length()) && doc[pos] != *quoteChar ) {
	    s += doc[pos];
	    pos++;
	}
	eat(doc, pos, *quoteChar);
    }
    else {
	while( pos < int(doc.length()) &&
	       doc[pos] != *closeChar && doc[pos] != *openChar
	       && doc[pos] != *equalChar
	       && !doc[pos].isSpace())  {
	    s += doc[pos];
	    pos++;
	}
	if (lower)
	    s = s.lower();
    }
    valid &= pos <= int(doc.length());

    return s;
}

QString QMLDocument::parsePlainText(const QString& doc, int& pos)
{
    QString s;
    while( pos < int(doc.length()) &&
	   doc[pos] != *closeChar && doc[pos] != *openChar ) {
	if (doc[pos].isSpace()){
	    while (pos+1 < int(doc.length() ) && doc[pos+1].isSpace() ){
		pos++;
	    }
	}
	s += doc[pos];
	pos++;
    }
    valid &= pos <= int(doc.length());
    return s;
}


bool QMLDocument::hasPrefix(const QString& doc, int pos, const QChar& c)
{
    return valid && doc[pos] ==c;
}

QString QMLDocument::parseOpenTag(const QString& doc, int& pos,
				  QDict<QString> &attr)
{
    pos++;
    QString tag = parseWord(doc, pos, TRUE);
    eatSpace(doc, pos);

    while (valid && !lookAhead(doc, pos, *closeChar) ) {
	QString key = parseWord(doc, pos, TRUE);
	eatSpace(doc, pos);
	if (eat(doc, pos, *equalChar)) {
	    eatSpace(doc, pos);
	    QString value = parseWord(doc, pos, TRUE);
	    attr.insert(key, new QString(value) );
	    eatSpace(doc, pos);
	}
    }

    eat(doc, pos, *closeChar);
    return tag;
}

bool QMLDocument::eatCloseTag(const QString& doc, int& pos, const QString& open)
{
    pos++;
    pos++;
    QString tag = parseWord(doc, pos, TRUE);
    eatSpace(doc, pos);
    eat(doc, pos, *closeChar);
    valid &= tag == open;
    return valid;
}




//************************************************************************

/*!
  depthFirstSearch traversal for the tag tree
 */
inline QMLNode* QMLNode::depthFirstSearch(QMLNode* tag, QMLContainer* &parent, bool down)
{
    if (down) {
	if (tag->isContainer && ((QMLContainer*)tag)->child){
	    parent = (QMLContainer*)tag;
	    return ((QMLContainer*)tag)->child;
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
	QMLContainer* p = (QMLContainer*)tag->next;
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

inline QMLNode* QMLNode::nextLayout(QMLNode* tag, QMLContainer* &parent){
    QMLNode* t;

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

inline QMLNode* QMLNode::nextLeaf(QMLNode* tag, QMLContainer* &parent){
    do {
	tag = depthFirstSearch(tag, parent);

    } while (tag && tag->isContainer);

    return tag;
}



inline QMLNode* QMLNode::lastSibling() const
{
    QMLNode* n = (QMLNode*) this;

    while (n && !n->isLastSibling)
	n = n->next;
    return n;
}

inline QMLContainer* QMLNode::parent() const
{
    if (isContainer)
	return ((QMLContainer*)this)->parent;
    else {
	QMLNode* n = lastSibling();
	if (n) return (QMLContainer*)n->next;
    }
    return 0;
}

inline QMLBox* QMLNode::box() const
{
    QMLContainer* par = parent();
    if (!par)
	return 0;
    else
	return par->box();
}

inline QMLNode* QMLNode::previousSibling() const
{
    QMLContainer* par = parent();
    QMLNode* result = par->child;
    if (result == this)
	return 0;
    while (result->next && result->next != this)
	result = result->next;
    return result;
}


inline QMLNode* QMLNode::nextSibling() const
{
    if (isLastSibling)
	return 0;
    return next;
}
//************************************************************************


/*!
  \class QMLView qml.h
  \brief A sophisticated single-page QML viewer.

  Unlike QMLSimpleDocument, which merely draws small pieces of QML,
  a QMLView is a real widget, with scrollbars when necessary, for showing
  large QML documents.

  For even more, see QMLBrowser.
*/ 

struct QMLViewData
{
    const QMLStyleSheet* sheet_;
    QMLDocument* doc_;
    const QMLProvider* provider_;
    QString txt;
    QColorGroup mypapcolgrp;
    QColorGroup papcolgrp;
};


/*!
  Constructs an empty QMLView
  with the standard \a parent and \a name optional arguments.
*/
QMLView::QMLView(QWidget *parent, const char *name)
    : QScrollView( parent, name)
{
    init();
}


/*!
  Constructs a QMLView displaying the contents \a doc,
  with the standard \a parent and \a name optional arguments.
*/
QMLView::QMLView( const QString& doc, QWidget *parent, const char *name)
    : QScrollView( parent, name)
{
    init();
    d->txt = doc;
}


void QMLView::init()
{
    d = new QMLViewData;
    d->mypapcolgrp = palette().normal();
    d->papcolgrp = d->mypapcolgrp;

    setKeyCompression( TRUE );
    setVScrollBarMode( QScrollView::Auto );
    setHScrollBarMode( AlwaysOff );

    d->doc_ = 0;
    d->sheet_ = 0;
    d->provider_ = 0;
    d->txt = "<p></p>";

    viewport()->setBackgroundMode(NoBackground);
    setFocusPolicy( StrongFocus );
}

/*!
  Destructs the view.
*/
QMLView::~QMLView()
{
    delete d->doc_;
    delete d->sheet_;
    delete d;
}

/*!
  Changes the contents of the view to the string \a doc.

  \sa contents()
*/
void QMLView::setContents( const QString& doc)
{
    delete d->doc_;
    d->doc_ = 0;
    d->txt = doc;
    if ( d->txt.isEmpty() )
	d->txt = "<p></p>";
    if ( isVisible() ) {
	{
	    QPainter p( this );
	    currentDocument().setWidth(&p, viewport()->width());
	}
	resizeContents(currentDocument().width, currentDocument().height);
	viewport()->update();
	viewport()->setCursor( arrowCursor );
    }
}


/*!
  Returns the contents of the view.

  \sa setContents()
*/
QString QMLView::contents() const
{
    return d->txt;
}


void QMLView::createDocument()
{
    d->papcolgrp = d->mypapcolgrp;
    d->doc_ = new QMLDocument( d->txt, provider(), styleSheet(), viewport() );
    d->doc_->border = 5;
    if ( !d->doc_->attributes() )
	return;
    if (d->doc_->attributes()->find("bgcolor")){
	QColor  col ( d->doc_->attributes()->find("bgcolor")->ascii() );
	if ( col.isValid() )
	    d->papcolgrp.setColor( QColorGroup::Base, col );
    }
    if (d->doc_->attributes()->find("text")){
	QColor  col ( d->doc_->attributes()->find("text")->ascii() );
	if ( col.isValid() )
	    d->papcolgrp.setColor( QColorGroup::Text,  col );
    }
    if (d->doc_->attributes()->find("bgpixmap")){
	QPixmap pm = provider().image(*d->doc_->attributes()->find("bgpixmap"));
	if (!pm.isNull())
	    d->papcolgrp.setBrush( QColorGroup::Base, QBrush(d->papcolgrp.base(), pm) );
    }
}


/*!
  Returns the current style sheet of the view.

  \sa setStyleSheet()
*/
const QMLStyleSheet& QMLView::styleSheet() const
{
    if (!d->sheet_)
	return QMLStyleSheet::defaultSheet();
    else
	return *d->sheet_;

}

/*!
  Sets the style sheet of the view.

  \sa styleSheet()
*/
void QMLView::setStyleSheet( const QMLStyleSheet* styleSheet )
{
    d->sheet_ = styleSheet;
}


/*!
  Returns the current provider for the view.

  \sa setProvider()
*/
const QMLProvider& QMLView::provider() const
{
    if (!d->provider_)
	return QMLProvider::defaultProvider();
    else
	return *d->provider_;

}

/*!
  Sets the provider for the view.

  \sa provider()
*/
void QMLView::setProvider( const QMLProvider* newProvider )
{
    d->provider_ = newProvider;
}


/*!
  Sets the brush to use as the background to \a pap.

  \sa paper()
*/
void QMLView::setPaper( const QBrush& pap)
{
    d->mypapcolgrp.setBrush( QColorGroup::Base, pap );
    d->papcolgrp.setBrush( QColorGroup::Base, pap );
}

/*!
  Sets the full colorgroup of the background to \a colgrp.
*/
void QMLView::setPaperColorGroup( const QColorGroup& colgrp)
{
    d->mypapcolgrp = colgrp;
    d->papcolgrp = colgrp;
}

/*!
  Returns the colorgroup of the background.
*/
const QColorGroup& QMLView::paperColorGroup() const
{
    return d->papcolgrp;
}


/*!
  Returns the document title parsed from the content.
*/
QString QMLView::documentTitle() const
{
    QString* s = currentDocument().attributes()?currentDocument().attributes()->find("title"):0;
    if (s)
	return *s;
    else
	return QString();
}

/*!
  Returns the height of the view given a width of \a w.
*/
int QMLView::heightForWidth( int w ) const
{
    QMLDocument doc ( d->txt, provider(), styleSheet(), viewport() );
    {
	QPainter p( this );
	doc.setWidth(&p, w);
    }
    return doc.height;
}

/*!
  Returns the current document defining the view.  This is not currently
  useful for applications.
*/
QMLDocument& QMLView::currentDocument() const
{
    if (!d->doc_){
	QMLView* that = (QMLView*) this;
	that->createDocument();
    }
    return *d->doc_;
}

/*!
  Returns the brush used to paint the background.
*/
const QBrush& QMLView::paper()
{
    return d->papcolgrp.brush( QColorGroup::Base );
}

/*!
  \override
*/
void QMLView::drawContentsOffset(QPainter* p, int ox, int oy,
				 int cx, int cy, int cw, int ch)
{
    QRegion r(cx-ox, cy-oy, cw, ch);
    currentDocument().draw(p, 0, 0, ox, oy, cx, cy, cw, ch, r, paperColorGroup(), &paper() );

    p->setClipRegion(r);

    if ( paper().pixmap() )
	p->drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			   *paper().pixmap(), ox, oy);
    else
	p->fillRect(0, 0, viewport()->width(), viewport()->height(), paper() );

    qApp->syncX();

    p->setClipping( FALSE );
}

/*!
  \override
*/
void QMLView::viewportResizeEvent(QResizeEvent* )
{
    {
	QPainter p( this );
	currentDocument().setWidth(&p, viewport()->width());
    }
    resizeContents(currentDocument().width, currentDocument().height);
    viewport()->update();
}

/*!
  \override
*/
void QMLView::viewportMousePressEvent( QMouseEvent* )
{
}

/*!
  \override
*/
void QMLView::viewportMouseReleaseEvent( QMouseEvent* )
{
}

/*!
  \override
*/
void QMLView::viewportMouseMoveEvent( QMouseEvent* )
{
}

/*!
  Provides scrolling and paging.
*/
void QMLView::keyPressEvent( QKeyEvent * e)
{
    switch (e->key()) {
    case Key_Right:
	break;
    case Key_Left:
	break;
    case Key_Up:
	scrollBy( 0, -10 );
	break;
    case Key_Down:
	scrollBy( 0, 10 );
	break;
    case Key_Home:
	setContentsPos(0,0);
	break;
    case Key_End:
	setContentsPos(0,contentsHeight()-viewport()->height());
	break;
    case Key_PageUp:
	scrollBy( 0, -viewport()->height() );
	break;
    case Key_PageDown:
	scrollBy( 0, viewport()->height() );
	break;
    }
}

/*!
  \override
*/
void QMLView::paletteChange( const QPalette & )
{
    d->mypapcolgrp = palette().normal();
}


//************************************************************************

#if 0
QMLEdit::QMLEdit(QWidget *parent, const char *name)
    : QMLView( parent, name )
{
    setKeyCompression( TRUE );
    setVScrollBarMode( AlwaysOn );
    cursor_hidden = FALSE;
    cursorTimer = new QTimer( this );
    cursorTimer->start(200, TRUE);
    connect( cursorTimer, SIGNAL( timeout() ), this, SLOT( cursorTimerDone() ));

    cursor = 0;

}

QMLEdit::~QMLEdit()
{
}



/*!
  reimplemented for internal purposes
 */
void QMLEdit::setContents( const QString& contents)
{
    QMLView::setContents( contents );
    delete cursor;
    cursor = new QMLCursor(currentDocument());
}

/*!
  Make a tree dump
 */
QString QMLEdit::contents()
{
    debug("not yet implemented");
    return "not yet implemented";
}

void QMLEdit::keyPressEvent( QKeyEvent * e)
{

//     if (e->key() == Key_Plus)
// 	exit(2); // profiling


    hideCursor();
    bool select = e->state() & Qt::ShiftButton;
#define CLEARSELECT if (!select) {cursor->clearSelection();updateSelection();}


    if (e->key() == Key_Right
	|| e->key() == Key_Left
	|| e->key() == Key_Up
	|| e->key() == Key_Down
	|| e->key() == Key_Home
	|| e->key() == Key_End
	|| e->key() == Key_PageUp
	|| e->key() == Key_PageDown
	) {
	// cursor movement
	CLEARSELECT
	    QMLRow*  oldCursorRow = cursor->row;
	bool ensureVisibility = TRUE;
	{
	    QPainter p( viewport() );
	    switch (e->key()) {
	    case Key_Right:
		cursor->right(&p, select);
		p.end();
		break;
	    case Key_Left:
		cursor->left(&p, select);
		p.end();
		break;
	    case Key_Up:
		cursor->up(&p, select);
		p.end();
		break;
	    case Key_Down:
		cursor->down(&p, select);
		p.end();
		break;
	    case Key_Home:
		cursor->home(&p, select);
		p.end();
		break;
	    case Key_End:
		cursor->end(&p, select);
		p.end();
		break;
	    case Key_PageUp:
		p.end();
		ensureVisibility = FALSE;
		{
		    int oldContentsY = contentsY();
		    if (!cursor->ylineOffsetClean)
			cursor->yline-=oldContentsY;
		    scrollBy( 0, -viewport()->height() );
		    if (oldContentsY == contentsY() )
			break;
		    p.begin(viewport());
		    int oldXline = cursor->xline;
		    int oldYline = cursor->yline;
		    cursor->goTo( &p, oldXline, oldYline +  1 + contentsY(), select);
		    cursor->xline = oldXline;
		    cursor->yline = oldYline;
		    cursor->ylineOffsetClean = TRUE;
		    p.end();
		}
		break;
	    case Key_PageDown:
		p.end();
		ensureVisibility = FALSE;
		{
		    int oldContentsY = contentsY();
		    if (!cursor->ylineOffsetClean)
			cursor->yline-=oldContentsY;
		    scrollBy( 0, viewport()->height() );
		    if (oldContentsY == contentsY() )
			break;
		    p.begin(viewport());
		    int oldXline = cursor->xline;
		    int oldYline = cursor->yline;
		    cursor->goTo( &p, oldXline, oldYline + 1 + contentsY(), select);
		    cursor->xline = oldXline;
		    cursor->yline = oldYline;
		    cursor->ylineOffsetClean = TRUE;
		    p.end();
		}
		break;
	    }
	}
	if (cursor->row == oldCursorRow)
	    updateSelection(cursor->rowY, cursor->rowY);
	else
	    updateSelection();
	if (ensureVisibility) {
	    ensureVisible(cursor->x, cursor->y);
	}
	showCursor();
    }
    else {
	
	if (e->key() == Key_Return || e->key() == Key_Enter ) {
	    CLEARSELECT
		{
		    QPainter p( viewport() );
		    for (int i = 0; i < QMIN(4, e->count()); i++)
			cursor->enter( &p ); // can be optimized
		}
	    updateScreen();
	}
	else if (e->key() == Key_Delete) {
	    CLEARSELECT
		{
		    QPainter p( viewport() );
		    cursor->del( &p, QMIN(4, e->count() ));
		}
	    updateScreen();
	}
	else if (e->key() == Key_Backspace) {
	    CLEARSELECT
		{
		    QPainter p( viewport() );
		    cursor->backSpace( &p, QMIN(4, e->count() ) );
		}
	    updateScreen();
	}
	else if (!e->text().isEmpty() ){
	    CLEARSELECT
		{
		    QPainter p( viewport() );
		    cursor->insert( &p, e->text() );
		}
	    updateScreen();
	}
    }
}


/* Updates the visible selection according to the internal
  selection. If oldY and newY is defined, then only the area between
  both horizontal lines is taken into account. */
void QMLEdit::updateSelection(int oldY, int newY)
{
    if (!cursor || !cursor->selectionDirty)
	return;

    if (oldY > newY) {
	int tmp = oldY;
	oldY = newY;
	newY = tmp;
    }

    QPainter p(viewport());
    int minY = oldY>=0?QMAX(QMIN(oldY, newY), contentsY()):contentsY();
    int maxY = newY>=0?QMIN(QMAX(oldY, newY), contentsY()+viewport()->height()):contentsY()+viewport()->height();
    QRegion r;
    currentDocument().draw(&p, 0, 0, contentsX(), contentsY(),
			   contentsX(), minY,
			   viewport()->width(), maxY-minY,
			   r, paperColorGroup(), &paper(), FALSE, TRUE);
    cursor->selectionDirty = FALSE;
}

void QMLEdit::viewportMousePressEvent( QMouseEvent * e)
{
    hideCursor();
    cursor->clearSelection();
    updateSelection();
    {
	QPainter p( viewport() );
	cursor->goTo( &p, contentsX() + e->x(), contentsY() + e->y());
    }
    showCursor();
}

void QMLEdit::viewportMouseReleaseEvent( QMouseEvent * )
{
    // nothing
}

void QMLEdit::viewportMouseMoveEvent( QMouseEvent * e)
{
    if (e->state() & LeftButton) {
	hideCursor();
	QMLRow*  oldCursorRow = cursor->row;
	{
	    QPainter p(viewport());
	    cursor->goTo( &p, e->pos().x() + contentsX(),
			  e->pos().y() + contentsY(), TRUE);
	}
	if (cursor->row == oldCursorRow)
	    updateSelection(cursor->rowY, cursor->rowY );
	else
	    updateSelection();
	if (cursor->y + cursor->height > contentsY() + viewport()->height()) {
	    scrollBy(0, cursor->y + cursor->height-contentsY()-viewport()->height());
	}
	else if (cursor->y < contentsY())
	    scrollBy(0, cursor->y - contentsY() );
	showCursor();
    }
}

void QMLEdit::drawContentsOffset(QPainter*p, int ox, int oy,
				 int cx, int cy, int cw, int ch)
{
    QMLView::drawContentsOffset(p, ox, oy, cx, cy, cw, ch);
    if (!cursor_hidden)
	cursor->draw(p, ox, oy, cx, cy, cw, ch);
}

void QMLEdit::cursorTimerDone()
{
    if (cursor_hidden) {
	if (QMLEdit::hasFocus())
	    showCursor();
	else
	    cursorTimer->start(400, TRUE);
    }
    else {
	hideCursor();
    }
}

void QMLEdit::showCursor()
{
    cursor_hidden = FALSE;
    QPainter p( viewport() );
    cursor->draw(&p, contentsX(), contentsY(),
		 contentsX(), contentsY(),
		 viewport()->width(), viewport()->height());
    cursorTimer->start(400, TRUE);
}

void QMLEdit::hideCursor()
{
    if (cursor_hidden)
	return;
    cursor_hidden = TRUE;
    repaintContents(cursor->x, cursor->y,
		    cursor->width(), cursor->height);
    cursorTimer->start(300, TRUE);
}



/*!  Updates the visible screen according to the (changed) internal
  data structure.
*/
void QMLEdit::updateScreen()
{
    {
	QPainter p( viewport() );
	QRegion r(0, 0, viewport()->width(), viewport()->height());
	currentDocument().draw(&p, 0, 0, contentsX(), contentsY(),
			       contentsX(), contentsY(),
			       viewport()->width(), viewport()->height(),
			       r, paperColorGroup(), &paper(), TRUE);
	p.setClipRegion(r);
	if ( paper().pixmap() )
	    p.drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			      *paperColorGroup().brush( QColorGroup::Base ).pixmap(), contentsX(), contentsY());
	else
	    p.fillRect(0, 0, viewport()->width(), viewport()->height(), paper());
    }
    showCursor();
    resizeContents(currentDocument().width, currentDocument().height);
    ensureVisible(cursor->x, cursor->y);
}

void QMLEdit::viewportResizeEvent(QResizeEvent* e)
{
    QMLView::viewportResizeEvent(e);
    {
	QPainter p( this );
	cursor->calculatePosition(&p);
    }
}

#endif
//************************************************************************



/*!
  \class QMLBrowser qml.h
  \brief A QML browser with simple navigation.

  This class is the same as the QMLView it inherits, with the addition
  that it provides basic navigation features to follow links in QML
  documents that link to other QML documents.

  This class doesn't provide actual Back and Forward buttons, but it
  has backward() and forward() slots that implement the functionality.

  By using QMLView::setProvider(), you can provide your own subclass
  of QMLProvider, to access QML data from anywhere you need to.

  For simpler QML use, see QMLView or QMLSimpleDocument.
*/ 

struct QMLBrowserData
{
    QString searchPath;
    const QMLContainer* buttonDown;
    const QMLContainer* highlight;
    QPoint lastClick;
    QStack<QString> stack;
    QStack<QString> forwardStack;
    QString home;
};


/*!
  Constructs an empty QMLBrowser.
*/
QMLBrowser::QMLBrowser(QWidget *parent, const char *name)
    : QMLView( parent, name )
{
    d = new QMLBrowserData;

    viewport()->setMouseTracking( TRUE );
    d->buttonDown = 0;
    d->highlight = 0;
}

/*!
  Destructs the browser.
*/
QMLBrowser::~QMLBrowser()
{
    delete d;
}


/*!
  Sets the document with the given \a name to be displayed.  The name
  is looked up in the provider() of the browser.

  If the first tag in the document is "<qml type=detail>", it is displayed as
  a popup rather than as a new document.
*/
void QMLBrowser::setDocument(const QString& name)
{
    if ( d->home.isNull() )
	d->home = name;

    QString doc = provider().document( name );
    if ( isVisible() ) {
	QString firstTag = doc.left( doc.find('>' )+1 );
	QMLDocument tmp( firstTag );
	if (tmp.attributes() && tmp.attributes()->find("type") && *tmp.attributes()->find("type") == "detail" ) {
	    popupDetail( doc, d->lastClick );
	    return;
	}
    }

    if ( d->stack.isEmpty() || *d->stack.top() != name) {
	emit backwardAvailable( !d->stack.isEmpty() );
	d->stack.push(new QString( name ) );
    }
		
    setContents( doc );
}

/*!
  Sets the contents of the browser to \a contents, and emits the
  contentsChanged() signal.
*/
void QMLBrowser::setContents( const QString& contents )
{
    QMLView::setContents( contents );
    emit contentsChanged();
}

/*!
  \fn void QMLBrowser::backwardAvailable(bool available)
  This signal is emitted when the availability of the backward()
  changes.  It becomes available when the user navigates forward,
  and unavailable when the user is at the home().
*/

/*!
  \fn void QMLBrowser::forwardAvailable(bool available)
  This signal is emitted when the availability of the forward()
  changes.  It becomes available after backward() is activated,
  and unavailable when the user navigates or goes forward() to
  the last navigated document.
*/

/*!
  \fn void QMLBrowser::highlighted (const QString &href)
  This signal is emitted when the user has selected but not activated
  a link in the document.  \a href is the value of the href tag
  in the link.
*/

/*!
  \fn void QMLBrowser::contentsChanged()
  This signal is emitted whenever the setContents() changes the
  contents (eg. because the user clicked on a link).
*/

/*!
  \fn void QMLBrowser::highlighted (const QString &)
  This signal is emitted when the user has selected but not activated
  a link in the document.
*/

/*!
  Changes the document displayed to be the previous document
  in the list of documents build by navigating links.

  \sa forward(), backwardAvailable()
*/
void QMLBrowser::backward()
{
    if ( d->stack.count() <= 1)
	return;
    d->forwardStack.push( d->stack.pop() );
    QString* ps = d->stack.pop();
    setDocument( *ps );
    delete ps;
    emit forwardAvailable( TRUE );
}

/*!
  Changes the document displayed to be the next document
  in the list of documents build by navigating links.

  \sa backward(), forwardAvailable()
*/
void QMLBrowser::forward()
{
    if ( d->forwardStack.isEmpty() )
	return;
    QString* ps = d->forwardStack.pop();
    emit forwardAvailable( !d->forwardStack.isEmpty() );
    setDocument( *ps );
    delete ps;
}

/*!
  Changes the document displayed to be the first document the
  browser displayed.
*/
void QMLBrowser::home()
{
    if (!d->home.isNull() )
	setDocument( d->home );
}

/*!
  Add Backward and Forward on ALT-Left and ALT-Right respectively.
*/
void QMLBrowser::keyPressEvent( QKeyEvent * e )
{
    if ( e->state() & AltButton ) {
	switch (e->key()) {
	case Key_Right:
	    forward();
	    return;
	case Key_Left:
	    backward();
	    return;
	case Key_Up:
	    home();
	    return;
	}
    }
    QMLView::keyPressEvent(e);
}

/*!
  \e override to press anchors.
*/
void QMLBrowser::viewportMousePressEvent( QMouseEvent* e )
{
    if ( e->button() == LeftButton ) {
	d->buttonDown = anchor( e->pos() );
	d->lastClick = e->globalPos();
    }
}

/*!
  \e override to activate anchors.
*/
void QMLBrowser::viewportMouseReleaseEvent( QMouseEvent* e )
{
    if ( e->button() == LeftButton ) {
	if (d->buttonDown && d->buttonDown == anchor( e->pos() )){
	    QString href;
	    if ( d->buttonDown->attributes() && d->buttonDown->attributes()->find("href"))
		href = *d->buttonDown->attributes()->find("href");
	    setDocument( href );
	}
    }
    d->buttonDown = 0;
}

/*!
  Activate to emit highlighted().
*/
void QMLBrowser::viewportMouseMoveEvent( QMouseEvent* e)
{
    const QMLContainer* act = anchor( e->pos() );
    if (d->highlight != act) {
	QString href;
	viewport()->setCursor( act?upArrowCursor:arrowCursor );
	if (act && act->attributes() && act->attributes()->find("href"))
	    href = *act->attributes()->find("href");
	emit highlighted( href );
    }
    d->highlight = act;
}


const QMLContainer* QMLBrowser::anchor( const QPoint& pos)
{
    QPainter p( viewport() );
    QMLNode* n = currentDocument().hitTest(&p, 0, 0,
					   contentsX() + pos.x(),
					   contentsY() + pos.y());
    if (n)
	return n->parent()->anchor();
    return 0;
}


class QMLDetailPopup : public QFrame
{
public:
    QMLDetailPopup( const QString& txt)
	: QFrame ( 0, 0, WType_Popup )
	{
	    setFrameStyle( QFrame::Box | QFrame::Plain );
	    QMLView* view = new QMLView( this );
	    view->setVScrollBarMode( QScrollView::Auto );
	    view->setContents( txt );

	    int h = view->heightForWidth( 400 );

	    if ( h < height () ) {
		resize( 400 + 2 * frameWidth(), h + 2 * frameWidth() );
	    }
	    view->setGeometry( contentsRect() );
	    view->viewport()->installEventFilter( this );

	}

protected:
    void hideEvent( QHideEvent * ) //###### need real popup function, see qwhatsthis
	{
	    delete this;
	}

    bool eventFilter( QObject* ,QEvent* e)
	{
	    if (e->type() == QEvent::MouseButtonPress ){
		hide();
		return TRUE;
	    }
	    return FALSE;
	}
};


void QMLBrowser::popupDetail( const QString& contents, const QPoint& pos )
{
    QMLDetailPopup* popup = new QMLDetailPopup( contents );

    //###### we need a global fancy popup positioning somewhere
    popup->move(pos - popup->rect().center());
    if (popup->geometry().right() > QApplication::desktop()->width())
	popup->move( QApplication::desktop()->width() - popup->width(),
		     popup->y() );
    if (popup->geometry().bottom() > QApplication::desktop()->height())
	popup->move( popup->x(),
		     QApplication::desktop()->height() - popup->height() );
    if ( popup->x() < 0 )
	popup->move( 0, popup->y() );
    if ( popup->y() < 0 )
	popup->move( popup->x(), 0 );

    popup->show();
}



/*!
  \class QMLSimpleDocument qml.h
  \brief A small displayable piece of QML.

  This class encapsulates simple QML usage where a string is interpretted
  as QML and can be drawn.

  For large documents, see QMLView or QMLBrowser.

  \sa QLabel::setQML(), QMLView
*/

struct QMLSimpleDocumentData
{
    QMLDocument* doc;
};

/*!
  Constructs a QMLSimpleDocument from the QML \a contents.

  If \a w is
  not 0, its properties (font, etc.) are used to set default
  properties for the display. No reference is kept to the widget.

  Once created, changes cannot be made (just throw it away and make
  a new one with the new contents).
*/
QMLSimpleDocument::QMLSimpleDocument( const QString& contents, const QWidget* w)
{
    d  = new QMLSimpleDocumentData;
    d->doc = new QMLDocument( contents, w );
}

/*!
  Destructs the document, freeing memory.
*/
QMLSimpleDocument::~QMLSimpleDocument()
{
    delete d->doc;
    delete d;
}

/*!
  Sets the width of the document to \a w pixels, recalculating the layout
  as if it were to be drawn with \a p.  ####### QPaintDevice

  \sa height()
*/
void QMLSimpleDocument::setWidth( QPainter* p, int w)
{
    d->doc->setWidth( p, w );
}

int QMLSimpleDocument::width() const
{
    return d->doc->width;
}

/*!
  Returns the height of the document, in pixels.
  \sa setWidth()
*/
int QMLSimpleDocument::height() const
{
    return d->doc->height;
}

/*!
  Draws the formatted QML with \a p, at position (\a x,\a y), clipped to
  \a clipRegion.  Colors from \a cg are used as needed, and if not 0,
  *\a paper is used as the background brush.

  Note that the display code is highly optimized to reduce flicker, so
  passing a brush for \a paper is preferrable to simply clearing the area
  to be painted and then calling this without a brush.
*/
void QMLSimpleDocument::draw( QPainter* p,  int x, int y, const QRegion& clipRegion,
			      const QColorGroup& cg, const QBrush* paper) const
{
    QRect r = clipRegion.boundingRect();
    QRegion bg = clipRegion;

    d->doc->draw(p, x, y, 0, 0, r.x(), r.y(), r.width(), r.height(), bg, cg, paper);
    if (paper) {
	p->setClipRegion(bg);
	if ( paper->pixmap() )
	    p->drawTiledPixmap( r, *paper->pixmap());
	else
	    p->fillRect(r, *paper);
	p->setClipping( FALSE );
    }
}
