/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qrichtext.cpp#1 $
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
#include "qrichtextintern.h"
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



QtTextIterator::~QtTextIterator()
{
}


QtTextIterator QtTextIterator::next() const
{
    if ( !node )
	return QtTextIterator( node );

    if ( node->isContainer && (( QtTextContainer*) node)->child )
	return QtTextIterator(  (( QtTextContainer*) node)->child, ( QtTextContainer*) node);
    if ( node->next &&  !node->isLastSibling )
	return QtTextIterator( node->next, par );
    QtTextNode* i = node->next;
    while ( i && i->isLastSibling ) {
	i = i->next;
    }
    if ( i )
	return QtTextIterator( i->next, ((QtTextContainer*)i)->parent );
    return 0;
}


QtTextIterator QtTextIterator::operator++ (int)
{
    QtTextIterator tmp = next(); // not really faster but doesn't matter
    node = tmp.node;
    par = tmp.par;
    return *this;
}

QtTextIterator& QtTextIterator::operator++ ()
{
    QtTextIterator tmp = next();
    node = tmp.node;
    par = tmp.par;
    return *this;
}

QtTextNode::QtTextNode()
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


QtTextNode::~QtTextNode()
{
}




//************************************************************************

QtTextCustomNode::QtTextCustomNode()
    :QtTextNode()
{
    isSimpleNode = 0;
    width = height = 0;
}

QtTextCustomNode::~QtTextCustomNode()
{
}


bool QtTextCustomNode::expandsHorizontally()
{
    return FALSE;
}


