#include "qml.h"
#include <qapplication.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qpainter.h>

#include <qstack.h>
#include <stdio.h>

QPixmap *bg = 0;



QMLStyle::QMLStyle( const QString& name )
{
    stylename = name.lower();
    init();
}

const QMLStyle& QMLStyle::nullStyle()
{
    static QMLStyle* nullstyle = 0;
    if (!nullstyle)
	nullstyle = new QMLStyle(0);
    return *nullstyle;
}


void QMLStyle::init()
{
    disp = display_inline;

    fontstyle = style_undefined;
    fontweight = weight_undefined;
    fontsize = -1;
    ncolumns = 1;

}

QString QMLStyle::name() const
{
    return stylename;
}

QMLStyle::Display QMLStyle::display() const
{
    return disp;
}

void QMLStyle::setDisplay(Display d)
{
    disp=d;
}


QMLStyle::FontStyle QMLStyle::fontStyle() const
{
    return fontstyle;
}

void QMLStyle::setFontStyle(FontStyle s)
{
    fontstyle=s;
}

int QMLStyle::fontWeight() const
{
    return fontweight;
}

void QMLStyle::setFontWeight(int w)
{
    fontweight=w;
}

int QMLStyle::fontSize() const
{
    return fontsize;
}

void QMLStyle::setFontSize(int s)
{
    fontsize=s;
}

int QMLStyle::numberOfColumns() const
{
    return ncolumns;
}

void QMLStyle::setNumberOfColumns(int ncols)
{
    if (ncols > 0)
	ncolumns = ncols;
}


//************************************************************************




QMLStyleSheet::QMLStyleSheet()
{
    init();
}

QMLStyleSheet::~QMLStyleSheet()
{
    delete defaultstyle;
}

void QMLStyleSheet::init()
{
    styles.setAutoDelete( TRUE );

    defaultstyle = new QMLStyle("");
    defaultstyle->setDisplay(QMLStyle::display_inline);
    defaultstyle->setFontStyle(QMLStyle::style_normal);
    defaultstyle->setFontWeight(QMLStyle::weight_normal);
    defaultstyle->setFontSize(12);

    QMLStyle*  style;

    style = new QMLStyle( "qml" );
    style->setDisplay(QMLStyle::display_block);
    insert(style);

    style = new QMLStyle( "em" );
    style->setFontStyle( QMLStyle::style_italic);
    insert(style);

    style = new QMLStyle( "large" );
    style->setFontSize( 24 );
    insert(style);

    style = new QMLStyle( "b" );
    style->setFontWeight( QMLStyle::weight_bold);
    insert(style);

    style = new QMLStyle( "h1" );
    style->setFontWeight( QMLStyle::weight_bold);
    style->setFontSize(24);
    style->setDisplay(QMLStyle::display_block);
    insert(style);

    style = new QMLStyle( "p" );
    style->setDisplay(QMLStyle::display_block);
    //     style->setNumberOfColumns(2);
    insert(style);
    style = new QMLStyle( "p2" );
    style->setDisplay(QMLStyle::display_block);
    style->setNumberOfColumns(2);
    insert(style);
    style = new QMLStyle( "p3" );
    style->setDisplay(QMLStyle::display_block);
    style->setNumberOfColumns(3);
    insert(style);
    insert(new QMLStyle("img"));
}


QMLStyle& QMLStyleSheet::defaultStyle() const
{
    return *defaultstyle;
}

QMLStyleSheet& QMLStyleSheet::defaultSheet()
{
    static QMLStyleSheet* defaultsheet = 0;
    if (!defaultsheet)
	defaultsheet = new QMLStyleSheet();
    return *defaultsheet;
}
void QMLStyleSheet::insert( QMLStyle* style)
{
    styles.insert(style->name(), style);
}

const QMLStyle& QMLStyleSheet::style(const char* name) const
{
    if (!name)
	return QMLStyle::nullStyle();
    QMLStyle* s = styles[name];
    return s?*s:QMLStyle::nullStyle();
}

QMLContainer* QMLStyleSheet::tag( const QMLStyle& style,
				  const QDict<QString> *, const QMLContext*   ) const
{
    if (style.display() == QMLStyle::display_block)
	return new QMLBox( style );

    return new QMLContainer( style );
}

