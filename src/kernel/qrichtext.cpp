/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrichtext.cpp#17 $
**
** Implementation of the Qt classes dealing with rich text
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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
#include "qstylesheet.h"
#include "qrichtextintern.cpp"
#include <qapplication.h>
#include <qlayout.h>
#include <qpainter.h>

#include <qstack.h>
#include <stdio.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qimage.h>
#include <qdragobject.h>



QTextIterator::~QTextIterator()
{
}


QTextIterator QTextIterator::next() const
{
    if ( !node )
	return QTextIterator( node );

    if ( node->isContainer && (( QTextContainer*) node)->child )
	return QTextIterator(  (( QTextContainer*) node)->child, ( QTextContainer*) node);
    if ( node->next &&  !node->isLastSibling )
	return QTextIterator( node->next, par );
    QTextNode* i = node->next;
    while ( i && i->isLastSibling ) {
	i = i->next;
    }
    if ( i )
	return QTextIterator( i->next, ((QTextContainer*)i)->parent );
    return 0;
}


QTextIterator QTextIterator::operator++ (int)
{
    QTextIterator tmp = next(); // not really faster but doesn't matter
    node = tmp.node;
    par = tmp.par;
    return *this;
}

QTextIterator& QTextIterator::operator++ ()
{
    QTextIterator tmp = next();
    node = tmp.node;
    par = tmp.par;
    return *this;
}

QTextNode::QTextNode()
{
    next = 0;
    isSimpleNode = 1;
    isLastSibling = 0;
    isContainer = 0;
    isBox = 0;
    isRoot = 0;
    isSelected = 0;
    isSelectionDirty = 0;
}


QTextNode::~QTextNode()
{
}




//************************************************************************

QTextCustomNode::QTextCustomNode()
    :QTextNode()
{
    isSimpleNode = 0;
    width = height = 0;
}

QTextCustomNode::~QTextCustomNode()
{
}


bool QTextCustomNode::expandsHorizontally()
{
    return FALSE;
}


QTextImage::QTextImage(const QMap<QString, QString> &attr, const QString& context,
		       const QMimeSourceFactory &factory)
    : QTextCustomNode()
{
    width = height = 0;
    if ( attr.contains("width") )
	width = attr["width"].toInt();
    if ( attr.contains("height") )
	height = attr["height"].toInt();

    reg = 0;
    QImage img;
    QString imageName = attr["source"];

    if (!imageName)
	imageName = attr["src"];

    if ( !imageName.isEmpty() ) {
	const QMimeSource* m = 0;
	m = factory.data( imageName, context );
	if ( !m ) {
	    qWarning("QTextImage: no mimesource for %s", imageName.latin1() );
	}
	else {
	    if ( !QImageDrag::decode( m, img ) ) {
		qWarning("QTextImage: cannot decode %s", imageName.latin1() );
	    }
	}
    }

    if ( !img.isNull() ) {
	if ( width == 0 )
	    width = img.width();
	if ( height == 0 )
	    height = img.height();

	if ( img.width() != width || img.height() != height ){
	    img = img.smoothScale(width, height);
	    width = img.width();
	    height = img.height();
	}
	pm.convertFromImage( img );
	if ( pm.mask() ) {
	    QRegion mask( *pm.mask() );
	    QRegion all( 0, 0, pm.width(), pm.height() );
	    reg = new QRegion( all.subtract( mask ) );
	}
    }

    if ( pm.isNull() && (width*height)==0 ) {
	width = height = 50;
    }
}


QTextImage::~QTextImage()
{
}


void QTextImage::draw(QPainter* p, int x, int y,
		    int ox, int oy, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/,
		    QRegion& backgroundRegion, const QColorGroup& cg, const QBrush* /*bg*/)
{
    if ( pm.isNull() ) {
	p->fillRect( x-ox , y-oy, width, height,  cg.dark() );
	return;
    }
    if ( reg ){
	QRegion tmp( *reg );
	tmp.translate( x-ox, y-oy );
	backgroundRegion = backgroundRegion.unite( tmp );
    }
    p->drawPixmap( x-ox , y-oy, pm );
}



QTextHorizontalLine::QTextHorizontalLine(const QMap<QString, QString> &, const QMimeSourceFactory&)
{
    height = 8;
    width = 200;
}


QTextHorizontalLine::~QTextHorizontalLine()
{
}


bool QTextHorizontalLine::expandsHorizontally()
{
    return TRUE;
}

void QTextHorizontalLine::draw(QPainter* p, int x, int y,
			     int ox, int oy, int cx, int cy, int cw, int ch,
			     QRegion&, const QColorGroup&, const QBrush* paper)
{
    QRect rm( x-ox, y-oy, width, height);
    QRect ra( cx-ox, cy-oy, cw,  ch);
    QRect r = rm.intersect( ra );
    if (paper) {
	if ( paper->pixmap() )
	    p->drawTiledPixmap( r, *paper->pixmap(), QPoint(r.x()+ox, r.y()+oy) );
	else
	    p->fillRect(r, *paper );
    }
    QPen pen(p->pen());
    pen.setWidth( 2 );
    p->setPen( pen );
    p->drawLine( r.left()-1, y-oy+4, r.right()+1, y-oy+4) ;
}


//************************************************************************

QTextRow::QTextRow()
{
    x = y = width = height = base = 0;
    fill = 0;
    first = last = 0;
    parent = 0;

    dirty = TRUE;
}