QtTextImage::QtTextImage(const QMap<QString, QString> &attr, const QString& context,
		       const QMimeSourceFactory &factory)
    : QtTextCustomNode()
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
	const QMimeSource* m =
			factory.data( imageName, context );
	if ( !m ) {
	    qWarning("QtTextImage: no mimesource for %s", imageName.latin1() );
	}
	else {
	    if ( !QImageDrag::decode( m, img ) ) {
		qWarning("QtTextImage: cannot decode %s", imageName.latin1() );
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


QtTextImage::~QtTextImage()
{
}


void QtTextImage::draw(QPainter* p, int x, int y,
		    int ox, int oy, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/,
		    QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& /*to*/)
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



QtTextHorizontalLine::QtTextHorizontalLine(const QMap<QString, QString> &, const QMimeSourceFactory&)
{
    height = 8;
    width = 200;
}


QtTextHorizontalLine::~QtTextHorizontalLine()
{
}


bool QtTextHorizontalLine::expandsHorizontally()
{
    return TRUE;
}

void QtTextHorizontalLine::draw(QPainter* p, int x, int y,
			     int ox, int oy, int cx, int cy, int cw, int ch,
			     QRegion&, const QColorGroup&, const QtTextOptions& to)
{
    QRect rm( x-ox, y-oy, width, height);
    QRect ra( cx-ox, cy-oy, cw,  ch);
    QRect r = rm.intersect( ra );
    if (to.paper) {
	if ( to.paper->pixmap() )
	    p->drawTiledPixmap( r, *to.paper->pixmap(), QPoint(r.x()+ox, r.y()+oy) );
	else
	    p->fillRect(r, *to.paper );
    }
    QPen pen(p->pen());
    pen.setWidth( 2 );
    p->setPen( pen );
    p->drawLine( r.left()-1, y-oy+4, r.right()+1, y-oy+4) ;
}


//************************************************************************

QtTextRow::QtTextRow()
{
    x = y = width = height = base = 0;
    fill = 0;
    first = last = 0;
    parent = 0;

    dirty = TRUE;
}

QtTextRow::QtTextRow( QPainter* p, QFontMetrics &fm,
		    QtTextIterator& it, int w, int& min, int align)
{
    x = y = width = height = base = 0;
    dirty = TRUE;

    width = w;

    QtTextIterator end = it.parentNode()->box()->end();
    while ( it != end  && !it->isBox && it->isContainer) {
	++it;
    }	

    if ( it == end ) {
	fill = 0;
	first = last = 0;
	parent = 0;
	return;
    }

    first = *it;

    parent = it.parentNode();

    int tx = 0;
    int rh = 0;
    int rasc = 0;
    int rdesc = 0;

    if ( first->isBox ) {
	QtTextBox* b = (QtTextBox*)first;
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
    QtTextIterator lastSpace = it;
    int lastHeight = rh;
    int lastWidth = 0;
    int lastBearing = 0;
    int lastAsc = rasc;
    int lastDesc = rdesc;
    bool noSpaceFound = TRUE; // TRUE;

    QtTextIterator prev = it;


    while ( it != end ) {
	int h,a,d;
	if (it->isSimpleNode) {
	    if ( it.parentNode()->font() != p->font() ) {
		p->setFont( it.parentNode()->font() );
	    }
	    if (!it->isNull())
		tx += fm.width(it->text);
	    h = fm.height();
	    a = fm.ascent();
	    d = h-a;
	}
	else if ( !it->isContainer ) {
	    QtTextCustomNode* c = (QtTextCustomNode*)*it;
	    if ( c->expandsHorizontally() ) {
		c->width = width + fm.minRightBearing() - fm.width(' ');
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
	
	if (tx > width - fm.width(' ') + fm.minRightBearing() && !noSpaceFound  && !it->isSpace() )
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
	    lastBearing = fm.minRightBearing();
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

    min = lastWidth - lastBearing;

    ++it;

}



QtTextRow::~QtTextRow()
{
}



QtTextIterator QtTextRow::begin() const
{
    return QtTextIterator( first, parent );
}

QtTextIterator QtTextRow::end() const
{
    QtTextIterator it( last, 0 );
    ++it;
    return it;
}

void QtTextRow::draw( QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to,
		  bool onlyDirty, bool onlySelection)
{

    if (!intersects(cx-obx, cy-oby, cw,ch)) {
	dirty = FALSE;
	return;
    }


    if (first->isBox) {
	//we have to draw the box
	((QtTextBox*)first)->draw(p, obx+x, oby+y, ox, oy, cx, cy, cw, ch,
			       backgroundRegion, cg, to, dirty?FALSE:onlyDirty, onlySelection);
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
    QtTextIterator it;
    // reduced flicker mode is broken, does not deal with shearing and bearing yet
//     for ( it = begin(); it != end(); ++it ) {
// 	if ( it->isCustomNode() && to.paper ) {
//  	    reducedFlickerMode = TRUE;
// 	    break;
// 	}
//     }

    if ( !reducedFlickerMode ) {
	if (!onlyDirty && !onlySelection && to.paper) {
	    if ( to.paper->pixmap() )
		p->drawTiledPixmap(x+obx-ox, y+oby-oy, width, height, *to.paper->pixmap(), x+obx, y+oby);
	    else
		p->fillRect(x+obx-ox, y+oby-oy, width, height, *to.paper);
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
	const QtTextContainer* anc = it.parentNode()->anchor();
	s.truncate(0);
	QFont font = it.parentNode()->font();
	if ( anc && to.linkUnderline && anc->attributes()
	     && anc->attributes()->contains("href") )
	    font.setUnderline( TRUE );
	if ( font != p->font() ) {
		
	    p->setFont( font );
	}
	int tw = 0;
	bool select = it->isSelected;
	bool selectionDirty = it->isSelectionDirty;
	it->isSelectionDirty = 0;
	if (it->isSimpleNode) {
	    if (!it->isNull()){
		s += it->text;
		tw += fm.width( it->text );
	    }
	    QtTextNode* tmp;
	    // special optimized code for simple nodes (characters)
	    while ( *it != last && (tmp = it->nextSibling() ) && tmp->isSimpleNode
		    && ((bool)tmp->isSelected) == select
		    && ((bool) tmp->isSelectionDirty) == selectionDirty
		    && it->isSimpleNode
		    ) {
		it = QtTextIterator( tmp, it.parentNode() );
		tmp->isSelectionDirty = 0;
		if (!it->isNull()) {
		    s += it->text;
		    tw += fm.width( it->text );
		}
		// 	    if (it->isSpace())
		// 	      break;
	    }
	}
	else {
	    // custom nodes
	    tw += ((QtTextCustomNode*)*it)->width;
	}
	

	if (!onlySelection || selectionDirty) {
	    p->setPen( it.parentNode()->color(cg.text()) );
	
	    if ( anc && anc->attributes() && anc->attributes()->contains("href") )
		p->setPen( to.linkColor );
		

	
	    if (select) {
		if ( inFirst )
		    p->fillRect(x+obx-ox, y+oby-oy, tw+(tx-x), height, cg.highlight() );
		if (*it ==last)
		    p->fillRect(tx+obx-ox, y+oby-oy, width-(tx-x), height, cg.highlight());
		else
		    p->fillRect(tx+obx-ox, y+oby-oy, tw, height, cg.highlight());
		p->setPen( cg.highlightedText() );
	    }
	    else if ( (onlyDirty || onlySelection || (reducedFlickerMode && it->isSimpleNode)) && to.paper ) {
		int txo = 0;
		if ( inFirst ) {
		    if ( to.paper->pixmap() )
			p->drawTiledPixmap(x+obx-ox, y+oby-oy, tw+(tx-x), height,
					   *to.paper->pixmap(), x+obx, y+oby);
		    else
			p->fillRect(x+obx-ox, y+oby-oy, tw+(tx-x), height, *to.paper );
		    txo = tw;
		}
		if (*it == last){
		    if ( to.paper->pixmap() )
			p->drawTiledPixmap(tx+obx-ox+txo, y+oby-oy, width-(tx-x)-txo, height,
					   *to.paper->pixmap(), tx+obx+txo, y+oby);
		    else
			p->fillRect(tx+obx-ox+txo, y+oby-oy, width-(tx-x)-txo, height, *to.paper);
		}
		else {
		    if ( to.paper->pixmap() )
			p->drawTiledPixmap(tx+obx-ox, y+oby-oy, tw, height,
					   *to.paper->pixmap(), tx+obx, y+oby);
		    else
			p->fillRect(tx+obx-ox, y+oby-oy, tw, height, *to.paper);
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
			    if ( to.paper->pixmap() )
				p->drawTiledPixmap(x+obx-ox, y+oby-oy, (tx-x), height,
						   *to.paper->pixmap(), x+obx, y+oby);
			    else
				p->fillRect(x+obx-ox, y+oby-oy, (tx-x), height, *to.paper);
			}
			if ( *it == last){
			    if ( to.paper->pixmap() )
				p->drawTiledPixmap(tx+tw+obx-ox, y+oby-oy, width-(tx-x)-tw, height,
						   *to.paper->pixmap(), tx+obx+tw, y+oby);
			    else
				p->fillRect(tx+tw+obx-ox, y+oby-oy, width-(tx-x)-tw, height, *to.paper);
			}
		    }
		    int h = ((QtTextCustomNode*)*it)->height;
		    if ( h < height && !it->isSelected ) {
			if ( to.paper->pixmap() )
			    p->drawTiledPixmap(tx+obx-ox, y+oby-oy, tw, base-h,
					       *to.paper->pixmap(), tx+obx, y+oby);
			else
			    p->fillRect(tx+obx-ox, y+oby-oy, tw, base-h, *to.paper);
			if ( to.paper->pixmap() )
			    p->drawTiledPixmap(tx+obx-ox, y+oby-oy+base, tw, height-base,
					       *to.paper->pixmap(), tx+obx, y+oby+base);
			else
			    p->fillRect(tx+obx-ox, y+oby-oy+base, tw, height-base, *to.paper);
		    }
 		    ((QtTextCustomNode*)*it)->draw(p,tx+obx,y+oby+base-h,
						  ox, oy, cx, cy, QMIN(tx+obx+width,cw), ch, backgroundRegion, cg, to);
		
		    if ( it->isSelected ) {
			QRect tr( tx+obx-ox, y+oby-oy+base-h, tw, h );
			backgroundRegion = backgroundRegion.subtract( tr );
		    }
		}
		else {
		    int h = ((QtTextCustomNode*)*it)->height;
		    ((QtTextCustomNode*)*it)->draw(p,tx+obx,y+oby+base-h,
					      ox, oy, cx, cy, cw, ch, backgroundRegion, cg, to);
		}
	    }
	}
	tx += tw;
	inFirst = FALSE;
    };

}

QtTextNode* QtTextRow::hitTest(QPainter* p, int obx, int oby, int xarg, int yarg)
{
  QFontMetrics fm = p->fontMetrics();
    if (!intersects(xarg-obx, yarg-oby, 0,0))
	return 0;

    if (first->isBox) {
	return ((QtTextBox*)first)->hitTest(p, obx+x, oby+y, xarg, yarg);
    }
    QtTextIterator it;
    int tx = fill;
    for ( it = begin(); it != end() && ( it->isContainer || !it->isNull()); ++it ) {
	if ( it->isContainer )
	    continue;
	p->setFont( it.parentNode()->font() );
	if (it->isSimpleNode)
	    tx += fm.width( it->text );
	else
	    tx += ((QtTextCustomNode*)*it)->width;
	if (obx + x + tx > xarg)
	    break;
    }
    return *it;

}

bool QtTextRow::locate(QPainter* p, QtTextNode* node, int &lx, int &ly, int &lh)
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

    qDebug("qtextrow locate");
    QtTextIterator it;
    for ( it = begin(); it != end() && *it != node; ++it )
	;
    if ( *it != node ) {
	return FALSE; // nothing found
    }

    lx = x + fill;
    QFontMetrics fm = p->fontMetrics();
    for ( it = begin(); *it != node; ++it ) {
	if ( it->isContainer && !it->isBox )
	    continue;
	p->setFont( it.parentNode()->font() );
	if (it->isSimpleNode)
	    lx += fm.width( it->text );
	else
	    lx += ((QtTextCustomNode*)*it)->width;
    };
    p->setFont( it.parentNode()->font() );
    ly = y + base - fm.ascent();
    lh = fm.height();

    return TRUE;
}



//************************************************************************
QtTextContainer::QtTextContainer( const QStyleSheetItem *stl)
    : style(stl)
{
    isSimpleNode = 0;
    isContainer = 1;
    fnt = 0;
    parent = 0;
    child = 0;
    attributes_ = 0;
    fontsize = -1;
}

QtTextContainer::QtTextContainer( const QStyleSheetItem *stl, const QMap<QString, QString> &attr )
    : style(stl)
{
    isSimpleNode = 0;
    isContainer = 1;
    fnt = 0;
    parent = 0;
    child = 0;
    attributes_ = 0;
    fontsize = -1;
    if (!attr.isEmpty() )
	setAttributes( attr );
}

void QtTextContainer::setParent( QtTextContainer* p)
{
    // invalidate any cached values
    delete  fnt;
    fnt = 0;
    fontsize = -1;
    if ( col.isValid() )
	col = QColor();

    // set the parent
    parent = p;
}

void QtTextContainer::setColor( const QColor& c)
{
    col = c;
}


void QtTextContainer::setFont( const QFont& f)
{
    fnt = new QFont( f );
}

void QtTextContainer::setFontSize( int s )
{
    if ( s < 1)
	s = 1;
    fontsize = s;
}


void QtTextContainer::setAttributes(const QMap<QString, QString> &attr )
{
    delete attributes_;
    attributes_ = new QMap<QString, QString>( attr );
}

QtTextContainer::~QtTextContainer()
{
    delete fnt;
    delete attributes_;
    QtTextNode* n = child;
    while ( n ) {
	QtTextNode* nx;

	if (n->isLastSibling)
	    nx = 0;
	else
	    nx = n->next;
	
	if (n->isBox)
	    delete (QtTextBox*)n;
	else if (n->isContainer)
	    delete (QtTextContainer*)n;
	else if (n->isSimpleNode)
	    delete n;
	else
	    delete (QtTextCustomNode*)n;

	n = nx;
    }
}


QtTextContainer* QtTextContainer::copy() const
{
    QtTextContainer* result = new QtTextContainer( style );
    if (attributes_)
	result->setAttributes( *attributes_ );
    return result;
}

void QtTextContainer::split(QtTextNode* node)
{
    QtTextContainer* c2 = copy();

    QtTextNode* prev = node->previousSibling(); // slow!
    if (!node->isContainer) {
	QtTextNode* n = new QtTextNode;
	//n->text = QString::null;  already does this
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


const QMap<QString, QString> * QtTextContainer::attributes() const
{
    return attributes_;
}

const QtTextContainer* QtTextContainer::anchor() const
{
    if ( style->isAnchor() )
	return this;
    if ( parent )
	return parent->anchor();
    return 0;
}


QtTextContainer* QtTextContainer::findAnchor(const QString& name ) const
{
    if (style->isAnchor() && attributes() &&
	attributes()->contains("name") && (*attributes())["name"] == name)
	return (QtTextContainer*)this;

    QtTextNode* n = child;
    while( n ) {
	if (n->isContainer) {
	    QtTextContainer* t = ((QtTextContainer*)n)->findAnchor( name );
	    if (t)
		return t;
	}
	n = n->nextSibling();
    }
    return 0;
}

QtTextBox* QtTextContainer::box() const
{
    QtTextContainer* result = (QtTextContainer*) this;
    while (result && !result->isBox)
	result = result->parent;
    return (QtTextBox*)result;
}

QtTextBox* QtTextContainer::parentBox() const
{
    QtTextContainer* result = (QtTextContainer*) parent;
    while (result && !result->isBox)
	result = result->parent;
    return (QtTextBox*)result;
}


QtTextIterator QtTextContainer::begin() const
{
    return QtTextIterator( this );
}

QtTextIterator QtTextContainer::end() const
{
    if ( !isLastSibling )
	return QtTextIterator( next, parent );
    QtTextContainer* i = parent;
    while ( i && i->isLastSibling)
	i = i->parent;
    if ( i )
	return QtTextIterator( i->next );
    return 0;
}



QtTextNode* QtTextContainer::lastChild() const
{
    if (!child)
	return 0;
    return child->lastSibling();
}


void QtTextContainer::reparentSubtree()
{
    QtTextNode* n = child;
    while (n) {
	if (n->isContainer) {
	    ((QtTextContainer*)n)->setParent( this );
	    ((QtTextContainer*)n)->reparentSubtree();
	}
	if (n->isLastSibling) {
	    n->next = this;
	    break;
	}
	n = n->next;
    }
}




void QtTextContainer::createFont()
{
    // fnt is used to cache these values, therefore
    // use a temporary QFont* here
    QFont* f = new QFont( fontFamily() );
    f->setWeight( fontWeight() );
    f->setItalic( fontItalic() );
    f->setUnderline( fontUnderline() );
    if ( style->fontSize() > 0 )
	f->setPointSize( style->fontSize() );
    else {
	QtRichText* r = root();
	if ( r ) {
	    f->setPointSize( r->font().pointSize() );
	    style->styleSheet()->scaleFont( *f, fontSize() );
	}
    }
    fnt = f;
}



int QtTextContainer::fontWeight() const
{
    if ( fnt )
      return fnt->weight();
    int w = style->fontWeight();
    if ( w == QStyleSheetItem::Undefined && parent )
	w = parent->fontWeight();
    return w;
}

bool QtTextContainer::fontItalic() const
{
    if ( fnt )
      return fnt->italic();
    if ( style->definesFontItalic() )
      return style->fontItalic();
    return parent? parent->fontItalic() : FALSE;
}

bool QtTextContainer::fontUnderline() const
{
    if ( fnt )
      return fnt->underline();
    if ( style->definesFontUnderline() ) {
	return style->fontUnderline();
    }
    return parent? parent->fontUnderline() : FALSE;
}

int QtTextContainer::fontSize() const
{
    if ( fontsize != -1 )
	return fontsize;


   int f = style->logicalFontSize();

    if ( f == -1 && parent )
	f = parent->fontSize();

    f += style->logicalFontSizeStep();

   QtTextContainer* that = (QtTextContainer*) this;
   that->setFontSize( f );

    return fontsize;
}

QString QtTextContainer::fontFamily() const
{
    if ( fnt )
      return fnt->family();
    QString f = style->fontFamily();
    if ( f.isNull() && parent )
	f = parent->fontFamily();
    return f;
}


QtTextFont::QtTextFont( const QStyleSheetItem *stl)
    : QtTextContainer( stl )
{
}

QtTextFont::QtTextFont( const QStyleSheetItem *stl, const QMap<QString, QString> &attr )
    : QtTextContainer( stl, attr )
{
}

QtTextFont::~QtTextFont()
{
}

void QtTextFont::setParent( QtTextContainer* p)
{
    QtTextContainer::setParent( p );

    if ( attributes() && attributes()->contains("color") )
	setColor( QColor( (*attributes())["color"] ) );

    if ( attributes() && attributes()->contains("size") ) {
	QString a = (*attributes())["size"];
	int n = a.toInt();
	if ( a[0] == '+' || a[0] == '-' )
	    n += fontSize();
	setFontSize( n );
    }

    //### TODO some more font attributes
}

//************************************************************************

QtTextBox::QtTextBox( const QStyleSheetItem *stl)
    :QtTextContainer(stl)
{
    rows.setAutoDelete(TRUE);
    isSimpleNode = 0;
    isBox = 1;
    width = height = widthUsed = 0;
}

QtTextBox::QtTextBox( const QStyleSheetItem *stl, const QMap<QString, QString> &attr )
    :QtTextContainer(stl, attr)
{
    rows.setAutoDelete(TRUE);
    isSimpleNode = 0;
    isBox = 1;
    width = height = widthUsed = 0;
}

QtTextContainer* QtTextBox::copy() const
{
    QtTextBox* result = new QtTextBox( style );
    return result;
}

QtTextBox::~QtTextBox()
{
}




#define IN16BIT(x) QMAX( (2<<15)-1, x)

void QtTextBox::draw(QPainter *p,  int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to,
		  bool onlyDirty, bool onlySelection)
{
    if (onlySelection && !isSelectionDirty)
	return;
    isSelectionDirty = 0;

    if ( !onlySelection && style->displayMode() == QStyleSheetItem::DisplayListItem && rows.first()) {
	QtTextRow* row = rows.first();
	QRect r (obx-ox + row->x - 25, oby-oy + row->y, 25, row->height); //#### label width
	if ( to.paper ) {
 	    if ( to.paper->pixmap() )
 		p->drawTiledPixmap( r, *to.paper->pixmap(), QPoint(r.x()+ox, r.y()+oy) );
 	    else
		p->fillRect(r, *to.paper );
	}
	
	QtTextBox* b = parentBox();
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



    for (QtTextRow* row = rows.first(); row; row = rows.next()) {
	row->draw(p, obx, oby, ox, oy, cx, cy, cw, ch, backgroundRegion, cg, to, onlyDirty, onlySelection);
    }

}


void QtTextBox::setWidth( QPainter* p, int newWidth, bool forceResize )
{
    if (newWidth == width && !forceResize) // no need to resize
	return;

    if (style->displayMode() == QStyleSheetItem::DisplayNone) {
	height = 0;
	return;
    }

    QList<QtTextRow> oldRows;
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

    QtTextRow* row;

    int margintop = margin( QStyleSheetItem::MarginTop );
    int marginbottom = margin( QStyleSheetItem::MarginBottom );
    int marginleft = margin( QStyleSheetItem::MarginLeft );
    int marginright = margin( QStyleSheetItem::MarginRight );
    int marginhorizontal = marginright + marginleft;
    int h = margintop;

    QFontMetrics fm = p->fontMetrics();
    QtTextIterator it = begin();
    ++it;
    if ( it != end() ) {
	p->setFont( it.parentNode()->font() );
	int min = 0;
	while ( it != end() ) {
	    row = new QtTextRow(p, fm, it,
			       colwidth-marginhorizontal - label_offset, min,
			       alignment() );
	    if ( !row->first ) {
		delete row;
		break;
	    }
	    else {
		rows.append(row);
		row->x = marginleft + label_offset;
		row->y = h;
		h += row->height;
		widthUsed = QMAX( widthUsed , min + marginhorizontal + label_offset);
	    }
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
	QtTextRow* old = oldRows.first();
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
	height += QMAX( ((QtTextContainer*)next)->style->margin( QStyleSheetItem::MarginTop), marginbottom);
    } else {
	// nothing to collapse
        height += marginbottom;
    }
}


void QtTextBox::update(QPainter* p, QtTextRow* r)
{
    if ( r ) { // optimization
	QtTextRow* row;
	QtTextRow* prev = 0;
	
	//todo drop QList and connect the rows directly *sigh*
	for ( row = rows.first(); row && row != r; row = rows.next()) {
	    prev = row;
	}
	bool fast_exit = TRUE;
	QFontMetrics fm = p->fontMetrics();
	if (prev) {
	    int min = 0;
	    QtTextIterator it( prev->first, prev->parent );
	    QtTextRow tr (p, fm, it, prev->width, min);
	    fast_exit = fast_exit && prev->last == tr.last;
	}
	if (fast_exit) {
	    int min = 0;
	    QtTextIterator it( r->first, r->parent );
	    QtTextRow tr (p, fm, it,  r->width, min, alignment() );
	    widthUsed = QMAX( widthUsed, min * numberOfColumns() );
	    fast_exit = fast_exit && r->last == tr.last && r->height == tr.height;
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
	QtTextBox* b = parentBox();
	if (b){
	    b->update( p ); // TODO SLOW
	}
    }
}

QtTextRow* QtTextBox::locate(QPainter* p, QtTextNode* node, int &lx, int &ly, int &lh, int&lry, int &lrh)
{
	
    QtTextRow* row;
    for ( row = rows.first(); row; row = rows.next()) {
	if (row->locate(p, node, lx, ly, lh) ) {
	    lry = row->y;
	    lrh = row->height;
	    break;
	}
    }
    if (row) {
	QtTextBox* b = parentBox();
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

QtTextNode* QtTextBox::hitTest(QPainter* p, int obx, int oby, int xarg, int yarg)
{
    QtTextRow* row;
    QtTextNode* result = 0;
    for ( row = rows.first(); row; row = rows.next()) {
	result = row->hitTest(p, obx, oby, xarg, yarg);
	if (result)
	    break;
    }
    return result;
}


int QtTextBox::numberOfSubBox( QtTextBox* subbox, bool onlyListItems)
{
    QtTextNode* i = child;
    int n = 1;
    while (i && i != subbox ) {
	if (!onlyListItems || (i->isBox && ((QtTextBox*)i)->style->displayMode() == QStyleSheetItem::DisplayListItem) )
	    n++;
	i = i->nextSibling();
    }
    if (i)
	return n;
    return 1;
}


QStyleSheetItem::ListStyle QtTextBox::listStyle()
{
    if ( attributes() ) {
	QString s =  (*attributes())["type"];
	
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


QtTextCursor::QtTextCursor(QtRichText& doc)
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

QtTextCursor::~QtTextCursor()
{
}

void QtTextCursor::draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch)
{
    if ( QMAX( x, cx ) <= QMIN( x+width(), cx+cw ) &&
	 QMAX( y, cy ) <= QMIN( y+height, cy+ch ) ) {

	p->drawLine(x-ox, y-oy, x-ox, y-oy+height-1);
	// warwick says two-pixels cursor are ugly
	//	p->drawLine(x+1-ox, y-oy, x+1-ox, y-oy+height-1);
    }
}


void QtTextCursor::clearSelection()
{
    if (!hasSelection)
	return;

    QtTextNode* i = document;
    QtTextContainer* ip = 0;
    while (i && i->isContainer)
	i = document->depthFirstSearch( i, ip);

    while ( i ) {
	if (i->isSelected) {
	    i->isSelected = 0;
	    i->isSelectionDirty = 1;
	    QtTextBox* b = ip->box();
	    do {
		b->isSelectionDirty = 1;
	    }while ( ( b = b->parentBox() ) );
	}
	i = document->nextLeaf( i, ip );
    }

    selectionDirty = TRUE;
    hasSelection = FALSE;
}

void QtTextCursor::goTo(QtTextNode* n, QtTextContainer* par, bool select)
{
    if (select && node != n){
	selectionDirty = TRUE;
	hasSelection = TRUE;

	QtTextNode* other = n;
	QtTextContainer* otherParent = par;

	QtTextNode* i1 = node;
	QtTextContainer* i1p = nodeParent;
	QtTextNode* i2 = other;
	QtTextContainer* i2p = otherParent;
	
	while (i1 != other && i2 != node){
	    if (i1) i1 = document->nextLeaf(i1, i1p);
	    if (i2) i2 = document->nextLeaf(i2, i2p);
	    if ( i1 == i2 && !i1 ) {
		qDebug("it's fucked");
		return;
	    }
	}
	QtTextNode* first = 0;
	QtTextContainer* firstParent = 0;
	QtTextNode* last = 0;
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
	    QtTextBox* b = firstParent->box();
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
void QtTextCursor::calculatePosition(QPainter* p)
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
    qDebug("xline = %d", xline );
}

/*!  Move the cursor to the node near the given coordinates. If select
  is TRUE, the nodes in between toggle their selection state.  */
void QtTextCursor::goTo(QPainter* p, int xarg, int yarg, bool select)
{
    QtTextNode* n = document->hitTest(p, 0, 0, xarg, yarg);
    if (n)
	goTo(n, n->parent(), select);
    calculatePosition(p);
}


/*!
  Insert the given string at the current cursor position.
 */
void QtTextCursor::insert(QPainter* p, const QString& s)
{
    if (s.isEmpty())
	return;

    QtTextNode* n = new QtTextNode;
    n->text = s[0];

    QtTextNode* last = n;
    for (unsigned int i = 1; i < s.length(); i++) {
	last->next = new QtTextNode;
	last = last->next;
	last->text = s[int(i)];
    }

    if (nodeParent->child == node) {
	last->next = node;
	nodeParent->child = n;
	//	row = 0;
    } else {
	QtTextNode* prev = node->previousSibling(); // slow!
	last->next = node;
	prev->next = n;
    }
    QtTextBox* b = node->box();
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
void QtTextCursor::enter(QPainter* p)
{

    nodeParent->split(node);

    QtTextBox* b = nodeParent->box();
    b->update(p);
    b->next->box()->update( p );

    nodeParent = node->parent();
    calculatePosition(p);
}


/*!
  Delete c items behind the cursor.
 */
void QtTextCursor::del(QPainter* p, int c)
{
    QtTextNode* curNode = node;
    QtTextContainer* curParent = nodeParent;
    QtTextRow* curRow = row;
    bool useCurRow = (curRow->first!=curNode && curRow->last != curNode);

    curRow->dirty = TRUE;

    if (c < 1)
	c = 1;

    QtTextBox* nodeBox = 0;
    QtTextBox* curBox = 0;

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

	QtTextNode* prev = curNode->previousSibling();

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
	    QtTextNode* hook = prev;
	    if (hook) {
		// climb up the tree if we are at the last of usual containers
		while (hook->isLastSibling && !hook->next->isBox )
		    hook = hook->next;
	    }

	    // first, disconnect the moving items, then reconnect them
	    // behind the hook

	    // find the target and the lastTarget
	    bool curBoxInNodeBox = curBox->parentBox()==nodeBox;
	    QtTextNode* target = curBoxInNodeBox?curBox->next:nodeBox->child;

	    while (target->isBox && ((QtTextContainer*)target)->child)
		target = ((QtTextContainer*)target)->child;

	    QtTextNode* lastTarget;
	    if (!curBoxInNodeBox) {
		lastTarget = target->lastSibling();
	    }
	    else {
		QtTextNode* i = target;
		while ( !i->isLastSibling && (i->next && !i->next->isBox ))
		    i = i->next;
		lastTarget = i;
	    }

	    // disconnect
	    QtTextNode* prevTarget = target->previousSibling();
	    if (prevTarget) {
		prevTarget->isLastSibling = lastTarget->isLastSibling;
		prevTarget->next = lastTarget->next;
	    } else {
		QtTextContainer* targetParent = target->parent();
		targetParent->child = 0;
		if (lastTarget->next != targetParent)
		    targetParent->child = lastTarget->next;
		else {
		    // targetParent is empty => remove
		    QtTextNode* targetParentPrev = targetParent->previousSibling();
		    if (targetParentPrev) {
			targetParentPrev->isLastSibling = targetParent->isLastSibling;
			targetParentPrev->next = targetParent->next;
		    }
		    else {
			QtTextContainer* targetParentParent = targetParent->parent;
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
			delete (QtTextBox*)targetParent;
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


	    QtTextBox* b = curBox->parentBox();
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
	QtTextBox* b = curBox->parentBox();
	if (b && oldCurBoxHeight == curBox->height) {
	    b->update( p );
	}
    }

    calculatePosition(p);

}



/*!
  Delete c items before the cursor.
 */
void QtTextCursor::backSpace(QPainter* p, int c)
{
    QtTextNode* curNode = node;
    for (int i = 0; i < c; i++)
	leftInternal();
    if ( node == curNode )
	return;
    del(p, c);
}

/*!
  Move the cursor one item to the right
 */
void QtTextCursor::right(QPainter* p,bool select)
{
    rightInternal(select);
    calculatePosition(p);
}

/*!
  internal
 */
void QtTextCursor::rightInternal(bool select)
{
    QtTextContainer* np = nodeParent;
    QtTextNode* n = document->nextLeaf(node, np);
    if (n)
	goTo(n, np, select);
}

/*!
  Move the cursor one item to the left
 */
void QtTextCursor::left(QPainter* p, bool select)
{
    leftInternal(select);
    calculatePosition(p);
}

/*!
  internal
 */
void QtTextCursor::leftInternal(bool select)
{
    QtTextContainer* tmpParent = 0;

    QtTextContainer* np = nodeParent;
    while (np->parent && document->nextLeaf(np, tmpParent) == node)
	np = np->parent;


    QtTextNode* n = 0;
    QtTextNode* tmp = np->nextLeaf(np, tmpParent);

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
void QtTextCursor::up(QPainter* p, bool select)
{
    QtTextNode* tmp = node;
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
void QtTextCursor::down(QPainter* p, bool select)
{
    QtTextNode* tmp = node;
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
void QtTextCursor::home(QPainter* p, bool select)
{
    goTo(row->first, row->parent, select );
    calculatePosition(p);
}

/*!
  End key
 */
void QtTextCursor::last(QPainter* p, bool select)
{
    goTo(row->last, row->last->parent(), select );
    calculatePosition(p);
}





QtRichText::QtRichText( const QString &doc, const QFont& font,
		      const QString& context,
		      int margin,  const QMimeSourceFactory* factory, const QStyleSheet* sheet  )
    :QtTextBox( (base = new QStyleSheetItem( 0, QString::fromLatin1(""))) )
{
    isRoot = 1;
    contxt = context;

    // for access during parsing only
    factory_ = factory? factory : QMimeSourceFactory::defaultFactory();
    // for access during parsing only
    sheet_ = sheet? sheet : QStyleSheet::defaultSheet();

    init( doc, font, margin );

    // clear references that are no longer needed
    factory_ = 0;
    sheet_ = 0;
}


void QtRichText::init( const QString& doc, const QFont& font, int margin )
{
    //set up base style
    base->setDisplayMode(QStyleSheetItem::DisplayInline);
    base->setFontFamily( font.family() );
    base->setFontItalic( font.italic() );
    base->setFontUnderline( font.underline() );
    base->setFontWeight( font.weight() );
    base->setFontSize( font.pointSize() );
    base->setLogicalFontSize( 3 );
    base->setMargin( QStyleSheetItem::MarginAll, margin );

    valid = TRUE;
    int pos = 0;
    parse(this, 0, doc, pos);
}

QtRichText::~QtRichText()
{
    delete base;
}



void QtRichText::dump()
{
}



bool QtRichText::isValid() const
{
    return valid;
}

/*!
  Returns the context of the rich text document. If no context has been specified
  in the constructor, a null string is returned.
*/
QString QtRichText::context() const
{
    return contxt;
}


bool QtRichText::parse (QtTextContainer* current, QtTextNode* lastChild, const QString &doc, int& pos)
{
    bool pre = current->whiteSpaceMode() == QStyleSheetItem::WhiteSpacePre;
    while ( valid && pos < int(doc.length() )) {
	int beforePos = pos;
	if (hasPrefix(doc, pos, QChar('<')) ){
	    if (hasPrefix(doc, pos+1, QChar('/'))) {
		
 		// only in editor mode!
 		if (current->isBox){ // todo this inserts a hitable null character
 		    QtTextNode* n = new QtTextNode;
 		    QtTextNode* l = lastChild;
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
		QString msg;
		msg.sprintf( "QtText Warning: Document not valid ( '%s' not allowed in '%s' #%d)",
			 tagname.ascii(), current->style->name().ascii(), pos);
		sheet_->error( msg );
		pos = beforePos;
		return FALSE;
	    }
	
	    QtTextNode* tag = (QtTextNode*)sheet_->tag(tagname, attr, context(), *factory_, emptyTag );
	    if (tag->isContainer ) {
		QtTextContainer* ctag = (QtTextContainer*) tag;
		bool cpre = ctag->whiteSpaceMode() == QStyleSheetItem::WhiteSpacePre;
		if (current == this && !attr.isEmpty() ) {
		    setAttributes( attr );
		}
		valid = valid && ctag != 0;
		if (valid) {
		    QtTextNode* l = lastChild;
		    if (!l){
			current->child  = ctag;
			ctag->isLastSibling = 1;
		    }
		    else {
			l->next = ctag;
			l->isLastSibling = 0;
		    }
			
		    ctag->setParent( current );
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
		    QtTextNode* l = lastChild;
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
// 		QtTextNode* l = lastChild;
// 		for (int i = 0; i < int(word.length()); i++){
// 		    QtTextNode* n = new QtTextNode;
// 		    n->text = word[i];
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
		QtTextNode* l = lastChild;
		QtTextNode* n = new QtTextNode;
		n->text = word;
		if (!l)
		    current->child = n;
		else {
		    l->isLastSibling = 0;
		    l->next = n;
		}
		n->next = current;
		n->isLastSibling = 1;
		lastChild = n;
		//l = n; NOT USED
 		if (!pre && doc[pos] == '<')
 		    (void) eatSpace(doc, pos);
	    }
	}
    }
    return TRUE;
}

bool QtRichText::eatSpace(const QString& doc, int& pos, bool includeNbsp )
{
    int old_pos = pos;
    while (pos < int(doc.length()) && doc[pos].isSpace() && ( includeNbsp || doc[pos] != QChar(0x00a0U) ) )
	pos++;
    return old_pos < pos;
}

bool QtRichText::eat(const QString& doc, int& pos, QChar c)
{
    valid = valid && (bool) (doc[pos] == c);
    if (valid)
	pos++;
    return valid;
}

bool QtRichText::lookAhead(const QString& doc, int& pos, QChar c)
{
    return (doc[pos] == c);
}


static QMap<QCString, QChar> *html_map = 0;
static void qt_cleanup_html_map()
{
    delete html_map;
    html_map = 0;
}

QMap<QCString, QChar> *htmlMap()
{
    if ( !html_map ){
	html_map = new QMap<QCString, QChar>;
	qAddPostRoutine( qt_cleanup_html_map );
  	html_map->insert("lt", '<');
  	html_map->insert("gt", '>');
  	html_map->insert("amp", '&');
  	html_map->insert("nbsp", 0x00a0U);
  	html_map->insert("aring", 'å');
  	html_map->insert("oslash", 'ø');
  	html_map->insert("ouml", 'ö');
  	html_map->insert("auml", 'ä');
  	html_map->insert("uuml", 'ü');
  	html_map->insert("Ouml", 'Ö');
  	html_map->insert("Auml", 'Ä');
  	html_map->insert("Uuml", 'Ü');
  	html_map->insert("szlig", 'ß');
  	html_map->insert("copy", '©');
  	html_map->insert("deg", '°');
  	html_map->insert("micro", 'µ');
  	html_map->insert("plusmn", '±');
    }
    return html_map;
}

QChar QtRichText::parseHTMLSpecialChar(const QString& doc, int& pos)
{
    QCString s;
    pos++;
    int recoverpos = pos;
    while ( pos < int(doc.length()) && doc[pos] != ';' && !doc[pos].isSpace() && pos < recoverpos + 6) {
	s += doc[pos];
	pos++;
    }
    if (doc[pos] != ';' && !doc[pos].isSpace() ) {
	pos = recoverpos;
	return '&';
    }
    pos++;

    if ( s.length() > 1 && s[0] == '#') {
	return s.mid(1).toInt();
    }

    QMap<QCString, QChar>::Iterator it = htmlMap()->find(s);
    if ( it != htmlMap()->end() ) {
	return *it;
    }

    pos = recoverpos;
    return '&';
}

QString QtRichText::parseWord(const QString& doc, int& pos, bool insideTag, bool lower)
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

QString QtRichText::parsePlainText(const QString& doc, int& pos, bool pre, bool justOneWord)
{
    QString s;
    while( pos < int(doc.length()) &&
	   doc[pos] != '<' ) {
	if (doc[pos].isSpace() && doc[pos] != QChar(0x00a0U) ){
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
		
		s += ' ';
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


bool QtRichText::hasPrefix(const QString& doc, int pos, QChar c)
{
    return valid && doc[pos] ==c;
}

bool QtRichText::hasPrefix(const QString& doc, int pos, const QString& s)
{
    if ( pos + s.length() >= doc.length() )
	return FALSE;
    for (int i = 0; i < int(s.length()); i++) {
	if (doc[pos+i] != s[i])
	    return FALSE;
    }
    return TRUE;
}

QString QtRichText::parseOpenTag(const QString& doc, int& pos,
				  QMap<QString, QString> &attr, bool& emptyTag)
{
    emptyTag = FALSE;
    pos++;
    QString tag = parseWord(doc, pos, TRUE, TRUE);
    eatSpace(doc, pos, TRUE);

    if (tag[0] == '!') {
	if (tag.left(3) == QString::fromLatin1("!--")) {
	    // eat comments
	    QString pref = QString::fromLatin1("-->");
	    while ( valid && !hasPrefix(doc, pos, pref )
			&& pos < int(doc.length()) )
		pos++;
	    if ( valid && hasPrefix(doc, pos, pref ) ) {
		pos += 4;
		eatSpace(doc, pos, TRUE);
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
		eatSpace(doc, pos, TRUE);
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
	eatSpace(doc, pos, TRUE);
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
	eatSpace(doc, pos, TRUE);
    }

    if (emptyTag) {
	eat(doc, pos, '/');
	eat(doc, pos, '>');
    }
    else
	eat(doc, pos, '>');

    return tag;
}

bool QtRichText::eatCloseTag(const QString& doc, int& pos, const QString& open)
{
    pos++;
    pos++;
    QString tag = parseWord(doc, pos, TRUE, TRUE);
    eatSpace(doc, pos, TRUE);
    eat(doc, pos, '>');
    if (!valid) {
	QString msg;
	msg.sprintf( "QtText Warning: Document not valid ( '%s' not closing #%d)", open.ascii(), pos);
	sheet_->error( msg );
	valid = TRUE;
    }
    valid = valid && tag == open;
    if (!valid) {
	QString msg;
	msg.sprintf( "QtText Warning: Document not valid ( '%s' not closed before '%s' #%d)",
		     open.ascii(), tag.ascii(), pos);
	sheet_->error( msg );
    }
    return valid;
}
