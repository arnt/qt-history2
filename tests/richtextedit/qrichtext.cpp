/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qrichtext.cpp#6 $
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
#include "qformatstuff.h"
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
    box = 0;
    dirty = TRUE;
}


QtTextRow::QtTextRow( QPainter* p, QFontMetrics & fm,
		      QtBox* b, int w, int& min, int /*align*/)
{
    x = y = width = height = base = 0;
    dirty = TRUE;
    text = 0;
    box = b;

    width = w;

    box->setWidth(p, fm, width );
    width = QMAX( box->widthUsed, width );
    height = box->height;
    base = height;
    last = first;
    min = box->widthUsed;
}

QtTextRow::QtTextRow( QPainter* p, QFontMetrics &fm,
		    const QtTextRichString* t, int &index, int w, int& min, int align)
{
    x = y = width = height = base = 0;
    dirty = TRUE;

    width = w;

    first = last = 0;
    box = 0;
    text = t;


    first = index;

    int tx = 0;
    int rh = 0;
    int rasc = 0;
    int rdesc = 0;

    // do word wrap
    int lastSpace = index;
    int lastHeight = rh;
    int lastWidth = 0;
    int lastBearing = 0;
    int lastAsc = rasc;
    int lastDesc = rdesc;
    bool noSpaceFound = TRUE;

    QtTextCharFormat* fmt = text->formatAt( index );
    int fmt_index = index;
    p->setFont( fmt->font() );
    int minRightBearing = fm.minRightBearing();
    int space_width = fm.width(' ');
    int fm_ascent = fm.ascent();
    int fm_height = fm.height();

    while ( index < text->length() ) {
	int h,a,d;

	// TODO custom nodes
	if ( !text->haveSameFormat( fmt_index, index ) ) {
	    fmt = text->formatAt( index );
	    fmt_index = index;
	    p->setFont( fmt->font() );
	    minRightBearing = fm.minRightBearing();
	    space_width = fm.width(' ');
	    fm_ascent = fm.ascent();
	    fm_height = fm.height();
	}

	QtTextRichString::Item* item = &text->items[index];
	QString c = item->c;
	QChar lastc = c[c.length()-1];
	if ( item->width < 0 )
	    item->width = fm.width( c );
	tx += item->width;
	
	h = fm_height;
	a = fm_ascent;
	d = h-a;

	/* custom nodes
	    if ( c->expandsHorizontally() ) {
		c->width = width + fm.minRightBearing() - fm.width(' ');
	    }
	    tx +=c->width;
	    h = c->height;
	    a = h;
	    d = 0;
	*/
	
	if ( tx > width - space_width + minRightBearing && !noSpaceFound )
	    break;

	if ( h > rh )
	    rh = h;
	if ( a > rasc )
	    rasc = a;
	if ( d > rdesc )
	    rdesc = d;
	
	++index;
	
	// break (a) after a space, (b) before a box, (c) if we have
	// to or (d) at the end of a box.
	if ( lastc == ' ' || lastc == '\n' || index == text->length() ){
	    lastSpace = index - 1;
	    lastHeight = rh;
	    lastAsc = rasc;
	    lastDesc = rdesc;
	    lastWidth = tx;
	    lastBearing = minRightBearing;
 	    if ( lastc == ' ' )
 		noSpaceFound = FALSE;
	}
	if ( lastc == '\n' )
	    break;
    }

    last = lastSpace;

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

    index = lastSpace;

    if ( lastWidth > width ) {
	width = lastWidth;
	fill = 0;
    }

    min = lastWidth - lastBearing;

    ++index;

}



QtTextRow::~QtTextRow()
{
}