QTextRow::QTextRow( QPainter* p, QFontMetrics &fm,
		    QTextIterator& it, int w, int& min, int align)
{
    x = y = width = height = base = 0;
    dirty = TRUE;

    width = w;

    QTextIterator end = it.parentNode()->box()->end();
    while ( it != end  && !it->isBox && it->isContainer) {
	++it;
    }	

    first = *it;
    parent = it.parentNode();

    int tx = 0;
    int rh = 0;
    int rasc = 0;
    int rdesc = 0;

    if ( first->isBox ) {
	QTextBox* b = (QTextBox*)first;
	b->setWidth(p, width );
	width = QMAX( b->widthUsed, width );
	height = b->height;
	base = height;
	last = first;
	it = b->end();
	min = b->widthUsed;
	return;
    }

    // do word wrap
    QTextIterator lastSpace = it;
    int lastHeight = rh;
    int lastWidth = 0;
    int lastAsc = rasc;
    int lastDesc = rdesc;
    bool noSpaceFound = TRUE; // TRUE;

    QTextIterator prev = it;


    while ( it != end ) {
	int h,a,d;
	if (it->isSimpleNode) {
	    if ( it.parentNode()->font() != p->font() ) {
		p->setFont( it.parentNode()->font() );
	    }
	    if (!it->isNull())
		tx += fm.width(it->c);
	    h = fm.height();
	    a = fm.ascent();
	    d = h-a;
	}
	else if ( !it->isContainer ) {
	    QTextCustomNode* c = (QTextCustomNode*)*it;
	    if ( c->expandsHorizontally() ) {
		c->width = width - fm.width(' ');
	    }
	    tx +=c->width;
	    h = c->height;
	    a = h;
	    d = 0;
	}
	else if ( it->isContainer ){
	    if ( it->isBox )
		break;
	    else {
		++it;
		continue;
	    }
	}
	
	if (tx > width - fm.width(' ') && !noSpaceFound  && !it->isSpace() )
// 	if (tx > width - fm.width(' ') && *it != first && !it->isSpace() )
	    break;

	rh = QMAX( rh, h );
	rasc = QMAX( rasc, a );
	rdesc = QMAX( rdesc, d );
	
	prev = it;
	++it;
	
	// break (a) after a space, (b) before a box, (c) if we have
	// to or (d) at the end of a box.
	if (prev->isSpace() || prev->isNewline() ||
	    it == end || it->isBox || noSpaceFound ){
	    lastSpace = prev;
	    lastHeight = rh;
	    lastAsc = rasc;
	    lastDesc = rdesc;
	    lastWidth = tx;
 	    if (noSpaceFound && prev->isSpace())
 		noSpaceFound = FALSE;
	}
	if ( prev->isNewline() )
	    break;
    }

    last = *lastSpace;

    rh = lastHeight;
    rasc = lastAsc;
    rdesc = lastDesc;

    height = QMAX(rh, rasc+rdesc);
    base = rasc;

    if (align == Qt::AlignCenter)
	fill = (width - lastWidth) / 2;
    else if (align == Qt::AlignRight)
	fill = width - lastWidth;
    else
	fill = 0;

    it = lastSpace;

    if ( lastWidth > width ) {
	width = lastWidth;
	fill = 0;
    }

    min = lastWidth;

    ++it;

}



QTextRow::~QTextRow()
{
}



QTextIterator QTextRow::begin() const
{
    return QTextIterator( first, parent );
}

QTextIterator QTextRow::end() const
{
    QTextIterator it( last, 0 );
    ++it;
    return it;
}