// QMLNode* QMLStyleSheet::emptyTag( const QMLStyle& style, QMLContainer* parent,
// 		   const QDict<QString> *attr, const QMLContext* context ) const
// {
//     if (style.name() == "img")
// 	return new QMLImage(attr, context, parent);
//     else
// 	return 0;
// }



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



/*!
  depthFirstSearch traversal for the tag tree
 */
QMLNode* QMLNode::depthFirstSearch(QMLNode* tag, QMLContainer* &parent, bool down)
{
     if (down) {
 	if (tag->isContainer && ((QMLContainer*)tag)->child){
	    parent = (QMLContainer*)tag;
 	    return ((QMLContainer*)tag)->child;
 	}
 	return depthFirstSearch(tag, parent, FALSE);
     }
     else {
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

QMLNode* QMLNode::nextLayout(QMLNode* tag, QMLContainer* &parent){
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

QMLNode* QMLNode::nextLeaf(QMLNode* tag, QMLContainer* &parent){
    do {
	tag = depthFirstSearch(tag, parent);

    } while (tag && tag->isContainer);

     return tag;
}




QMLContainer* QMLNode::parent() const
{
    if (isContainer)
	return ((QMLContainer*)this)->parent;
    else {
	QMLNode* n = lastSibling();
	if (n) return (QMLContainer*)n->next;
    }
    return 0;
}

QMLBox* QMLNode::box() const
{
    QMLContainer* par = parent();
    if (!par)
	return 0;
    else
	return par->box();
}

QMLNode* QMLNode::previous() const
{
    QMLContainer* par = parent();
    QMLNode* result = par->child;
    if (result == this)
	return 0;
    while (result->next && result->next != this)
	result = result->next;
    return result;
}

QMLNode* QMLNode::lastSibling() const
{
    QMLNode* n = (QMLNode*) this;

    while (n && !n->isLastSibling)
	n = n->next;
    return n;
}

QMLNode* QMLNode::nextSibling() const
{
    if (isLastSibling)
	return 0;
    return next;
}


//************************************************************************

QMLRow::QMLRow()
{
    x = y = width = height = base = 0;
//     frameLeft = frameRight = 0;
    start = end = 0;
    parent = 0;


    dirty = TRUE;
}

QMLRow::QMLRow( QMLContainer* box, QPainter* p, QMLNode* &t, QMLContainer* &par, int w)
{
    x = y = width = height = base = 0;
    start = end = 0;
    dirty = TRUE;

    width = w;

    start = t;
    parent = par;

    int tx = 0;
    int rh = 0;
    int rbase = 0;

    if (t->isBox) {
	QMLBox* b = (QMLBox*)t;
	//todo move / layout box
// 	width = b->width;
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
    int lastBase = rbase;
    bool noSpaceFound = TRUE;


    while (i && !i->isBox) {
	p->setFont( par->font() );
	QFontMetrics fm = p->fontMetrics();
	tx += fm.width(i->c);
	if (tx > width)
	    break;
	rh = QMAX( rh, fm.height() );
	rbase = QMAX( rbase, fm.ascent() );
	QMLNode* cur = i;
	i = box->nextLayout(i, par);
	
// 	fprintf(stderr, "%c", cur->c.cell);
	
	// break (a) after a space, (b) before a box, (c) if we have
	// to or (d) at the end of a box.
	if (cur->isSpace() || (i&&i->isBox) || noSpaceFound || !i){
	    lastPar = par;
	    lastSpace = cur;
	    lastHeight = rh;
	    lastBase = rbase;
	    if (noSpaceFound && cur->isSpace())
		noSpaceFound = FALSE;
	    }	
    }
    end = lastSpace;
    i = box->nextLayout(lastSpace, lastPar);
    rh = lastHeight;

    par = lastPar;

    height = rh;
    base = rbase;

    t = i;
}


bool QMLRow::intersects(int xr, int yr, int wr, int hr)
{
    int mx = x;
    int my = y;

    return ( QMAX( mx, xr ) <= QMIN( mx+width, xr+wr ) &&
	     QMAX( my, yr ) <= QMIN( my+height, yr+hr ) );

}

QMLRow::~QMLRow()
{
}



void QMLRow::draw(QMLContainer* box, QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, bool onlyDirty, bool onlySelection)
{

    if (!intersects(cx-obx, cy-oby, cw,ch))
  	return;


    if (start->isBox) {
	//we have to draw the box
	((QMLBox*)start)->draw(p, obx+x, oby+y, ox, oy, cx, cy, cw, ch,
			       backgroundRegion, dirty?FALSE:onlyDirty, onlySelection);
	dirty = FALSE;
	return;
    }

    QRegion r(x+obx-ox, y+oby-oy, width, height);

//      QRegion r(x+obx-ox-frameLeft, y+oby-oy, width+frameLeft+frameRight, height);
//      p->drawTiledPixmap(x-frameLeft+obx-ox, y+oby-oy, frameLeft, height, *bg, x-frameLeft+obx, y+oby);
//      p->drawTiledPixmap(x+width+obx-ox, y+oby-oy, frameRight, height, *bg, x+width+obx, y+oby);
// //     p->fillRect(x-frameLeft+obx-ox, y+oby-oy, frameLeft, height, Qt::red);
// //     p->fillRect(x+width+obx-ox, y+oby-oy, frameRight, height, Qt::red);
     backgroundRegion = backgroundRegion.subtract(r);

    if (onlyDirty) {
	if (!dirty)
	    return;
    }
//     p->eraseRect(x+obx-ox, y+oby-oy, width, height);

    dirty = FALSE;
    QMLNode* t = start;
    QMLContainer* par = parent;

    int tx = x;
    do {
	p->setFont( par->font() );
	QFontMetrics fm = p->fontMetrics();
	QString s = t->c;
	QMLNode* tmp;
	bool select = t->isSelected;
	bool selectionDirty = t->isSelectionDirty;
	t->isSelectionDirty = 0;
	while ( t != end && (tmp = t->nextSibling() ) && tmp->isSimpleNode
		&& tmp->isSelected == select
		&& tmp->isSelectionDirty == selectionDirty
 		&& !t->isSpace()) {
	    t = tmp;
	    tmp->isSelectionDirty = 0;
	    s += t->c;
	}
// 	debug("D:'%s'", s.data());
	
	int tw = fm.width( s );

	if (!onlySelection || selectionDirty) {
	
 	    if (select) {
 		if (t==end)
 		    p->fillRect(tx+obx-ox, y+oby-oy, width-(tx-x), height, Qt::white);
 		else
 		    p->fillRect(tx+obx-ox, y+oby-oy, tw, height, Qt::white);
 	    }
 	    else {
 		if (t==end){
 		    p->drawTiledPixmap(tx+obx-ox, y+oby-oy, width-(tx-x), height, *bg, tx+obx, y+oby);
 		}
 		else {
 		    p->drawTiledPixmap(tx+obx-ox, y+oby-oy, tw, height, *bg, tx+obx, y+oby);
 		}
 	    }
	
	    p->drawText(tx+obx-ox, y+oby-oy+base, s);
	}
	tx += tw;
	if (t == end)
	    break;
	t = box->nextLayout(t, par);
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
    int tx = 0;
    QMLNode* result = t;
    do {
	p->setFont( par->font() );
	QFontMetrics fm = p->fontMetrics();
	tx += fm.width( t->c );
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
    lx = x;
    QFontMetrics fm = p->fontMetrics();
    while (t != node) {
	p->setFont( par->font() );
	fm = p->fontMetrics();
	lx += fm.width( t->c );
	t = box->nextLayout(t, par);
    };
    p->setFont( par->font() );
    fm = p->fontMetrics();
    ly = y + base - fm.ascent();
    lh = fm.height();

    return TRUE;
}



//************************************************************************
QMLContainer::QMLContainer( const QMLStyle &stl)
{
    isSimpleNode = 0;
    isContainer = 1;
    style = &stl;
    fnt = 0;
    parent = 0;
    child = 0;
}

QMLContainer::~QMLContainer()
{
    delete fnt;
}


QMLContainer* QMLContainer::copy()
{
    QMLContainer* result = new QMLContainer(*style);
    return result;
}

void QMLContainer::split(QMLNode* node)
{
    debug("split");
    QMLContainer* c2 = copy();

    QMLNode* prev = node->previous(); // slow!
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
	c2->setParentPointersInSubtree();
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


void QMLContainer::setParentPointersInSubtree()
{
    debug("set parent points in subtree");
    QMLNode* n = child;
    while (n) {
	if (n->isContainer) {
	    ((QMLContainer*)n)->parent = this;
	     ((QMLContainer*)n)->setParentPointersInSubtree();
	}
	if (n->isLastSibling) {
	    n->next = this;
	    break;
	}
	n = n->next;
    }
}



QFont QMLContainer::font() const
{
    if (fnt)
	return *fnt;

    QMLContainer* that = (QMLContainer*) this;
    QFont* tmpfont = parent?new QFont(parent->font()) : new QFont("times");
    tmpfont->setPointSize( fontSize() );
    tmpfont->setWeight( fontWeight() );
    QMLStyle::FontStyle s = fontStyle();
    if ( s == QMLStyle::style_italic)
 	tmpfont->setItalic( TRUE );
    if ( s == QMLStyle::style_oblique)
	tmpfont->setItalic( TRUE );
    that->fnt = tmpfont;
    return *fnt;
}

int QMLContainer::fontWeight() const
{
    int w = style->fontWeight();
    if ( w == QMLStyle::weight_undefined && parent )
	w = parent->fontWeight();
    return w;
}

QMLStyle::FontStyle QMLContainer::fontStyle() const
{
    QMLStyle::FontStyle s = style->fontStyle();
    if ( s == QMLStyle::style_undefined && parent )
	s = parent->fontStyle();
    return s;
}

int QMLContainer::fontSize() const
{
    int w = style->fontSize();
    if ( w == -1 && parent )
	w = parent->fontSize();
    return w;
}


//************************************************************************

QMLBox::QMLBox( const QMLStyle &stl)
    :QMLContainer(stl)
{
    rows.setAutoDelete(true);
    isSimpleNode = 0;
    isBox = 1;
    width = height = 0;
}

QMLContainer* QMLBox::copy()
{
    QMLBox* result = new QMLBox(*style);
    return result;
}

QMLBox::~QMLBox()
{
}

// bool intersects(int x, int y, int w, int h, int x2, int y2,int w2, int h2) {

//     return QMAX( x, x2 ) <= QMIN( x+w, x2+w2 ) &&
// 	     QMAX( y, y2 ) <= QMIN( y+h, y2+h2 ) );
// }


#define IN16BIT(x) QMAX( (2<<15)-1, x)

void QMLBox::draw(QPainter *p,  int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, bool onlyDirty, bool onlySelection)
{
    for (QMLRow* row = rows.first(); row; row = rows.next()) {
	row->draw(this, p, obx, oby, ox, oy, cx, cy, cw, ch, backgroundRegion, onlyDirty, onlySelection);
    }

//     // TODO
//     int frameRight = 5;
//     int frameLeft = 5;
//     p->drawTiledPixmap(-frameLeft+obx-ox, oby-oy, frameLeft, height, *bg, frameLeft+obx, oby);
//     p->drawTiledPixmap(width+obx-ox, oby-oy, frameRight, height, *bg, width+obx, oby);


//      QRegion oldR = p->clipRegion();
//      p->setClipRegion(backgroundRegion);
//      p->drawTiledPixmap(obx-ox, oby-oy, IN16BIT(width), IN16BIT(height), *bg, obx, oby);
//      p->setClipRegion(oldR);
//      QRegion r(obx-ox, oby-oy, IN16BIT(width), IN16BIT(height));
//      backgroundRegion = backgroundRegion.subtract(r);

}


void QMLBox::resize(QPainter* p, int newWidth)
{
    if (newWidth == width) // no need to resize
	return;

//     debug("box %p resize to %d", this, newWidth);

    QList<QMLRow> newRows;

    width = newWidth;
    height = 0;
    int h = 0;

    int ncols = style->numberOfColumns();
    int colwidth = newWidth / ncols;
    if (colwidth < 10)
	colwidth = 10;

    QMLContainer* par = this;
    QMLNode* n = nextLayout( this, par);
    QMLRow* row = 0;
    while (n) {
	if (n->isBox){
	    ((QMLBox*)n)->resize(p, colwidth-10); // todo this can be done in word wrap?!
	}
	row = new QMLRow(this, p, n, par, colwidth-10);
	row->x = 5;
	row->y = h;
// 	row->frameLeft = 5;
// 	row->frameRight = 5;
	newRows.append(row);
	h += row->height;
    }

    // do multi columns if required. Also check with the old rows to
    // optimize the refresh
    row = newRows.first();
    QMLRow* old = rows.first();
    height = 0;
    h /= ncols;
    for (int col = 0; col < ncols; col++) {
	int colheight = 0;
	for (; row && colheight < h; row = newRows.next()) {
	    row->x = col  * colwidth + 5;
	    row->y = colheight;
	
	    if (old) {
		if (old->start == row->start && old->end == row->end
		&& old->height == row->height && old->width == old->width
		&& old->x == row->x && old->y == row->y) // TODO row operator==
		    row->dirty = old->dirty;
		old = rows.next();
	    }
	
	    colheight += row->height;
	}
	height = QMAX( height, colheight );
    }

    rows.clear();
    rows = newRows;

}


void QMLBox::update(QPainter* p, QMLRow* r)
{

    if (r) { // optimization
	QMLRow* row;
	QMLRow* prev = 0;
	
	//todo drop QList and connect the rows directly
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
	    QMLRow tr (this, p, n, par, r->width);
	    fast_exit &= r->end == tr.end && r->height == tr.height;
	}
	if (fast_exit) {
	    r->dirty = TRUE;
	    return;
	}
    }

    int oldHeight = height;
    int oldWidth = width;

    width = 0; // to force rebreak
    resize(p, oldWidth);
	
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


//************************************************************************

QMLCursor::QMLCursor(QMLDocument* doc)
{
    document = doc;
    node = doc;
    nodeParent = 0;

    while (node && node->isContainer)
	node = document->depthFirstSearch( node, nodeParent);


    x = y = height = rowY = rowHeight = 0;
    row = 0;
    xline = 0;
    yline = 0;
    ylineOffsetClean = FALSE;
}

void QMLCursor::draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch)
{
    cx = cy = cw = ch = 0; // no unsed
    p->drawLine(x-ox, y-oy, x-ox, y-oy+height-1);
    p->drawLine(x+1-ox, y-oy, x+1-ox, y-oy+height-1);
}

void QMLCursor::goTo(QMLNode* n, QMLContainer* par, bool select)
{
    if (select){
	//expand selection (TODO SLOW!)
	if (!node->isSelected) {
	    node->isSelected = 1;
	    node->isSelectionDirty = 1;
	    row->dirty = 1;
	}
    }
	
    node = n;
    nodeParent = par;
}

void QMLCursor::calculatePosition(QPainter* p)
{
    row = nodeParent->box()->locate(p, node, x, y, height, rowY, rowHeight);
    xline = x;
    yline = y;
    ylineOffsetClean = FALSE;
}

void QMLCursor::goTo(QPainter* p, int xarg, int yarg, bool select)
{
    QMLNode* n = document->hitTest(p, 0, 0, xarg, yarg);
    if (n)
	goTo(n, n->parent(), select);
    calculatePosition(p);
}


void QMLCursor::insert(QPainter* p, const QChar& c)
{
    QMLNode* n = new QMLNode;
    n->c = c;
    if (nodeParent->child == node) {
	n->next = node;
	nodeParent->child = n;
	row = 0;
    } else {
 	QMLNode* prev = node->previous(); // slow!
 	n->next = node;
 	prev->next = n;
// 	n->next = node->next;
// 	node->next = n;
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


void QMLCursor::enter(QPainter* p)
{
    
    nodeParent->split(node);
    
    QMLBox* b = nodeParent->box();
    b->update(p);
    b->next->box()->update( p );

    debug("done");
    nodeParent = node->parent();
    calculatePosition(p);
}

void QMLCursor::right(QPainter* p, bool select)
{
    QMLContainer* np = nodeParent;
    QMLNode* n = document->nextLeaf(node, np);
    if (n)
	goTo(n, np, select);
    calculatePosition(p);
}

void QMLCursor::left(QPainter* p, bool select)
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
    calculatePosition(p);
}

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

void QMLCursor::home(QPainter* p, bool select)
{
    goTo(row->start, row->parent, select );
    calculatePosition(p);
}

void QMLCursor::end(QPainter* p, bool select)
{
    goTo(row->end, row->end->parent(), select );
    calculatePosition(p);
}


//************************************************************************


QMLContext::QMLContext()
{
    images.setAutoDelete( TRUE );
}

void QMLContext::insert(QString name, const QPixmap& pm)
{
    images.insert(name, new QPixmap(pm));
}

QPixmap* QMLContext::image(const QString &name) const
{
    return  images[name];
}


//************************************************************************


QMLDocument::QMLDocument(const QString &doc,  const QMLContext* context,
			 const QMLStyleSheet* sheet )
    : QMLBox( (sheet_ = sheet)?(sheet_->defaultStyle()):(sheet_ = &QMLStyleSheet::defaultSheet())->defaultStyle())
{
    cursor = 0;
    context_ = context;
    valid = TRUE;
    openChar = new QChar('<');
    closeChar = new QChar('>');
    slashChar = new QChar('/');
    int pos = 0;
    parse(this, 0, doc, pos);
    cursor = new QMLCursor(this);
}

QMLDocument::~QMLDocument()
{
    delete openChar;
    delete closeChar;
    delete slashChar;
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
		    //		    debug("insert star");
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
	    QString tagname = parseOpenTag(doc, pos);
	    sep = eatSpace(doc, pos);
	    const QMLStyle& style = sheet_->style(tagname);
	    //hack
	    QDict<QString>* attr = 0;
	    if (tagname == "img"){
		attr = new QDict<QString>;
		attr->setAutoDelete( TRUE );
		attr->insert("source", new QString("qt.bmp"));
	    }
	    QMLNode* emptytag = 0;//sheet_->emptyTag(style, current, attr, context_);
	    if (emptytag) {
		sep |= eatSpace(doc, pos);
	    }
	    else {
		QMLContainer* tag = sheet_->tag(style);
		valid &= tag != 0;
		if (valid) {
		    QMLNode* l = lastChild;
		    if (!l){
			current->child  = tag;
			tag->isLastSibling = 1;
		    }
		    else {
			l->next = tag;
			l->isLastSibling = 0;
		    }
		
		    tag->parent = current; //TODO
		    tag ->next = current;
		    tag->isLastSibling = 1;
		    lastChild = tag;
		
		    // todo parse attributes
		    parse(tag, 0, doc, pos);
 		    sep |= eatSpace(doc, pos);
		    valid = (hasPrefix(doc, pos, *openChar)
			     && hasPrefix(doc, pos+1, *slashChar)
			     && eatCloseTag(doc, pos, tagname) );
		    if (!valid)
			return;
// 		    sep |= (eatSpace(doc, pos));
		}
	    }
// 	    if (sep) {
//    		    QMLNode* n = new QMLNode;
// 		    n->c = ' ';
// 		    QMLNode* l = lastChild;
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
// 	    }
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
    valid &= doc[pos] == c;
    if (valid)
	pos++;
    return valid;
}


QString QMLDocument::parseWord(const QString& doc, int& pos)
{
    QString s;
    while( doc[pos] != *closeChar && doc[pos] != *openChar
	   && !doc[pos].isSpace() && pos < int(doc.length()) ) {
	s += doc[pos];
	pos++;
    }
    valid &= pos <= int(doc.length());
    return s;
}

QString QMLDocument::parsePlainText(const QString& doc, int& pos)
{
    QString s;
    while( doc[pos] != *closeChar && doc[pos] != *openChar
	   && pos < int(doc.length()) ) {
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

QString QMLDocument::parseOpenTag(const QString& doc, int& pos)
{
    pos++;
    QString tag = parseWord(doc, pos).lower();
    eatSpace(doc, pos);
    eat(doc, pos, *closeChar);
    return tag;
}
bool QMLDocument::eatCloseTag(const QString& doc, int& pos, const QString& open)
{
    pos++;
    pos++;
    QString tag = parseWord(doc, pos).lower();
    eatSpace(doc, pos);
    eat(doc, pos, *closeChar);
    valid &= tag == open;
    return valid;
}



const QMLStyleSheet& QMLDocument::styleSheet() const
{
    return *sheet_;
}


//************************************************************************

QMLView::QMLView()
    : QScrollView(0,0)
{
    setVScrollBarMode( AlwaysOn );
    cursor_hidden = FALSE;
    bg = new QPixmap("bg.ppm");
	
     viewport()->setBackgroundMode(NoBackground); //PaletteBase);
    //    viewport()->setBackgroundPixmap(*bg);

    QPixmap pm("qt.bmp");
    QMLContext* context = new QMLContext();
    context->insert("qt.bmp", pm);

    QMLStyleSheet::defaultSheet().defaultStyle().setFontSize(14);
    //    doc = new QMLDocument("Hallo<em>emph</em>Welt", context);

   // QString text = "<p>Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. </p><p>Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. </p>";

       QString text = "<p>Hello <EM>this is <B>bold</B> italic</EM> this is <B>bold   </B> :-) </p><H1>And this is a pretty long <EM>heading</EM> in 24 point font!</H1><p>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text.  This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. </p><p2>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. <h1>This is a heading inside the p2 environment</h1>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. </p2><p3>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text.</p3>";

//     QString text = "Hello <EM>this is <B>bold</B> italic</EM> this is <B>bold  </B>:-) <H1> And this is a <EM>heading</EM>!</H1> And here the text continues.";

                text += text;
                text += text;
             text += text;
            text += text;
//             text += text;
//            text += text;
//           text += text;
//          text += text;
//          text += text;
//          text += text;

      debug("string length %d", text.length());

     doc = new QMLDocument(text, context);

}

void QMLView::keyPressEvent( QKeyEvent * e)
{

    bool select = e->state() & Qt::ShiftButton;

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

	hideCursor();
	int oldCursorY = doc->cursor->y + doc->cursor->height/2;
	{
	    QPainter p( viewport() );
	    switch (e->key()) {
	    case Key_Right:
		doc->cursor->right(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_Left:
		doc->cursor->left(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_Up:
		doc->cursor->up(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_Down:
		doc->cursor->down(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_Home:
		doc->cursor->home(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_End:
		doc->cursor->end(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_PageUp:
		p.end();
		{
		    int oldContentsY = contentsY();
		    if (!doc->cursor->ylineOffsetClean)
			doc->cursor->yline-=oldContentsY;
		    scrollBy( 0, -viewport()->height() );
		    if (oldContentsY == contentsY() )
			break;
		    p.begin(viewport());
		    int oldXline = doc->cursor->xline;
		    int oldYline = doc->cursor->yline;
 		    doc->cursor->goTo( &p, oldXline, oldYline +  1 + contentsY());
		    doc->cursor->xline = oldXline;
		    doc->cursor->yline = oldYline;
		    doc->cursor->ylineOffsetClean = TRUE;
		    p.end();
		}
		break;
	    case Key_PageDown:
		p.end();
		{
		    int oldContentsY = contentsY();
		    if (!doc->cursor->ylineOffsetClean)
			doc->cursor->yline-=oldContentsY;
		    scrollBy( 0, viewport()->height() );
		    if (oldContentsY == contentsY() )
			break;
		    p.begin(viewport());
		    int oldXline = doc->cursor->xline;
		    int oldYline = doc->cursor->yline;
 		    doc->cursor->goTo( &p, oldXline, oldYline + 1 + contentsY());
		    doc->cursor->xline = oldXline;
		    doc->cursor->yline = oldYline;
		    doc->cursor->ylineOffsetClean = TRUE;
		    p.end();
		}
		break;
	    }
	    if (select) {
		p.begin(viewport());
		int newCursorY = doc->cursor->y + doc->cursor->height/2;
		int minY = QMAX(QMIN(oldCursorY, newCursorY), contentsY());
		int maxY = QMIN(QMAX(oldCursorY, newCursorY), contentsY()+viewport()->height());
		//QRegion r(0, 0, viewport()->width(), viewport()->height());
		QRegion r;
		doc->draw(&p, 0, 0, contentsX(), contentsY(),
			  contentsX(), minY,
			  viewport()->width(), maxY-minY,
			  r, TRUE, TRUE);
		p.end();
	    }
	}
	showCursor();
    }
    else if (e->key() == Key_Return || e->key() == Key_Enter ) {
	{
	    QPainter p( viewport() );
	    debug("enter");
	    doc->cursor->enter( &p );
	    QRegion r(0, 0, viewport()->width(), viewport()->height());
	    doc->draw(&p, 0, 0, contentsX(), contentsY(),
		      contentsX(), contentsY(),
		      viewport()->width(), viewport()->height(),
		      r, TRUE);
	    p.setClipRegion(r);
	    // 	p.fillRect(0, 0, viewport()->width(), viewport()->height(), Qt::white);
	    p.drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			      *bg, contentsX(), contentsY());
	}
	showCursor();
	resizeContents(doc->width, doc->height);
	ensureVisible(doc->cursor->x, doc->cursor->y);
    }
    else if (!e->text().isEmpty() ){
	// other keys
	{
	    QPainter p( viewport() );
	    for (unsigned int i = 0; i < e->text().length(); i++)
		doc->cursor->insert( &p, e->text()[i] );
	    //TODO this is the wrong way. use repaint to schedule events more clever
	    QRegion r(0, 0, viewport()->width(), viewport()->height());
	    doc->draw(&p, 0, 0, contentsX(), contentsY(),
		      contentsX(), contentsY(),
		      viewport()->width(), viewport()->height(),
		      r, TRUE);
	    p.setClipRegion(r);
	    // 	p.fillRect(0, 0, viewport()->width(), viewport()->height(), Qt::white);
	    p.drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			      *bg, contentsX(), contentsY());
	}
	showCursor();
	resizeContents(doc->width, doc->height);
	ensureVisible(doc->cursor->x, doc->cursor->y);
	//viewport()->repaint();
    }

}

void QMLView::viewportMousePressEvent( QMouseEvent * e)
{
    hideCursor();
    {
	QPainter p( viewport() );
	doc->cursor->goTo( &p, contentsX() + e->x(), contentsY() + e->y());
    }
    showCursor();
}


void QMLView::drawContentsOffset(QPainter*p, int ox, int oy,
				    int cx, int cy, int cw, int ch)
{

//     static int c = 0;
    //    p->drawRect(cx-ox,cy-oy,cw,ch);

    //    p->setClipRect( cx-ox, cy-oy, cw, ch );

    //    p->eraseRect(cx-ox, cy-oy, cw, ch);
//     p->drawTiledPixmap(cx-ox, cy-oy, cw, ch, bg, ox, oy);


    QRegion r(cx-ox, cy-oy, cw, ch);
    doc->draw(p, 0, 0, ox, oy, cx, cy, cw, ch, r);
    p->setClipRegion(r);
//     p->fillRect(0, 0, viewport()->width(), viewport()->height(), Qt::white);
    p->drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
 		       *bg, ox, oy);
//     p->eraseRect(0, 0, viewport()->width(), viewport()->height());

    qApp->syncX();

    p->setClipping( FALSE );
    if (!cursor_hidden)
	doc->cursor->draw(p, ox, oy, cx, cy, cw, ch);
    //    doc->dump();
}

void QMLView::showCursor()
{
    cursor_hidden = FALSE;
    QPainter p( viewport() );
    doc->cursor->draw(&p, contentsX(), contentsY(),
		      contentsX(), contentsY(),
		      viewport()->width(), viewport()->height());
}

void QMLView::hideCursor()
{
    cursor_hidden = TRUE;
    viewport()->repaint(doc->cursor->x-contentsX(),
			doc->cursor->y-contentsY(),
			doc->cursor->width(),
			doc->cursor->height);
}

void QMLView::resizeEvent(QResizeEvent*e)
{
    QScrollView::resizeEvent(e);
    {
	QPainter p( this );
	doc->resize(&p, viewport()->width());
	doc->cursor->calculatePosition(&p);
    }
    resizeContents(doc->width, doc->height);
}


int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QMLView t;
    a.setMainWidget(&t);
    t.show();

    return a.exec();
}