void QtTextRow::draw( QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, const QtTextOptions& to,
		  bool onlyDirty, bool onlySelection)
{

    if (!intersects(cx-obx, cy-oby, cw,ch)) {
	dirty = FALSE;
	return;
    }


    if ( box ) {
	//we have to draw the box
	box->draw(p, obx+x, oby+y, ox, oy, cx, cy, cw, ch,
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

    if (TRUE ) { //!onlyDirty && !onlySelection && to.paper) {
	if ( to.paper->pixmap() )
	    p->drawTiledPixmap(x+obx-ox, y+oby-oy, width, height, *to.paper->pixmap(), x+obx, y+oby);
	else
	    p->fillRect(x+obx-ox, y+oby-oy, width, height, *to.paper);
    }

    int tx = x;

    if ( fill > 0 )
	tx += fill;

    static QString s;

    QFontMetrics fm = p->fontMetrics();
    QString c;
    int tw;
    for ( int index = first; index <= last; ++index ) {
	QtTextCharFormat *format  = text->formatAt( index );
	p->setFont( format->font() );
	p->setPen( format->color() );
	
	if ( !format->anchorHref().isEmpty() ) {
	    p->setPen( to.linkColor );
	    if ( to.linkUnderline ) {
		QFont f = format->font();
		f.setUnderline( TRUE );
		p->setFont( f );
	    }
	}
	
	tw = 0;
	// TODO custom nodes
	
	
	s = "";
	c = text->charAt( index );
	if (!c.isNull()){
	    s += c;
	    tw += fm.width( c );
	}
	//TODO: put the string together and ask for the width!
	while ( index < last && text->haveSameFormat( index, index+1 ) ) {
	    c = text->charAt( ++index );
	    if (!c.isNull()) {
		s += c;
		tw += fm.width( c );
	    }
	}
	
// 	// Get rid of garbage at last of line (\n etc. at visible
// 	// on Windows. ### Matthias: Fix in parser?
 	int len = s.length();
 	if ( len > 0 && s[len-1] < (char)32 ) {
 	    len--;
 	}
	p->drawText(tx+obx-ox, y+oby-oy+base, s, len);
	tx += tw;
    }

}

// QtTextNode* QtTextRow::hitTest(QPainter* p, int obx, int oby, int xarg, int yarg)
// {
//   QFontMetrics fm = p->fontMetrics();
//     if (!intersects(xarg-obx, yarg-oby, 0,0))
// 	return 0;

//     if (first->isBox) {
// 	return ((QtTextBox*)first)->hitTest(p, obx+x, oby+y, xarg, yarg);
//     }
//     QtTextIterator it;
//     int tx = fill;
//     for ( it = begin(); it != end() && ( it->isContainer || !it->isNull()); ++it ) {
// 	if ( it->isContainer )
// 	    continue;
// 	p->setFont( it.parentNode()->font() );
// 	if (it->isSimpleNode)
// 	    tx += fm.width( it->text );
// 	else
// 	    tx += ((QtTextCustomNode*)*it)->width;
// 	if (obx + x + tx > xarg)
// 	    break;
//     }
//     return *it;

// }

// bool QtTextRow::locate(QPainter* p, QtTextNode* node, int &lx, int &ly, int &lh)
// {
//     if (first->isBox) { // a box row
// 	if (node == first) {
// 	    lx = x;
// 	    ly = y;
// 	    lh = height;
// 	    return TRUE;
// 	}
// 	return FALSE;
//     }

//     qDebug("qtextrow locate");
//     QtTextIterator it;
//     for ( it = begin(); it != end() && *it != node; ++it )
// 	;
//     if ( *it != node ) {
// 	return FALSE; // nothing found
//     }

//     lx = x + fill;
//     QFontMetrics fm = p->fontMetrics();
//     for ( it = begin(); *it != node; ++it ) {
// 	if ( it->isContainer && !it->isBox )
// 	    continue;
// 	p->setFont( it.parentNode()->font() );
// 	if (it->isSimpleNode)
// 	    lx += fm.width( it->text );
// 	else
// 	    lx += ((QtTextCustomNode*)*it)->width;
//     };
//     p->setFont( it.parentNode()->font() );
//     ly = y + base - fm.ascent();
//     lh = fm.height();

//     return TRUE;
// }



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
	    ; // nothing
// 	    delete (QtTextBox*)n;
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
// 	QtRichText* r = root();
// 	if ( r ) {
// 	    f->setPointSize( r->font().pointSize() );
// 	    style->styleSheet()->scaleFont( *f, fontSize() );
// 	}
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




QtRichText::QtRichText( const QString &doc, const QFont& font,
		      const QString& context,
		      int margin,  const QMimeSourceFactory* factory, const QtStyleSheet* sheet  )
    :QtBox( 0, new QtTextFormatCollection(), (base = new QStyleSheetItem( 0, QString::fromLatin1(""))) )
{
//     isRoot = 1;
    contxt = context;

    // for access during parsing only
    factory_ = factory? factory : QMimeSourceFactory::defaultFactory();
    // for access during parsing only
    sheet_ = sheet? sheet : (QtStyleSheet*)QtStyleSheet::defaultSheet();

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
    QtTextCharFormat fmt( font, Qt::black );
    parse(this, base, 0, fmt, doc, pos);
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


void QtRichText::setWidth (QPainter* p, int newWidth )
{
    QFontMetrics fm = p->fontMetrics();
    QtBox::setWidth( p, fm, newWidth );
}

/*!
  Returns the context of the rich text document. If no context has been specified
  in the constructor, a null string is returned.
*/
QString QtRichText::context() const
{
    return contxt;
}

static int qt_link_count = 0;
bool QtRichText::parse (QtBox* current, const QStyleSheetItem* curstyle, QtBox* dummy,
			QtTextCharFormat fmt, const QString &doc, int& pos)
{
    bool pre = current->whiteSpaceMode() == QStyleSheetItem::WhiteSpacePre;
    if ( !pre )
	eatSpace(doc, pos); 
    while ( valid && pos < int(doc.length() )) {
	int beforePos = pos;
	if (hasPrefix(doc, pos, QChar('<')) ){
	    if (hasPrefix(doc, pos+1, QChar('/'))) {
		// TODO insert final space at the end if end of box!
		return TRUE;
	    }
	    QMap<QString, QString> attr;
	    bool emptyTag = FALSE;
	    QString tagname = parseOpenTag(doc, pos, attr, emptyTag);
	
	    const QStyleSheetItem* nstyle = sheet_->item(tagname);
 	    if ( nstyle && !nstyle->selfNesting() && ( tagname == curstyle->name() ) ) {
 		pos = beforePos;
 		return FALSE;
 	    }

	    if ( nstyle && !nstyle->allowedInContext( curstyle ) ) {
		QString msg;
		msg.sprintf( "QtText Warning: Document not valid ( '%s' not allowed in '%s' #%d)",
			 tagname.ascii(), current->style->name().ascii(), pos);
		sheet_->error( msg );
		pos = beforePos;
		return FALSE;
	    }
	
	    // TODO was wenn wir keinen nstyle haben?
	    if ( !nstyle )
		nstyle = base;
	
	    // TODO QtStyleSheet::tagEx
	
	    if (nstyle->displayMode() == QStyleSheetItem::DisplayBlock
		|| nstyle->displayMode() == QStyleSheetItem::DisplayListItem
		|| nstyle->displayMode() == QStyleSheetItem::DisplayNone
		) {
		if ( !current->text.isEmpty() ) {
		    dummy = new QtBox( current, formats, base );
		    dummy->text = current->text;
		    current->text.clear();
		    current->child = dummy;
		}
		QtBox* subbox = new QtBox( current, formats, nstyle, attr );
		if ( !current->child )
		    current->child = subbox;
		else {
		    QtBox* it = current->child;
		    while ( it->next )
			it = it->next;
		    it->next = subbox;
		}
		dummy = 0;
		if (parse( subbox, nstyle, 0, fmt.makeTextFormat( nstyle, attr ), doc, pos) ) {
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

		(void) eatSpace(doc, pos);
	    }
	    else { // containers and empty tags
		// TODO: check empty tags and custom tags in stylesheet
		if ( nstyle->isAnchor() ) {
		    qt_link_count++;
		}
		// for now: assume container
		if ( parse( current, nstyle, dummy, fmt.makeTextFormat( nstyle, attr ), doc, pos ) ) {
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
	    }
	}
	else { // plain text
	    if ( current->child && !dummy ) {
		dummy = new QtBox( current, formats, base );
		    QtBox* it = current->child;
		    while ( it->next )
			it = it->next;
		    it->next = dummy;
	    }
 	    QString word = parsePlainText( doc, pos, pre, TRUE );
 	    if (valid){
//  		for (int i = 0; i < int(word.length()); i++){
//  		    (dummy?dummy:current)->text.append( word[i], fmt );
// 		}
		(dummy?dummy:current)->text.append( word, fmt );
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



QtStyleSheet::QtStyleSheet( QObject *parent=0, const char *name=0 )
    : QStyleSheet( parent, name )
{
}

QtStyleSheet::~QtStyleSheet()
{
}

QtTextCustomItem* QtStyleSheet::tagEx( const QString& name,
				     const QMap<QString, QString> &attr,
				     const QString& context,
				     const QMimeSourceFactory& factory,
				     bool emptyTag) const
{
    return 0; //TODO
}

void QtBox::draw(QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	  QRegion& backgroundRegion,
	  const QColorGroup& cg, const QtTextOptions& to,
	  bool onlyDirty, bool onlySelection )
{
//     if (onlySelection && !isSelectionDirty)
// 	return;
//     isSelectionDirty = 0;

    if ( !onlySelection && style->displayMode() == QStyleSheetItem::DisplayListItem && rows.first()) {
	QtTextRow* row = rows.first();
	QRect r (obx-ox + row->x - 25, oby-oy + row->y, 25, row->height); //#### label width
	if ( to.paper ) {
 	    if ( to.paper->pixmap() )
 		p->drawTiledPixmap( r, *to.paper->pixmap(), QPoint(r.x()+ox, r.y()+oy) );
 	    else
		p->fillRect(r, *to.paper );
	}
	
	QtBox* b = parent;
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
		//p->setFont( font() );??? Font? TODO
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

void QtBox::setWidth( QPainter* p, QFontMetrics& fm,  int newWidth, bool forceResize )
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

    int ncols = 1; // TODO numberOfColumns();
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

    // two different kind of boxes, one has subboxes, the other has text

    if ( child ) {
	int min = 0;
	for ( QtBox* it = child; it; it = it->next ) {
	    row = new QtTextRow( p, fm, it,
				colwidth-marginhorizontal - label_offset, min,
				alignment() );
	    rows.append( row );
	    row->x = marginleft + label_offset;
	    row->y = h;
	    h += row->height;
	    widthUsed = QMAX( widthUsed , min + marginhorizontal + label_offset);
	}
    } else { // real text
	int index = 0;
	int min;
	while ( index < text.length() ) {
	    row = new QtTextRow( p, fm, &text, index,
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
		    if ( row->box ) {
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
    if ( !next && parent ) {
	// ignore bottom margin
    } else if ( next  ) {
	// collapse
	height += QMAX( next->style->margin( QStyleSheetItem::MarginTop), marginbottom );
    } else {
	// nothing to collapse
	height += marginbottom;
    }
}


QStyleSheetItem::ListStyle QtBox::listStyle()
{
    QString s =  attributes()["type"];
	
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
    return style->listStyle();
}


QtBox::~QtBox()
{
    QtBox* tmp = child;
    while ( child ) {
	tmp = child;
	child = child->next;
	delete tmp;
    }
}



QtTextRichString::QtTextRichString( QtTextFormatCollection* fmt )
    : format( fmt )
{
    items = 0;
    len = 0;
    store = 0;
}

QtTextRichString::~QtTextRichString()
{
    for (int i = 0; i < len; ++i )
	format->unregisterFormat( *items[i].format );
}


void QtTextRichString::clear()
{
    for (int i = 0; i < len; ++i )
	format->unregisterFormat( *items[i].format );
    delete [] items;
    len = 0;
    store = 0;
}


void QtTextRichString::remove( int index, int len )
{
    for (int i = 0; i < len; ++i )
	format->unregisterFormat( *formatAt( index + i ) );

    int olen = length();
    if ( index + len >= olen ) {		// range problems
	if ( index < olen ) {			// index ok
	    setLength( index );
	}
    } else if ( len != 0 ) {
// 	memmove( items+index, items+index+len,
// 		 sizeof(Item)*(olen-index-len) );
	for (int i = index; i < olen-len; ++i)
	    items[i] = items[i+len];
	setLength( olen-len );
    }
}

void QtTextRichString::insert( int index, const QString& c, const QtTextCharFormat& form )
{
    QtTextCharFormat* f = format->registerFormat( form );

    if ( index >= len ) {			// insert after end
	setLength( index+1 );
    } else {					// normal insert
	int olen = len;
	int nlen = len + 1;
	setLength( nlen );
// 	memmove( items+index+1, items+index,
// 		 sizeof(Item)*(olen-index) );
	for (int i = len+1; i > index; --i)
	    items[i] = items[i-1];
    }
    
    items[index].c = c;
    items[index].format = f;
}

void QtTextRichString::setLength( int l )
{
    if ( l <= store ) {
	len = l; // TODO shrinking
	return;
    } else {
	store = l*2;
	Item* newitems = new Item[store]; // TODO speedup using new char(....)
// 	if ( items )
// 	    memcpy( newitems, items, sizeof(Item)*len );
	for (int i = 0; i < len; ++i )
	    newitems[i] = items[i];
	delete [] items;
	items = newitems;
	len = l;
    }
}

QtTextRichString::QtTextRichString( const QtTextRichString &other )
{
    format = other.format;
    len = other.len;
    items = 0;
    store = 0;
    if ( len ) {
	items = new Item[ len ];
	store = len;
// 	memcpy( items, other.items, sizeof(Item)*len );
	for (int i = 0; i < len; ++i ) {
	    items[i] = other.items[i];
	    items[i].format->addRef();
	}
    }
}

QtTextRichString& QtTextRichString::operator=( const QtTextRichString &other )
{
    clear();
    format = other.format;
    len = other.len;
    items = 0;
    store = 0;
    if ( len ) {
	items = new Item[ len ];
	store = len;
// 	memcpy( items, other.items, sizeof(Item)*len );
	for (int i = 0; i < len; ++i ) {
	    items[i] = other.items[i];
	    items[i].format->addRef();
	}
    }
    return *this;
}