void QTextRow::draw( QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, const QBrush* paper,
		  bool onlyDirty, bool onlySelection)
{

    if (!intersects(cx-obx, cy-oby, cw,ch)) {
	dirty = FALSE;
	return;
    }


    if (first->isBox) {
	//we have to draw the box
	((QTextBox*)first)->draw(p, obx+x, oby+y, ox, oy, cx, cy, cw, ch,
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

    bool reducedFlickerMode = FALSE;
    QTextIterator it;
    // reduced flicker mode is broken, does not deal with shearing and bearing yet
//     for ( it = begin(); it != end(); ++it ) {
// 	if ( it->isCustomNode() && paper ) {
//  	    reducedFlickerMode = TRUE;
// 	    break;
// 	}
//     }

    if ( !reducedFlickerMode ) {
	if (!onlyDirty && !onlySelection && paper) {
	    if ( paper->pixmap() )
		p->drawTiledPixmap(x+obx-ox, y+oby-oy, width, height, *paper->pixmap(), x+obx, y+oby);
	    else
		p->fillRect(x+obx-ox, y+oby-oy, width, height, *paper);
	}
    }




    bool inFirst = TRUE;

    int tx = x;

    if ( fill > 0 )
	tx += fill;

    static QString s;

    QFontMetrics fm = p->fontMetrics();
    for ( it = begin(); it != end(); ++it ) {
	if ( it->isContainer )
	    continue;
	s.truncate(0);
	QFont font = it.parentNode()->font();
	if ( font != p->font() ) {
	    p->setFont( font );
	    fm = p->fontMetrics();
	}
	int tw = 0;
	QTextNode* tmp = 0;
	bool select = it->isSelected;
	bool selectionDirty = it->isSelectionDirty;
	it->isSelectionDirty = 0;
	if (it->isSimpleNode) {
	    if (!it->isNull()){
		s += it->c;
		tw += fm.width( it->c );
	    }
	    // special optimized code for simple nodes (characters)
	    while ( *it != last && (tmp = it->nextSibling() ) && tmp->isSimpleNode
		    && ((bool)tmp->isSelected) == select
		    && ((bool) tmp->isSelectionDirty) == selectionDirty
		    && it->isSimpleNode
		    ) {
		it = QTextIterator( tmp, it.parentNode() );
		tmp->isSelectionDirty = 0;
		if (!it->isNull()) {
		    s += it->c;
		    tw += fm.width( it->c );
		}
		// 	    if (it->isSpace())
		// 	      break;
	    }
	}
	else {
	    // custom nodes
	    tw += ((QTextCustomNode*)*it)->width;
	}
	

	if (!onlySelection || selectionDirty) {
	    p->setPen( it.parentNode()->color(cg.text()) );
	
	    if (select) {
		if ( inFirst )
		    p->fillRect(x+obx-ox, y+oby-oy, tw+(tx-x), height, cg.highlight() );
		if (*it ==last)
		    p->fillRect(tx+obx-ox, y+oby-oy, width-(tx-x), height, cg.highlight());
		else
		    p->fillRect(tx+obx-ox, y+oby-oy, tw, height, cg.highlight());
		p->setPen( cg.highlightedText() );
	    }
	    else if ( (onlyDirty || onlySelection || (reducedFlickerMode && it->isSimpleNode)) && paper ) {
		int txo = 0;
		if ( inFirst ) {
		    if ( paper->pixmap() )
			p->drawTiledPixmap(x+obx-ox, y+oby-oy, tw+(tx-x), height,
					   *paper->pixmap(), x+obx, y+oby);
		    else
			p->fillRect(x+obx-ox, y+oby-oy, tw+(tx-x), height, *paper );
		    txo = tw;
		}
		if (*it == last){
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
	
	    if (it->isSimpleNode) {
		// Get rid of garbage at last of line (\n etc. at visible
		// on Windows. ### Matthias: Fix in parser?
 		int len = s.length();
 		if ( len > 0 && s[len-1] < (char)32 ) {
 		   len--;
 		}
		p->drawText(tx+obx-ox, y+oby-oy+base, s, len);
	    }
	    else {
		if ( reducedFlickerMode ) {
		    if (!it->isSelected) {
			if ( inFirst ) {
			    if ( paper->pixmap() )
				p->drawTiledPixmap(x+obx-ox, y+oby-oy, (tx-x), height,
						   *paper->pixmap(), x+obx, y+oby);
			    else
				p->fillRect(x+obx-ox, y+oby-oy, (tx-x), height, *paper);
			}
			if ( *it == last){
			    if ( paper->pixmap() )
				p->drawTiledPixmap(tx+tw+obx-ox, y+oby-oy, width-(tx-x)-tw, height,
						   *paper->pixmap(), tx+obx+tw, y+oby);
			    else
				p->fillRect(tx+tw+obx-ox, y+oby-oy, width-(tx-x)-tw, height, *paper);
			}
		    }
		    int h = ((QTextCustomNode*)*it)->height;
		    if ( h < height && !it->isSelected ) {
			if ( paper->pixmap() )
			    p->drawTiledPixmap(tx+obx-ox, y+oby-oy, tw, base-h,
					       *paper->pixmap(), tx+obx, y+oby);
			else
			    p->fillRect(tx+obx-ox, y+oby-oy, tw, base-h, *paper);
			if ( paper->pixmap() )
			    p->drawTiledPixmap(tx+obx-ox, y+oby-oy+base, tw, height-base,
					       *paper->pixmap(), tx+obx, y+oby+base);
			else
			    p->fillRect(tx+obx-ox, y+oby-oy+base, tw, height-base, *paper);
		    }
 		    ((QTextCustomNode*)*it)->draw(p,tx+obx,y+oby+base-h,
						  ox, oy, cx, cy, QMIN(tx+obx+width,cw), ch, backgroundRegion, cg, paper);
		
		    if ( it->isSelected ) {
			QRect tr( tx+obx-ox, y+oby-oy+base-h, tw, h );
			backgroundRegion = backgroundRegion.subtract( tr );
		    }
		}
		else {
		    int h = ((QTextCustomNode*)*it)->height;
		    ((QTextCustomNode*)*it)->draw(p,tx+obx,y+oby+base-h,
					      ox, oy, cx, cy, cw, ch, backgroundRegion, cg, paper);
		}
	    }
	}
	tx += tw;
	inFirst = FALSE;
    };

}

QTextNode* QTextRow::hitTest(QPainter* p, int obx, int oby, int xarg, int yarg)
{
  QFontMetrics fm = p->fontMetrics();
    if (!intersects(xarg-obx, yarg-oby, 0,0))
	return 0;

    if (first->isBox) {
	return ((QTextBox*)first)->hitTest(p, obx+x, oby+y, xarg, yarg);
    }
    QTextIterator it;
    int tx = fill;
    for ( it = begin(); it != end(); ++it ) {
	if ( it->isContainer && !it->isBox )
	    continue;
	p->setFont( it.parentNode()->font() );
	if (it->isSimpleNode)
	    tx += fm.width( it->c );
	else
	    tx += ((QTextCustomNode*)*it)->width;
	if (obx + x + tx > xarg)
	    break;
    }
    return *it;

}

bool QTextRow::locate(QPainter* p, QTextNode* node, int &lx, int &ly, int &lh)
{
    if (first->isBox) { // a box row
	if (node == first) {
	    lx = x;
	    ly = y;
	    lh = height;
	    return TRUE;
	}
	return FALSE;
    }

    QTextIterator it;
    for ( it = begin(); it != end() && *it != node; ++it )
	;
    if ( *it != node )
	return FALSE; // nothing found

    lx = x + fill;
    QFontMetrics fm = p->fontMetrics();
    for ( it = begin(); *it != node; ++it ) {
	if ( !it->isContainer || it->isBox )
	    continue;
	p->setFont( it.parentNode()->font() );
	fm = p->fontMetrics();
	if (it->isSimpleNode)
	    lx += fm.width( it->c );
	else
	    lx += ((QTextCustomNode*)*it)->width;
    };
    p->setFont( it.parentNode()->font() );
    fm = p->fontMetrics();
    ly = y + base - fm.ascent();
    lh = fm.height();

    return TRUE;
}



//************************************************************************
QTextContainer::QTextContainer( const QStyleSheetItem *stl)
    : style(stl)
{
    isSimpleNode = 0;
    isContainer = 1;
    fnt = 0;
    parent = 0;
    child = 0;
    attributes_ = 0;
}

QTextContainer::QTextContainer( const QStyleSheetItem *stl, const QMap<QString, QString> &attr )
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

void QTextContainer::setAttributes(const QMap<QString, QString> &attr )
{
    delete attributes_;
    attributes_ = new QMap<QString, QString>( attr );
}

QTextContainer::~QTextContainer()
{
    delete fnt;
    delete attributes_;
    QTextNode* nx = 0;
    for (QTextNode* n = child; n; n = nx) {
	if (n->isLastSibling)
	    nx = 0;
	else
	    nx = n->next;
	
	if (n->isBox)
	    delete (QTextBox*)n;
	else if (n->isContainer)
	    delete (QTextContainer*)n;
	else if (n->isSimpleNode)
	    delete n;
	else
	    delete (QTextCustomNode*)n;
    }
}


QTextContainer* QTextContainer::copy() const
{
    QTextContainer* result = new QTextContainer( style );
    if (attributes_)
	result->setAttributes( *attributes_ );
    return result;
}

void QTextContainer::split(QTextNode* node)
{
    QTextContainer* c2 = copy();

    QTextNode* prev = node->previousSibling(); // slow!
    if (!node->isContainer) {
	QTextNode* n = new QTextNode;
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


const QMap<QString, QString> * QTextContainer::attributes() const
{
    return attributes_;
}

const QTextContainer* QTextContainer::anchor() const
{
    if ( style->isAnchor() )
	return this;
    if ( parent )
	return parent->anchor();
    return 0;
}


QTextContainer* QTextContainer::findAnchor(const QString& name ) const
{
    if (style->isAnchor() && attributes() &&
	attributes()->contains("name") && attributes()->operator[]("name") == name)
	return (QTextContainer*)this;

    QTextNode* n = child;
    while( n ) {
	if (n->isContainer) {
	    QTextContainer* t = ((QTextContainer*)n)->findAnchor( name );
	    if (t)
		return t;
	}
	n = n->nextSibling();
    }
    return 0;
}

QTextBox* QTextContainer::box() const
{
    QTextContainer* result = (QTextContainer*) this;
    while (result && !result->isBox)
	result = result->parent;
    return (QTextBox*)result;
}

QTextBox* QTextContainer::parentBox() const
{
    QTextContainer* result = (QTextContainer*) parent;
    while (result && !result->isBox)
	result = result->parent;
    return (QTextBox*)result;
}


QTextIterator QTextContainer::begin() const
{
    return QTextIterator( this );
}

QTextIterator QTextContainer::end() const
{
    if ( !isLastSibling )
	return QTextIterator( next, parent );
    QTextContainer* i = parent;
    while ( i && i->isLastSibling)
	i = i->parent;
    if ( i )
	return QTextIterator( i->next );
    return 0;
}



QTextNode* QTextContainer::lastChild() const
{
    if (!child)
	return 0;
    return child->lastSibling();
}


void QTextContainer::reparentSubtree()
{
    QTextNode* n = child;
    while (n) {
	if (n->isContainer) {
	    delete  ((QTextContainer*)n)->fnt;
	    ((QTextContainer*)n)->fnt = 0;
	    ((QTextContainer*)n)->parent = this;
	    ((QTextContainer*)n)->reparentSubtree();
	}
	if (n->isLastSibling) {
	    n->next = this;
	    break;
	}
	n = n->next;
    }
}




void QTextContainer::createFont()
{
    // fnt is used to cache these values, therefore
    // use a temporary QFont* here
    QFont* f = new QFont( fontFamily() );
    f->setPointSize( fontSize() );
    f->setWeight( fontWeight() );
    f->setItalic( fontItalic() );
    f->setUnderline( fontUnderline() );
    fnt = f;
}



int QTextContainer::fontWeight() const
{
    if ( fnt )
      return fnt->weight();
    int w = style->fontWeight();
    if ( w == QStyleSheetItem::Undefined && parent )
	w = parent->fontWeight();
    return w;
}

bool QTextContainer::fontItalic() const
{
    if ( fnt )
      return fnt->italic();
    if ( style->definesFontItalic() )
      return style->fontItalic();
    return parent? parent->fontItalic() : FALSE;
}

bool QTextContainer::fontUnderline() const
{
    if ( fnt )
      return fnt->underline();
    if ( style->definesFontUnderline() ) {
      if ((style->isAnchor()  && ( !attributes()  || !attributes()->contains("href") ) ) )
	return FALSE;
      return style->fontUnderline();
    }
    return parent? parent->fontUnderline() : FALSE;
}

int QTextContainer::fontSize() const
{
    if ( fnt )
      return fnt->pointSize();
    int w = style->fontSize();
    if ( w == -1 && parent )
	w = parent->fontSize();
    if ( style->fontSizeRelative() != 100 )
	w = w * style->fontSizeRelative() / 100;
    return w;
}

QString QTextContainer::fontFamily() const
{
    if ( fnt )
      return fnt->family();
    QString f = style->fontFamily();
    if ( f.isNull() && parent )
	f = parent->fontFamily();
    return f;
}

//************************************************************************

QTextBox::QTextBox( const QStyleSheetItem *stl)
    :QTextContainer(stl)
{
    rows.setAutoDelete(TRUE);
    isSimpleNode = 0;
    isBox = 1;
    width = height = widthUsed = 0;
}

QTextBox::QTextBox( const QStyleSheetItem *stl, const QMap<QString, QString> &attr )
    :QTextContainer(stl, attr)
{
    rows.setAutoDelete(TRUE);
    isSimpleNode = 0;
    isBox = 1;
    width = height = widthUsed = 0;
}

QTextContainer* QTextBox::copy() const
{
    QTextBox* result = new QTextBox( style );
    return result;
}

QTextBox::~QTextBox()
{
}




#define IN16BIT(x) QMAX( (2<<15)-1, x)

void QTextBox::draw(QPainter *p,  int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, const QBrush* paper,
		  bool onlyDirty, bool onlySelection)
{
    if (onlySelection && !isSelectionDirty)
	return;
    isSelectionDirty = 0;

    if ( !onlySelection && style->displayMode() == QStyleSheetItem::DisplayListItem && rows.first()) {
	QTextRow* row = rows.first();
	QRect r (obx-ox + row->x - 25, oby-oy + row->y, 25, row->height); //#### label width
	if (paper) {
 	    if ( paper->pixmap() )
 		p->drawTiledPixmap( r, *paper->pixmap(), QPoint(r.x()+ox, r.y()+oy) );
 	    else
		p->fillRect(r, *paper );
	}
	
	QTextBox* b = parentBox();
	QStyleSheetItem::ListStyle s = b?b->listStyle():QStyleSheetItem::ListDisc;
	
	switch ( s ) {
	case QStyleSheetItem::ListDecimal:
	case QStyleSheetItem::ListLowerAlpha:
	case QStyleSheetItem::ListUpperAlpha:
	    {
		int n = 1;
		if ( b )
		    n = b->numberOfSubBox( this, TRUE );
		QString l;
		switch ( s ) {
		case QStyleSheetItem::ListLowerAlpha:
		    if ( n < 27 ) {
			l = QChar( ('a' + (char) (n-1)));
			break;
		    }
		case QStyleSheetItem::ListUpperAlpha:
		    if ( n < 27 ) {
			l = QChar( ('A' + (char) (n-1)));
			break;
		    }
		    break;
		default:  //QStyleSheetItem::ListDecimal:
		    l.setNum( n );
		    break;
		}
		l += QString::fromLatin1(". ");
		p->setFont( font() );
		p->drawText( r, Qt::AlignRight|Qt::AlignVCenter, l);
	    }
	    break;
	case QStyleSheetItem::ListSquare:
	    {
		QRect er( r.right()-10, r.center().y()-1, 6, 6);
		p->fillRect( er , cg.brush( QColorGroup::Foreground ) );
	    }
	    break;
	case QStyleSheetItem::ListCircle:
	    {
		QRect er( r.right()-10, r.center().y()-1, 6, 6);
		p->drawEllipse( er );
	    }
	    break;
	case QStyleSheetItem::ListDisc:
	default:
	    {
		p->setBrush( cg.brush( QColorGroup::Foreground ));
		QRect er( r.right()-10, r.center().y()-1, 6, 6);
		p->drawEllipse( er );
		p->setBrush( Qt::NoBrush );
	    }
	    break;
	}
	
	backgroundRegion = backgroundRegion.subtract( r );
    }



    for (QTextRow* row = rows.first(); row; row = rows.next()) {
	row->draw(p, obx, oby, ox, oy, cx, cy, cw, ch, backgroundRegion, cg, paper, onlyDirty, onlySelection);
    }

}


void QTextBox::setWidth( QPainter* p, int newWidth, bool forceResize )
{
    if (newWidth == width && !forceResize) // no need to resize
	return;

    if (style->displayMode() == QStyleSheetItem::DisplayNone) {
	height = 0;
	return;
    }

    QList<QTextRow> oldRows;
    if ( newWidth == width ){
	// reduce flicker by storing the old rows.
	oldRows = rows;
	rows.setAutoDelete( FALSE );
	oldRows.setAutoDelete( TRUE );
    }
    rows.clear();
    rows.setAutoDelete( TRUE );

    width = newWidth;
    widthUsed = 0;
    height = 0;

    int label_offset = 0;
    if ( style->displayMode() == QStyleSheetItem::DisplayListItem )
	label_offset = 25; //### hardcoded

    int ncols = numberOfColumns();
    int colwidth = newWidth / ncols;
    if (colwidth < 10)
	colwidth = 10;

    QTextRow* row = 0;

    int margintop = margin( QStyleSheetItem::MarginTop );
    int marginbottom = margin( QStyleSheetItem::MarginBottom );
    int marginleft = margin( QStyleSheetItem::MarginLeft );
    int marginright = margin( QStyleSheetItem::MarginRight );
    int marginhorizontal = marginright + marginleft;
    int h = margintop;

    QFontMetrics fm = p->fontMetrics();
    QTextIterator it = begin();
    ++it;
    if ( it != end() ) {
	p->setFont( it.parentNode()->font() );
	int min = 0;
	while ( it != end() ) {
	    row = new QTextRow(p, fm, it,
			       colwidth-marginhorizontal - label_offset, min,
			       alignment() );
	    rows.append(row);
	    row->x = marginleft + label_offset;
	    row->y = h;
	    h += row->height;
	    widthUsed = QMAX( widthUsed , min + marginhorizontal + label_offset);
	}
    }

    height = h;

    // adapt colwidth in case some rows didn't fit
    widthUsed *= ncols;
    colwidth = QMAX( width, widthUsed) / ncols;
    if (colwidth < 10)
 	colwidth = 10;

    if (!oldRows.isEmpty() || ncols > 1 ) {
	// do multi columns if required. Also check with the old rows to
	// optimize the refresh

	row = rows.first();
	QTextRow* old = oldRows.first();
	height = 0;
	h /= ncols;
	for (int col = 0; col < ncols; col++) {
	    int colheight = margintop;
	    for (; row && colheight < h; row = rows.next()) {
		row->x = col  * colwidth + marginleft + label_offset;
		row->y = colheight;
		
		colheight += row->height;
		
		if ( old) {
		    if ( row->first->isBox ) {
			// do not check a height change of box rows!
			if ( old->first == row->first &&
			     old->last == row->last &&
			     old->width == old->width &&
			     old->x == row->x &&
			     old->y == row->y ) {
			    row->dirty = old->dirty;
			}
		    } else if ( old->first == row->first &&
				old->last == row->last &&
				old->height == row->height &&
				old->width == old->width &&
				old->x == row->x &&
				old->y == row->y ) {
			row->dirty = old->dirty;
		    }
		    old = oldRows.next();
		}
	    }
	    height = QMAX( height, colheight );
	}
    }

    // collapse the bottom margin
    if ( isLastSibling && parent && parent->isBox){
	// ignore bottom margin
    } else if ( !isLastSibling && next && next->isBox ) {
	// collapse
	height += QMAX( ((QTextContainer*)next)->style->margin( QStyleSheetItem::MarginTop), marginbottom);
    } else {
	// nothing to collapse
        height += marginbottom;
    }
}


void QTextBox::update(QPainter* p, QTextRow* r)
{
    if ( r ) { // optimization
	QTextRow* row;
	QTextRow* prev = 0;
	
	//todo drop QList and connect the rows directly *sigh*
	for ( row = rows.first(); row && row != r; row = rows.next()) {
	    prev = row;
	}
	bool fast_exit = TRUE;
	QFontMetrics fm = p->fontMetrics();
	if (prev) {
	    int min = 0;
	    QTextIterator it( prev->first, prev->parent );
	    QTextRow tr (p, fm, it, prev->width, min);
	    fast_exit = fast_exit && prev->last == tr.last;
	}
	if (fast_exit) {
	    int min = 0;
	    QTextIterator it( r->first, r->parent );
	    QTextRow tr (p, fm, it,  r->width, min, alignment() );
	    widthUsed = QMAX( widthUsed, min * numberOfColumns() );
	    fast_exit &= r->last == tr.last && r->height == tr.height;
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
	QTextBox* b = parentBox();
	if (b){
	    b->update( p ); // TODO SLOW
	}
    }
}

QTextRow* QTextBox::locate(QPainter* p, QTextNode* node, int &lx, int &ly, int &lh, int&lry, int &lrh)
{
	
    QTextRow* row;
    for ( row = rows.first(); row; row = rows.next()) {
	if (row->locate(p, node, lx, ly, lh) ) {
	    lry = row->y;
	    lrh = row->height;
	    break;
	}
    }
    if (row) {
	QTextBox* b = parentBox();
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

QTextNode* QTextBox::hitTest(QPainter* p, int obx, int oby, int xarg, int yarg)
{
    QTextRow* row;
    QTextNode* result = 0;
    for ( row = rows.first(); row; row = rows.next()) {
	result = row->hitTest(p, obx, oby, xarg, yarg);
	if (result)
	    break;
    }
    return result;
}


int QTextBox::numberOfSubBox( QTextBox* subbox, bool onlyListItems)
{
    QTextNode* i = child;
    int n = 1;
    while (i && i != subbox ) {
	if (!onlyListItems || (i->isBox && ((QTextBox*)i)->style->displayMode() == QStyleSheetItem::DisplayListItem) )
	    n++;
	i = i->nextSibling();
    }
    if (i)
	return n;
    return 1;
}


QStyleSheetItem::ListStyle QTextBox::listStyle()
{
    if ( attributes() ) {
	QString s =  attributes()->operator[]("type");
	
	if ( !s )
	    return style->listStyle();
	else {
	    QCString sl = s.latin1();
	    if ( sl == "1" )
		return QStyleSheetItem::ListDecimal;
	    else if ( sl == "a" )
		return QStyleSheetItem::ListLowerAlpha;
	    else if ( sl == "A" )
		return QStyleSheetItem::ListUpperAlpha;
	    sl = sl.lower();
	    if ( sl == "square" )
		return QStyleSheetItem::ListSquare;
	    else if ( sl == "disc" )
		return QStyleSheetItem::ListDisc;
	    else if ( sl == "circle" )
		return QStyleSheetItem::ListCircle;
	}
    }
    return style->listStyle();
}


//************************************************************************
#if 0
class QTextCursor{
public:
    QTextCursor(QRichText& doc);
    ~QTextCursor();
    void draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch);

    QRichText* document;

    int x;
    int y;
    int height;

    QTextRow* row;
    int rowY;
    int rowHeight;

    int width() { return 1; }

    QTextNode *node;
    QTextContainer *nodeParent;

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


    void goTo(QTextNode* n, QTextContainer* par,  bool select = FALSE);
    void calculatePosition(QPainter* p);

    int xline;
    int yline;
    bool ylineOffsetClean;

private:
    void rightInternal(bool select = FALSE);
    void leftInternal(bool select = FALSE);
};


QTextCursor::QTextCursor(QRichText& doc)
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

QTextCursor::~QTextCursor()
{
}

void QTextCursor::draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch)
{
    if ( QMAX( x, cx ) <= QMIN( x+width(), cx+cw ) &&
	 QMAX( y, cy ) <= QMIN( y+height, cy+ch ) ) {

	p->drawLine(x-ox, y-oy, x-ox, y-oy+height-1);
	// warwick says two-pixels cursor are ugly
	//	p->drawLine(x+1-ox, y-oy, x+1-ox, y-oy+height-1);
    }
}


void QTextCursor::clearSelection()
{
    if (!hasSelection)
	return;

    QTextNode* i = document;
    QTextContainer* ip = 0;
    while (i && i->isContainer)
	i = document->depthFirstSearch( i, ip);

    while ( i ) {
	if (i->isSelected) {
	    i->isSelected = 0;
	    i->isSelectionDirty = 1;
	    QTextBox* b = ip->box();
	    do {
		b->isSelectionDirty = 1;
	    }while ( ( b = b->parentBox() ) );
	}
	i = document->nextLeaf( i, ip );
    }

    selectionDirty = TRUE;
    hasSelection = FALSE;
}

void QTextCursor::goTo(QTextNode* n, QTextContainer* par, bool select)
{
    if (select && node != n){
	selectionDirty = TRUE;
	hasSelection = TRUE;

	QTextNode* other = n;
	QTextContainer* otherParent = par;

	QTextNode* i1 = node;
	QTextContainer* i1p = nodeParent;
	QTextNode* i2 = other;
	QTextContainer* i2p = otherParent;
	
	while (i1 != other && i2 != node){
	    if (i1) i1 = document->nextLeaf(i1, i1p);
	    if (i2) i2 = document->nextLeaf(i2, i2p);
	}
	QTextNode* first = 0;
	QTextContainer* firstParent = 0;
	QTextNode* last = 0;
	if (i1 == other) {
	    first = node;
	    firstParent = nodeParent;
	    last = other;
	}
	else {
	    first = other;
	    firstParent = otherParent;
	    last = node;
	}

	while (first != last ) {
	    first->isSelected = first->isSelected?0:1;
	    QTextBox* b = firstParent->box();
	    do {
		b->isSelectionDirty = 1;
	    }while ( ( b = b->parentBox() ) );
	    firstParent->box()->isSelectionDirty = 1;
	    first->isSelectionDirty = 1;
	    first = document->nextLeaf( first, firstParent );
	}
    }

    node = n;
    nodeParent = par;
}


/*! Set x, y, xline, yline, row, rowY and rowHeight according to the
  current cursor position.  */
void QTextCursor::calculatePosition(QPainter* p)
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
void QTextCursor::goTo(QPainter* p, int xarg, int yarg, bool select)
{
    QTextNode* n = document->hitTest(p, 0, 0, xarg, yarg);
    if (n)
	goTo(n, n->parent(), select);
    calculatePosition(p);
}


/*!
  Insert the given string at the current cursor position.
 */
void QTextCursor::insert(QPainter* p, const QString& s)
{
    if (s.isEmpty())
	return;

    QTextNode* n = new QTextNode;
    n->c = s[0];

    QTextNode* last = n;
    for (unsigned int i = 1; i < s.length(); i++) {
	last->next = new QTextNode;
	last = last->next;
	last->c = s[int(i)];
    }

    if (nodeParent->child == node) {
	last->next = node;
	nodeParent->child = n;
	//	row = 0;
    } else {
	QTextNode* prev = node->previousSibling(); // slow!
	last->next = node;
	prev->next = n;
    }
    QTextBox* b = node->box();
    if (b) {
	if (row && row->first == node){
	    row->first = n;
	}
	b->update(p, row);
    }
    calculatePosition(p);
}


/*!
  Enter key, splits the paragraph.
 */
void QTextCursor::enter(QPainter* p)
{

    nodeParent->split(node);

    QTextBox* b = nodeParent->box();
    b->update(p);
    b->next->box()->update( p );

    nodeParent = node->parent();
    calculatePosition(p);
}


/*!
  Delete c items behind the cursor.
 */
void QTextCursor::del(QPainter* p, int c)
{
    QTextNode* curNode = node;
    QTextContainer* curParent = nodeParent;
    QTextRow* curRow = row;
    bool useCurRow = (curRow->first!=curNode && curRow->last != curNode);

    curRow->dirty = TRUE;

    if (c < 1)
	c = 1;

    QTextBox* nodeBox = 0;
    QTextBox* curBox = 0;

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

	QTextNode* prev = curNode->previousSibling();

	// workaround for empty containers at the last
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
	    QTextNode* hook = prev;
	    if (hook) {
		// climb up the tree if we are at the last of usual containers
		while (hook->isLastSibling && !hook->next->isBox )
		    hook = hook->next;
	    }

	    // first, disconnect the moving items, then reconnect them
	    // behind the hook

	    // find the target and the lastTarget
	    bool curBoxInNodeBox = curBox->parentBox()==nodeBox;
	    QTextNode* target = curBoxInNodeBox?curBox->next:nodeBox->child;

	    while (target->isBox && ((QTextContainer*)target)->child)
		target = ((QTextContainer*)target)->child;

	    QTextNode* lastTarget;
	    if (!curBoxInNodeBox) {
		lastTarget = target->lastSibling();
	    }
	    else {
		QTextNode* i = target;
		while ( !i->isLastSibling && (i->next && !i->next->isBox ))
		    i = i->next;
		lastTarget = i;
	    }

	    // disconnect
	    QTextNode* prevTarget = target->previousSibling();
	    if (prevTarget) {
		prevTarget->isLastSibling = lastTarget->isLastSibling;
		prevTarget->next = lastTarget->next;
	    } else {
		QTextContainer* targetParent = target->parent();
		targetParent->child = 0;
		if (lastTarget->next != targetParent)
		    targetParent->child = lastTarget->next;
		else {
		    // targetParent is empty => remove
		    QTextNode* targetParentPrev = targetParent->previousSibling();
		    if (targetParentPrev) {
			targetParentPrev->isLastSibling = targetParent->isLastSibling;
			targetParentPrev->next = targetParent->next;
		    }
		    else {
			QTextContainer* targetParentParent = targetParent->parent;
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
			delete (QTextBox*)targetParent;
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


	    QTextBox* b = curBox->parentBox();
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
	QTextBox* b = curBox->parentBox();
	if (b && oldCurBoxHeight == curBox->height) {
	    b->update( p );
	}
    }

    calculatePosition(p);

}



/*!
  Delete c items before the cursor.
 */
void QTextCursor::backSpace(QPainter* p, int c)
{
    QTextNode* curNode = node;
    for (int i = 0; i < c; i++)
	leftInternal();
    if ( node == curNode )
	return;
    del(p, c);
}

/*!
  Move the cursor one item to the right
 */
void QTextCursor::right(QPainter* p,bool select)
{
    rightInternal(select);
    calculatePosition(p);
}

/*!
  internal
 */
void QTextCursor::rightInternal(bool select)
{
    QTextContainer* np = nodeParent;
    QTextNode* n = document->nextLeaf(node, np);
    if (n)
	goTo(n, np, select);
}

/*!
  Move the cursor one item to the left
 */
void QTextCursor::left(QPainter* p, bool select)
{
    leftInternal(select);
    calculatePosition(p);
}

/*!
  internal
 */
void QTextCursor::leftInternal(bool select)
{
    QTextContainer* tmpParent = 0;

    QTextContainer* np = nodeParent;
    while (np->parent && document->nextLeaf(np, tmpParent) == node)
	np = np->parent;


    QTextNode* n = 0;
    QTextNode* tmp = np->nextLeaf(np, tmpParent);

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
void QTextCursor::up(QPainter* p, bool select)
{
    QTextNode* tmp = node;
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
void QTextCursor::down(QPainter* p, bool select)
{
    QTextNode* tmp = node;
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
void QTextCursor::home(QPainter* p, bool select)
{
    goTo(row->first, row->parent, select );
    calculatePosition(p);
}

/*!
  End key
 */
void QTextCursor::end(QPainter* p, bool select)
{
    goTo(row->last, row->last->parent(), select );
    calculatePosition(p);
}


#endif



QRichText::QRichText( const QString &doc, const QFont& fnt,
		      const QString& context,
		      int margin,  const QMimeSourceFactory* factory, const QStyleSheet* sheet  )
    :QTextBox( (base = new QStyleSheetItem( 0, QString::fromLatin1(""))) )
{
    isRoot = 1;
    contxt = context;

    // for access during parsing only
    factory_ = factory? factory : QMimeSourceFactory::defaultFactory();
    // for access during parsing only
    sheet_ = sheet? sheet : QStyleSheet::defaultSheet();

    init( doc, fnt, margin );

    // clear references that are no longer needed
    factory_ = 0;
    sheet_ = 0;
}


void QRichText::init( const QString& doc, const QFont& fnt, int margin )
{
    //set up base style
    base->setDisplayMode(QStyleSheetItem::DisplayInline);
    base->setFontFamily( fnt.family() );
    base->setFontItalic( fnt.italic() );
    base->setFontUnderline( fnt.underline() );
    base->setFontWeight( fnt.weight() );
    base->setFontSize( fnt.pointSize() );
    base->setMargin( QStyleSheetItem::MarginAll, margin );

    valid = TRUE;
    int pos = 0;
    parse(this, 0, doc, pos);
}

QRichText::~QRichText()
{
    delete base;
}



void QRichText::dump()
{
}



bool QRichText::isValid() const
{
    return valid;
}

/*!
  Returns the context of the rich text document. If no context has been specified
  in the constructor, a null string is returned.
*/
QString QRichText::context() const
{
    return contxt;
}


bool QRichText::parse (QTextContainer* current, QTextNode* lastChild, const QString &doc, int& pos)
{
    bool pre = current->whiteSpaceMode() == QStyleSheetItem::WhiteSpacePre;
    while ( valid && pos < int(doc.length() )) {
	int beforePos = pos;
	if (hasPrefix(doc, pos, QChar('<')) ){
	    if (hasPrefix(doc, pos+1, QChar('/'))) {
		
// 		// only in editor mode!
// 		if (current->isBox){ // todo this inserts a hitable null character
// 		    QTextNode* n = new QTextNode;
// 		    n->c = QChar::null;
// 		    QTextNode* l = lastChild;
// 		    if (!l)
// 			current->child = n;
// 		    else {
// 			l->isLastSibling = 0;
// 			l->next = n;
// 		    }
// 		    n->next = current;
// 		    n->isLastSibling = 1;
// 		    lastChild = n;
// 		    l = n;
// 		}
		return TRUE;
	    }
	    QMap<QString, QString> attr;
	    bool emptyTag = FALSE;
	    QString tagname = parseOpenTag(doc, pos, attr, emptyTag);
	
	    const QStyleSheetItem* nstyle = sheet_->item(tagname);
 	    if ( nstyle && !nstyle->selfNesting() && ( tagname == current->style->name() ) ) {
 		pos = beforePos;
 		return FALSE;
 	    }

	    if ( nstyle && !nstyle->allowedInContext( current->style ) ) {
		qWarning( "QText Warning: Document not valid ( '%s' not allowed in '%s' #%d)",
			 tagname.ascii(), current->style->name().ascii(), pos);
		pos = beforePos;
		return FALSE;
	    }
	
	    QTextNode* tag = sheet_->tag(tagname, attr, context(), *factory_, emptyTag );
	    if (tag->isContainer ) {
		QTextContainer* ctag = (QTextContainer*) tag;
		bool cpre = ctag->whiteSpaceMode() == QStyleSheetItem::WhiteSpacePre;
		if (current == this && !attr.isEmpty() ) {
		    setAttributes( attr );
		}
		valid = valid && ctag != 0;
		if (valid) {
		    QTextNode* l = lastChild;
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

		    eatSpace(doc, pos); // no whitespace within an unknown container or box
		
		    bool ctagUnknown = ctag->style->name().isEmpty() ;
		    if ( !cpre && (ctagUnknown || ctag->isBox) )
			eatSpace(doc, pos); // no whitespace within an unknown container or box
		
		    if (parse(ctag, 0, doc, pos) ) {
			if (!cpre)
			    (void) eatSpace(doc, pos);
			int recoverPos = pos;
			valid = (hasPrefix(doc, pos, QChar('<'))
				 && hasPrefix(doc, pos+1, QChar('/'))
				 && eatCloseTag(doc, pos, tagname) );
			
			// sloppy mode, warning was done in eatCloseTag
			if (!valid) {
			    pos = recoverPos;
			    valid = TRUE;
			    return TRUE;
			}
			
			if (!valid)
			    return TRUE;
		    }
		    if ( !pre && (ctagUnknown || ctag->isBox) ) // no whitespace between unknown containers or boxes
			(void) eatSpace(doc, pos);
		}
	    }
	    else { // empty tags
		if (valid) {
		    QTextNode* l = lastChild;
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
			
		    if (!pre && tag->isSimpleNode && tag->isNewline()){
		      while (pos < int(doc.length())
			     && doc[pos].isSpace()
			     && doc[pos] != QChar(0x00a0U) )
			pos++;
		    }
		}
	    }
	}
	else {
// 	    QString word = parsePlainText(doc, pos, pre);
// 	    if (valid){
// 		QTextNode* l = lastChild;
// 		for (int i = 0; i < int(word.length()); i++){
// 		    QTextNode* n = new QTextNode;
// 		    n->c = word[i];
// 		    if (!l)
// 			current->child = n;
// 		    else {
// 			l->isLastSibling = 0;
// 			l->next = n;
// 		    }
// 		    n->next = current;
// 		    n->isLastSibling = 1;
// 		    lastChild = n;
// 		    l = n;
// 		}
// 		if (!pre)
// 		    sep |= eatSpace(doc, pos);
// 	    }
	    QString word = parsePlainText(doc, pos, pre, TRUE);
	    if (valid){
		QTextNode* l = lastChild;
		QTextNode* n = new QTextNode;
		n->c = word;
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
 		if (!pre && doc[pos] == '<')
 		    (void) eatSpace(doc, pos);
	    }
	}
    }
    return TRUE;
}

bool QRichText::eatSpace(const QString& doc, int& pos)
{
    int old_pos = pos;
    while (pos < int(doc.length()) && doc[pos].isSpace())
	pos++;
    return old_pos < pos;
}

bool QRichText::eat(const QString& doc, int& pos, QChar c)
{
    valid &= (bool) (doc[pos] == c);
    if (valid)
	pos++;
    return valid;
}

bool QRichText::lookAhead(const QString& doc, int& pos, QChar c)
{
    return (doc[pos] == c);
}


QChar QRichText::parseHTMLSpecialChar(const QString& doc, int& pos)
{
    QCString s;
    pos++;
    int recoverpos = pos;
    while ( pos < int(doc.length()) && doc[pos] != ';' && pos < recoverpos + 6) {
	s += doc[pos];
	pos++;
    }
    if (doc[pos] != ';' ) {
	pos = recoverpos;
	return '&';
    }
    pos++;
    if ( s == "lt")
	return '<';
    if ( s == "gt")
	return '>';
    if ( s == "amp")
	return '&';
    if ( s == "nbsp")
	return 0x00a0U;
    if ( s == "aring")
	return 'å';
    if ( s == "oslash")
	return 'ø';
    if ( s == "ouml")
	return 'ö';
    if ( s == "auml")
	return 'ä';
    if ( s == "uuml")
	return 'ü';

    pos = recoverpos;
    return '&';
}

QString QRichText::parseWord(const QString& doc, int& pos, bool insideTag, bool lower)
{
    QString s;

    if (doc[pos] == '"') {
	pos++;
	while ( pos < int(doc.length()) && doc[pos] != '"' ) {
	    s += doc[pos];
	    pos++;
	}
	eat(doc, pos, '"');
    }
    else {
	static QString term = QString::fromLatin1("/>");
	while( pos < int(doc.length()) &&
	       ( !insideTag || (doc[pos] != '>' && !hasPrefix( doc, pos, term)) )
	       && doc[pos] != '<'
	       && doc[pos] != '='
	       && !doc[pos].isSpace())
	{
	    if ( doc[pos] == '&')
		s += parseHTMLSpecialChar( doc, pos );
	    else {
		s += doc[pos];
		pos++;
	    }
	}
	if (lower)
	    s = s.lower();
    }
    valid = valid && pos <= int(doc.length());

    return s;
}

QString QRichText::parsePlainText(const QString& doc, int& pos, bool pre, bool justOneWord)
{
    QString s;
    while( pos < int(doc.length()) &&
	   doc[pos] != '<' ) {
	if (doc[pos].isSpace()){
	    if ( justOneWord && !s.isEmpty() ) {
		return s;
	    }
	
	    if ( pre ) {
		s += doc[pos];
	    }
	    else { // non-pre mode: collapse whitespace except nbsp
		while ( pos+1 < int(doc.length() ) &&
			doc[pos+1].isSpace()  && doc[pos+1] != QChar(0x00a0U) )
		    pos++;
		
		if (doc[pos] != QChar(0x00a0U) )
		    s += ' ';
		else
		    s += doc[pos];
	    }
	    pos++;
	    if ( justOneWord )
		return s;
	}
	else if ( doc[pos] == '&')
		s += parseHTMLSpecialChar( doc, pos );
	else {
	    s += doc[pos];
	    pos++;
	}
    }
    valid = valid && pos <= int(doc.length());
    return s;
}


bool QRichText::hasPrefix(const QString& doc, int pos, QChar c)
{
    return valid && doc[pos] ==c;
}

bool QRichText::hasPrefix(const QString& doc, int pos, const QString& s)
{
    if ( pos + s.length() >= doc.length() )
	return FALSE;
    for (int i = 0; i < int(s.length()); i++) {
	if (doc[pos+i] != s[i])
	    return FALSE;
    }
    return TRUE;
}

QString QRichText::parseOpenTag(const QString& doc, int& pos,
				  QMap<QString, QString> &attr, bool& emptyTag)
{
    emptyTag = FALSE;
    pos++;
    QString tag = parseWord(doc, pos, TRUE, TRUE);
    eatSpace(doc, pos);

    if (tag[0] == '!') {
	if (tag.left(3) == QString::fromLatin1("!--")) {
	    // eat comments
	    QString pref = QString::fromLatin1("-->");
	    while ( valid && !hasPrefix(doc, pos, pref )
			&& pos < int(doc.length()) )
		pos++;
	    if ( valid && hasPrefix(doc, pos, pref ) ) {
		pos += 4;
		eatSpace(doc, pos);
	    }
	    else
		valid = FALSE;
	    return QString::null;
	}
	else {
	    // eat strange internal tags
	    while ( valid && !hasPrefix(doc, pos, QChar('>')) && pos < int(doc.length()) )
		pos++;
	    if ( valid && hasPrefix(doc, pos, QChar('>')) ) {
		pos++;
		eatSpace(doc, pos);
	    }
	    else
		valid = FALSE;
	    return QString::null;
	}
    }

    static QString term = QString::fromLatin1("/>");
    static QString s_true = QString::fromLatin1("true");

    while (valid && !lookAhead(doc, pos, '>')
	    && ! (emptyTag = hasPrefix(doc, pos, term) ))
    {
	QString key = parseWord(doc, pos, TRUE, TRUE);
	eatSpace(doc, pos);
	if ( key.isEmpty()) {
	    // error recovery
	    while ( pos < int(doc.length()) && !lookAhead(doc, pos, '>'))
		pos++;
	    break;
	}
	QString value;
	if (hasPrefix(doc, pos, QChar('=')) ){
	    pos++;
	    eatSpace(doc, pos);
	    value = parseWord(doc, pos, TRUE, FALSE);
	}
	else
	    value = s_true;
	attr.insert(key, value );
	eatSpace(doc, pos);
    }

    if (emptyTag) {
	eat(doc, pos, '/');
	eat(doc, pos, '>');
    }
    else
	eat(doc, pos, '>');

    return tag;
}

bool QRichText::eatCloseTag(const QString& doc, int& pos, const QString& open)
{
    pos++;
    pos++;
    QString tag = parseWord(doc, pos, TRUE, TRUE);
    eatSpace(doc, pos);
    eat(doc, pos, '>');
    if (!valid) {
	qWarning( "QText Warning: Document not valid ( '%s' not closing #%d)", open.ascii(), pos);
	valid = TRUE;
    }
    valid &= tag == open;
    if (!valid) {
	qWarning( "QText Warning: Document not valid ( '%s' not closed before '%s' #%d)",
		 open.ascii(), tag.ascii(), pos);
    }
    return valid;
}
