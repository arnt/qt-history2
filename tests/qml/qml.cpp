#include "qml.h"
#include <qapplication.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qpainter.h>

#include <qstack.h>
#include <stdio.h>
#include <qfile.h>
#include <qlayout.h>


QMLStyle::QMLStyle( const QString& name )
{
    stylename = name.lower();
    init();
}

const QMLStyle& QMLStyle::nullStyle()
{
    static QMLStyle* nullstyle = 0;
    if (!nullstyle)
	nullstyle = new QMLStyle( QString::null );
    return *nullstyle;
}

QMLStyle::~QMLStyle()
{
    delete col;
}


void QMLStyle::init()
{
    disp = display_inline;

    fontstyle = style_undefined;
    fontweight = weight_undefined;
    fontsize = -1;
    ncolumns = 1;
    col = 0;
    active = FALSE;

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

QColor QMLStyle::color( const QColor &c ) const
{
    if (col) return *col;
    return c;
}

void QMLStyle::setColor( const QColor &c)
{
    delete col;
    col = new QColor(c);
}


bool QMLStyle::isActive() const
{
    return active;
}

void QMLStyle::setActive(bool act)
{
    active = act;
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
    style->setDisplay( QMLStyle::display_block );
    insert(style);

    style = new QMLStyle( "qwhatsthis" );
    style->setDisplay( QMLStyle::display_block );
    insert(style);

    style = new QMLStyle( "a" );
    style->setColor( Qt::blue );
    style->setActive( TRUE );
    insert( style );

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
    insert(style);
    style = new QMLStyle( "p2" );
    style->setDisplay(QMLStyle::display_block);
    style->setNumberOfColumns(2);
    insert(style);
    style = new QMLStyle( "p3" );
    style->setDisplay(QMLStyle::display_block);
    style->setNumberOfColumns(3);
    insert(style);


    style = new QMLStyle( "ul" );
    style->setDisplay(QMLStyle::display_block);
    style->setFontStyle( QMLStyle::style_italic);
    insert(style);

    style = new QMLStyle( "item" );
    style->setDisplay(QMLStyle::display_list_item);
    style->setFontWeight( QMLStyle::weight_bold );
    insert(style);

    insert(new QMLStyle("img"));

}


QMLStyle& QMLStyleSheet::defaultStyle() const
{
    return *defaultstyle;
}

QMLStyleSheet& QMLStyleSheet::defaultSheet()
{
    //### pseudo memory leak, needs clean up
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
				  const QDict<QString> &attr,
				  const QMLProvider& ) const
{
    switch ( style.display() ) {
    case QMLStyle::display_block:
    case QMLStyle::display_list_item:
	return style.isActive()? new QMLBox( style, attr ) : new QMLBox( style );
    default: // inline, none
	return style.isActive()? new QMLContainer( style, attr ) : new QMLContainer( style );
    }
}

QMLNode* QMLStyleSheet::emptyTag( const QMLStyle& style,
				  const QDict<QString> &attr,
				  const QMLProvider& provider ) const
{
    if (style.name() == "img")
	return new QMLImage(attr, provider);
    return 0;
}

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
    //   debug("create qmlimage");
    QString* imageName = attr["source"];
    if (imageName) {
	pm = provider.image( *imageName );
	width = pm.width();
	height = pm.height();
	if (pm.isNull()) {
	    width = height = 10;
	}
    }
    //   debug("create qmlimage done %d %d", width, height);
}

QMLImage::~QMLImage()
{
    debug("image destructor");
}

void QMLImage::draw(QPainter* p, int x, int y,
		    int ox, int oy, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/)
{
    p->drawPixmap( x-ox , y-oy, pm );
}


//************************************************************************

QMLRow::QMLRow()
{
    x = y = width = height = base = 0;
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
    int rasc = 0;
    int rdesc = 0;

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
	if (tx > width)
	    break;

	rh = QMAX( rh, h );
	rasc = QMAX( rasc, a );
	rdesc = QMAX( rdesc, d );

	QMLNode* cur = i;
	i = box->nextLayout(i, par);
	
	// 	fprintf(stderr, "%c", cur->c.cell);
	
	// break (a) after a space, (b) before a box, (c) if we have
	// to or (d) at the end of a box.
	if (cur->isSpace() || (i&&i->isBox) || noSpaceFound || !i){
	    lastPar = par;
	    lastSpace = cur;
	    lastHeight = rh;
	    lastAsc = rasc;
	    lastDesc = rdesc;
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

    t = i;
}



QMLRow::~QMLRow()
{
}


void QMLRow::draw(QMLContainer* box, QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg,  bool onlyDirty, bool onlySelection)
{
    static QString s;

    if (!intersects(cx-obx, cy-oby, cw,ch)) {
	dirty = FALSE;
	return;
    }


    if (start->isBox) {
	//we have to draw the box
	((QMLBox*)start)->draw(p, obx+x, oby+y, ox, oy, cx, cy, cw, ch,
			       backgroundRegion, cg, dirty?FALSE:onlyDirty, onlySelection);
	dirty = FALSE;
	return;
    }

    QRegion r(x+obx-ox, y+oby-oy, width, height);

    backgroundRegion = backgroundRegion.subtract(r);

    if (onlyDirty) {
	if (!dirty)
	    return;
    }

    if (!onlyDirty && !onlySelection) {
	if ( cg.fillBase().pixmap() )
	    p->drawTiledPixmap(x+obx-ox, y+oby-oy, width, height, *cg.fillBase().pixmap(), x+obx, y+oby);
	else
	    p->fillRect(x+obx-ox, y+oby-oy, width, height, cg.base());
    }

    dirty = FALSE;
    QMLNode* t = start;
    QMLContainer* par = parent;

    int tx = x;

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
		if (t==end)
		    p->fillRect(tx+obx-ox, y+oby-oy, width-(tx-x), height, cg.highlight());
		else
		    p->fillRect(tx+obx-ox, y+oby-oy, tw, height, cg.highlight());
		p->setPen( cg.highlightedText() );
	    }
	    else if (onlyDirty || onlySelection) {
		if (t==end){
		    if ( cg.fillBase().pixmap() )
			p->drawTiledPixmap(tx+obx-ox, y+oby-oy, width-(tx-x), height,
					   *cg.fillBase().pixmap(), tx+obx, y+oby);
		    else
			p->fillRect(tx+obx-ox, y+oby-oy, width-(tx-x), height, cg.base());
		}
		else {
		    if ( cg.fillBase().pixmap() )
			p->drawTiledPixmap(tx+obx-ox, y+oby-oy, tw, height,
					   *cg.fillBase().pixmap(), tx+obx, y+oby);
		    else
			p->fillRect(tx+obx-ox, y+oby-oy, tw, height, cg.base());
		}
	    }
	
	    if (t->isSimpleNode) {
		p->drawText(tx+obx-ox, y+oby-oy+base, s);
	    }
	    else {
		int h = ((QMLCustomNode*)t)->height;
		((QMLCustomNode*)t)->draw(p,tx+obx,y+oby+base-h,
					  ox, oy, cx, cy, cw, ch);
	    }
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
    lx = x;
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
QMLContainer::QMLContainer( const QMLStyle &stl)
{
    isSimpleNode = 0;
    isContainer = 1;
    style = &stl;
    fnt = 0;
    parent = 0;
    child = 0;
    attributes_ = 0;
}

QMLContainer::QMLContainer( const QMLStyle &stl, const QDict<QString>& attr )
{
    isSimpleNode = 0;
    isContainer = 1;
    style = &stl;
    fnt = 0;
    parent = 0;
    child = 0;
    attributes_ = 0;
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
    QMLContainer* result = new QMLContainer(*style);
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


const QDict<QString>& QMLContainer::attributes() const
{
    if (!attributes_) {
	QMLContainer* that = (QMLContainer*) this;
	that->attributes_ = new QDict<QString>;
    }
    return * attributes_;
}

const QMLContainer* QMLContainer::activeContainer() const
{
    if ( style->isActive() )
	return this;
    if ( parent )
	return parent->activeContainer();
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
    fnt = parent?new QFont(parent->font()) : new QFont("times");
    fnt->setPointSize( fontSize() );
    fnt->setWeight( fontWeight() );
    QMLStyle::FontStyle s = fontStyle();
    if ( s == QMLStyle::style_italic)
	fnt->setItalic( TRUE );
    if ( s == QMLStyle::style_oblique)
	fnt->setItalic( TRUE );
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

QMLBox::QMLBox( const QMLStyle &stl, const QDict<QString>& attr )
    :QMLContainer(stl, attr)
{
    rows.setAutoDelete(true);
    isSimpleNode = 0;
    isBox = 1;
    width = height = 0;
}

QMLContainer* QMLBox::copy() const
{
    QMLBox* result = new QMLBox(*style);
    return result;
}

QMLBox::~QMLBox()
{
}



#define IN16BIT(x) QMAX( (2<<15)-1, x)

void QMLBox::draw(QPainter *p,  int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, bool onlyDirty, bool onlySelection)
{
    if (onlySelection && !isSelectionDirty)
	return;
    isSelectionDirty = 0;
    for (QMLRow* row = rows.first(); row; row = rows.next()) {
	row->draw(this, p, obx, oby, ox, oy, cx, cy, cw, ch, backgroundRegion, cg, onlyDirty, onlySelection);
    }

}


void QMLBox::resize(QPainter* p, int newWidth, bool forceResize)
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
    int h = 0;

    int ncols = style->numberOfColumns();
    int colwidth = newWidth / ncols;
    if (colwidth < 10)
	colwidth = 10;

    QMLContainer* par = this;
    QMLNode* n = nextLayout( this, par);
    QMLRow* row = 0;

    int border = parent?0:5;

    while (n) {
	if (n->isBox){
	    ((QMLBox*)n)->resize(p, colwidth-2*border); // todo this can be done in word wrap?!
	}
	row = new QMLRow(this, p, n, par, colwidth-2*border);
	row->x = border;
	row->y = h;
	rows.append(row);
	h += row->height;
    }

    // do multi columns if required. Also check with the old rows to
    // optimize the refresh

    row = rows.first();
    QMLRow* old = oldRows.first();
    height = 0;
    h /= ncols;
    for (int col = 0; col < ncols; col++) {
	int colheight = 0;
	for (; row && colheight < h; row = rows.next()) {
	    row->x = col  * colwidth + border;
	    row->y = colheight;
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
	
	    colheight += row->height;
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
	    QMLRow tr (this, p, n, par, r->width);
	    fast_exit &= r->end == tr.end && r->height == tr.height;
	}
	if (fast_exit) {
	    r->dirty = TRUE;
	    return;
	}
    }

    int oldHeight = height;
    resize(p, width, TRUE);

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


QMLProvider::QMLProvider()
{
    images.setAutoDelete( TRUE );
    documents.setAutoDelete( TRUE );
}

QMLProvider::~QMLProvider()
{
}


QMLProvider& QMLProvider::defaultProvider()
{
    //### pseudo memory leak, needs clean up
    static QMLProvider* defaultprovider = 0;
    if (!defaultprovider)
	defaultprovider = new QMLProvider;
    return *defaultprovider;
}

/*!  Bind the given name to a pixmap. The pixmap can be accessed with
  image() */
void QMLProvider::setImage(const QString& name, const QPixmap& pm)
{
    images.insert(name, new QPixmap(pm));
}

/*!  Return the corresponding image for the given name.
\sa insert()
*/
QPixmap QMLProvider::image(const QString &name) const
{
    QPixmap* p = images[name];
    if (p)
	return *p;
    else
	return QPixmap();
}

/*!  Bind the given name to a document. The document can be accessed with
  document() */
void QMLProvider::setDocument(const QString &name, const QString& doc)
{
    documents.insert(name, new QString(doc));
}

/*!  Return the corresponding document for the given name.
\sa insert()
*/
QString QMLProvider::document(const QString &name) const
{
    QString* s = documents[name];
    if (s)
	return *s;
    else
	return QString();
}



//************************************************************************


QMLDocument::QMLDocument( const QString &doc)
    :QMLBox( QMLStyleSheet::defaultSheet().defaultStyle() )
{
    provider_ = new QMLProvider; // for access during parsing only
    sheet_ = &QMLStyleSheet::defaultSheet();// for access during parsing only
    init( doc );
    delete provider_;
}

QMLDocument::QMLDocument( const QString &doc, const QMLProvider& provider)
    :QMLBox( QMLStyleSheet::defaultSheet().defaultStyle() )
{
    provider_ = &provider; // for access during parsing only
    sheet_ = &QMLStyleSheet::defaultSheet();// for access during parsing only
    init( doc );
}

QMLDocument::QMLDocument(const QString &doc,  const QMLProvider& provider,
			 const QMLStyleSheet& sheet )
    : QMLBox( sheet.defaultStyle())
{

    provider_ = &provider; // for access during parsing only
    sheet_ = &sheet; // for access during parsing only
    init( doc );
}

void QMLDocument::init( const QString& doc )
{
    valid = TRUE;
    openChar = new QChar('<');
    closeChar = new QChar('>');
    slashChar = new QChar('/');
    quoteChar = new QChar('"');
    equalChar = new QChar('=');
    int pos = 0;
    parse(this, 0, doc, pos);
    if (!valid)
	debug("qml document not valid!");
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



QString QMLDocument::firstTag( const QString& doc)
{
    QString tag;
    int i = 0;
    while ( i < int(doc.length()) && doc[i].isSpace() )
	i++;
    if ( i < int(doc.length()) && doc[i] == QChar('<') ) {
	i++;
	while ( i < int(doc.length()) && doc[i].isSpace() )
	    i++;
	int l = 1;
	while ( i+l < int(doc.length()) &&
		!doc[i+l].isSpace() && doc[i+l] != QChar('>') )
	    l++;
	tag = doc.mid( i,l ).lower();
    }
    debug("first tag is %s", tag.ascii() );
    return tag;
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
	    QDict<QString> attr;
	    attr.setAutoDelete( TRUE );
	    QString tagname = parseOpenTag(doc, pos, attr);
	    sep = eatSpace(doc, pos);
	    const QMLStyle& style = sheet_->style(tagname);

	    QMLNode* emptytag = sheet_->emptyTag(style, attr, *provider_);
	    if (emptytag) {
		if (valid) {
		    sep |= eatSpace(doc, pos);
		    QMLNode* l = lastChild;
		    if (!l){
			current->child  = emptytag;
			emptytag->isLastSibling = 1;
		    }
		    else {
			l->next = emptytag;
			l->isLastSibling = 0;
		    }
		    emptytag ->next = current;
		    emptytag->isLastSibling = 1;
		    lastChild = emptytag;
		}
	    }
	    else {
		QMLContainer* tag = sheet_->tag(style, attr, *provider_);
	
		if (current == this &&
		    (style.name() == "qml" || style.name() == "qwhatsthis") ) {
		    setAttributes( attr );
		}

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
		
		    parse(tag, 0, doc, pos);
		    sep |= eatSpace(doc, pos);
		    valid = (hasPrefix(doc, pos, *openChar)
			     && hasPrefix(doc, pos+1, *slashChar)
			     && eatCloseTag(doc, pos, tagname) );
		    if (!valid)
			return;
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
	//     debug("parsed key %s", key.ascii());
	if (eat(doc, pos, *equalChar)) {
	    eatSpace(doc, pos);
	    QString value = parseWord(doc, pos, TRUE);
	    //       debug("parsed value %s", value.ascii());
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


QMLView::QMLView(QWidget *parent, const char *name)
    : QScrollView( parent, name)
{
    init();
}


QMLView::QMLView( const QString& doc, QWidget *parent, const char *name)
    : QScrollView( parent, name)
{
    init();
    txt = doc;
}


void QMLView::init()
{
    mypapcolgrp = palette().normal();
    papcolgrp = mypapcolgrp;

    setKeyCompression( TRUE );
    setVScrollBarMode( AlwaysOn );
    setHScrollBarMode( AlwaysOff );

    doc_ = 0;
    sheet_ = 0;
    provider_ = 0;
    txt = "<p></p>";

    viewport()->setBackgroundMode(NoBackground);
}

QMLView::~QMLView()
{
    delete doc_;
    delete sheet_;
}

void QMLView::setContents( const QString& doc)
{
    delete doc_;
    doc_ = 0;
    txt = doc;
    if ( txt.isEmpty() )
	txt = "<p></p>";
    if ( isVisible() ) {
	{
	    QPainter p( this );
	    currentDocument().resize(&p, viewport()->width());
	}
	resizeContents(currentDocument().width, currentDocument().height);
	updateContents( contentsX(), contentsY(), viewport()->width(), viewport()->height() );
	viewport()->setCursor( arrowCursor );
    }
}


QString QMLView::contents() const
{
    return txt;
}

void QMLView::createDocument()
{
    papcolgrp = mypapcolgrp;
    doc_ = new QMLDocument( txt, provider(), styleSheet() );
    if (doc_->attributes()["bgcolor"]){
	QColor  col ( doc_->attributes()["bgcolor"]->ascii() );
	if ( col.isValid() )
	    papcolgrp.setBase( col );
    }
    if (doc_->attributes()["text"]){
	QColor  col ( doc_->attributes()["text"]->ascii() );
	if ( col.isValid() )
	    papcolgrp.setText( col );
    }
    if (doc_->attributes()["bgpixmap"]){
	QPixmap pm = provider().image(*doc_->attributes()["bgpixmap"]);
	if (!pm.isNull())
	    papcolgrp.setBase( QBrush(papcolgrp.base(), pm) );
    }
}


QMLStyleSheet& QMLView::styleSheet() const
{
    if (!sheet_)
	return QMLStyleSheet::defaultSheet();
    else
	return *sheet_;

}

void QMLView::setStyleSheet( QMLStyleSheet* styleSheet )
{
    if (sheet_ != styleSheet) {
	delete sheet_;
    }
    sheet_ = styleSheet;
}


QMLProvider& QMLView::provider() const
{
    if (!provider_)
	return QMLProvider::defaultProvider();
    else
	return *provider_;

}

void QMLView::setProvider( QMLProvider* newProvider )
{
    if ( provider_ != newProvider ) {
	delete provider_;
    }
    provider_ = newProvider;
}


void QMLView::setPaperColorGroup( const QColorGroup& colgrp)
{
    mypapcolgrp = colgrp;
    papcolgrp = colgrp;
}

void QMLView::setPaperPixmap( const QPixmap& pm)
{
    mypapcolgrp.setBase( QBrush(mypapcolgrp.base(), pm) );
    papcolgrp.setBase( QBrush( papcolgrp.base(), pm) );
}

const QColorGroup& QMLView::paperColorGroup() const
{
    return papcolgrp;
}


QString QMLView::title() const
{
    QString* s = currentDocument().attributes()["title"];
    if (s)
	return *s;
    else
	return QString();
}

int QMLView::heightForWidth( int w ) const
{
    QMLDocument doc ( txt, provider(), styleSheet() );
    {
	QPainter p( this );
	doc.resize(&p, w);
    }
    return doc.height;
}

void QMLView::drawContentsOffset(QPainter* p, int ox, int oy,
				 int cx, int cy, int cw, int ch)
{
    QRegion r(cx-ox, cy-oy, cw, ch);
    currentDocument().draw(p, 0, 0, ox, oy, cx, cy, cw, ch, r, paperColorGroup());

    p->setClipRegion(r);

    if ( paperColorGroup().fillBase().pixmap() )
	p->drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			   *paperColorGroup().fillBase().pixmap(), ox, oy);
    else
	p->fillRect(0, 0, viewport()->width(), viewport()->height(), paperColorGroup().base());

    qApp->syncX();

    p->setClipping( FALSE );
}

void QMLView::resizeEvent(QResizeEvent* e)
{
    QScrollView::resizeEvent(e);
    {
	QPainter p( this );
	currentDocument().resize(&p, viewport()->width());
    }
    resizeContents(currentDocument().width, currentDocument().height);
    updateContents( contentsX(), contentsY(), viewport()->width(), viewport()->height() );
}

void QMLView::viewportMousePressEvent( QMouseEvent* )
{
    debug("QMLView::viewportMousePressEvent");
}

void QMLView::viewportMouseReleaseEvent( QMouseEvent* )
{
    debug("QMLView::viewportMouseReleaseEvent");
}

void QMLView::viewportMouseMoveEvent( QMouseEvent* )
{
    debug("QMLView::viewportMouseMoveEvent");
}

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


void QMLView::paletteChange( const QPalette & )
{
    mypapcolgrp = palette().normal();
}


//************************************************************************

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

    //   QString text = "<h1>Unordered list</h1><p>Bla bla bla</p><ul>ul<item>erstens</item><item>zweitens</item><item>drittens</item>/ul</ul><p>Ein Absatz nach der Liste</p>";

    //   QString text = "<h1>Unordered list</h1><p>Bla bla bla</p><ul>ul<item>erstens</item><item>zweitens</item><item>drittens</item>ul</ul><p>Ein Absatz nach der Liste</p><p>Schluss</p>";

    //  QString text = "<h1>Unordered list</h1><p>Bla bla bla <ul><item>erstens</item><item>zweitens</item><item>drittens</item>ul</ul> Ein Absatz nach der Liste</p><p>Schluss</p>";

    //    QString text = "<p>Ein Absatz. Ein Absatz.Ein Absatz.Ein Absatz.Ein Absatz.Ein Absatz.Ein Absatz.Ein Absatz.Ein Absatz.</p><p>Noch ein Absatz. Ein Absatz.<em> Ein Absatz.<ul><item>Ein Absatz in einem Absatz.<h1>Heading</h1>Immernoch Absatz in einem Absatz</item><item>second item</item></ul>Ein Absatz.Ein Absatz.</em> Ein Absatz.Ein Absatz.</p>";

    //   QString text = "<p>Ein Absatz <em>emphasize<h1>Heading in Absatz</h1>Absatz geht weiter<p>Noch ein Absatz INNERHALB </p> Hier geht's </em> weiter</p>";

    //   QString text = "<p>Ein Absatz emphasize<h1>Heading in Absatz</h1>Absatz geht weiter<p>Noch ein Absatz INNERHALB </p> Hier geht's weiter</p>";


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

    if (e->key() == Key_Plus)
	exit(2); // profiling


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
		    // 		debug("length()=%d", e->text().length());
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
			   r, paperColorGroup(), FALSE, TRUE);
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
	    // 	    debug("yes %d", contentsY()+viewport()->height()-cursor->y-cursor->height);
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
			       r, paperColorGroup(), TRUE);
	p.setClipRegion(r);
	if ( paperColorGroup().fillBase().pixmap() )
	    p.drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			      *paperColorGroup().fillBase().pixmap(), contentsX(), contentsY());
	else
	    p.fillRect(0, 0, viewport()->width(), viewport()->height(), paperColorGroup().base());
    }
    showCursor();
    resizeContents(currentDocument().width, currentDocument().height);
    ensureVisible(cursor->x, cursor->y);
}

void QMLEdit::resizeEvent(QResizeEvent* e)
{
    QMLView::resizeEvent(e);
    {
	QPainter p( this );
	cursor->calculatePosition(&p);
    }
}

//************************************************************************

QMLBrowser::QMLBrowser(QWidget *parent, const char *name)
    : QMLView( parent, name )
{
    viewport()->setMouseTracking( TRUE );
    goBackwards = 0;
    buttonDown = 0;
    highlight = 0;
}

QMLBrowser::~QMLBrowser()
{
}


void QMLBrowser::setDocument(const QString& name)
{
    QString doc = provider().document( name );
    if ( isVisible() && QMLDocument::firstTag( doc ) == "qwhatsthis" ){
	popupDefinition( doc, lastClick );
	return;
    }

    // put onto stack #### TODO
    setContents( provider().document(name) );
}

void QMLBrowser::setContents( const QString& contents )
{
    QMLView::setContents( contents );
    emit contentsChanged();
}

// QPixmap QMLBrowser::image(const QString &name) const
// {
//     QPixmap pm = QMLView::image( name );
//     if ( !pm.isNull() )
// 	return pm;
//     return QPixmap( searchPath + name );

// }

// QString QMLBrowser::document(const QString &name) const
// {
//     QString doc = QMLView::document( name );
//     if ( !doc.isNull() )
// 	return doc;

//     QFile f( searchPath + name );
//     if ( f.open( IO_ReadOnly ) ) {
// 	int c;
// 	while ( (c = f.getch()) >= 0 ) {
// 	    doc += (char)c;
// 	}
// 	f.close();
//     }

//     return doc;
// }


// void QMLBrowser::setPath( const QString &path )
// {
//     searchPath = path;
// }


void QMLBrowser::backward()
{
    goBackwards = 1;
    goBackwards = 0;
}

void QMLBrowser::forward()
{
}

void QMLBrowser::viewportMousePressEvent( QMouseEvent* e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = activeContainer( e->pos() );
	lastClick = e->globalPos();
    }
}

void QMLBrowser::viewportMouseReleaseEvent( QMouseEvent* e)
{
    if ( e->button() == LeftButton ) {
	if (buttonDown && buttonDown == activeContainer( e->pos() )){
	    QString href;
	    if ( buttonDown->attributes()["href"])
		href = *buttonDown->attributes()["href"];
	    setDocument( href );
	}
    }
    buttonDown = 0;
}

void QMLBrowser::viewportMouseMoveEvent( QMouseEvent* e)
{
    const QMLContainer* act = activeContainer( e->pos() );
    if (highlight != act) {
	QString href;
	viewport()->setCursor( act?upArrowCursor:arrowCursor );
	if (act && act->attributes()["href"])
	    href = *act->attributes()["href"];
	debug("highlight: %s", href.ascii());
	emit highlighted( href );
    }
    highlight = act;
}


const QMLContainer* QMLBrowser::activeContainer( const QPoint& pos)
{
    QPainter p( viewport() );
    QMLNode* n = currentDocument().hitTest(&p, 0, 0,
					   contentsX() + pos.x(),
					   contentsY() + pos.y());
    if (n)
	return n->parent()->activeContainer();
    return 0;
}


class QMLDefinitionPopup : public QFrame
{
public:
    QMLDefinitionPopup( const QString& txt)
	: QFrame ( 0, 0, WType_Popup )
	{
	    QMLView* view = new QMLView( this );
	    view->setVScrollBarMode( QScrollView::Auto );
	    view->setContents( txt );

	    int h = view->heightForWidth( width() );

	    if ( h < height () ) {
		resize( width(), h );
	    }
	    view->setGeometry(0, 0, width(), height() );
	    view->viewport()->installEventFilter( this );

	}

protected:
    void hideEvent( QHideEvent * ) //###### need real popup function, see qwhatsthis
	{
	    debug("hide event, delete myself!");
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


void QMLBrowser::popupDefinition( const QString& contents, const QPoint& pos )
{
    QMLDefinitionPopup* popup = new QMLDefinitionPopup( contents );

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


//************************************************************************

int main( int argc, char* argv[] ){
    QApplication a(argc,argv);
    //QMLBrowser t;
    QMLEdit t;


    QString text = "<qml title=\"Hallo Du Da!\" bgcolor=\"dudel\" bgpixmap=\"marble01.bmp\" text=\"dudel\"><p>Hello <EM>this is <B>bold</B> italic</EM> this is <B>bold   </B> :-) </p><p><B>wichtig</B></p><H1>And <a href=\"this\">this</a> is a pretty long <EM>heading</EM> in 24 point font!</H1><p>a<large>Grosser Text</large>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text and an image <img SOURCE= \"qt.bmp\" >.  This is another huge paragraph, it contains more or less stupid text. </p><H1> Heading in paragraph </H1><p>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. </p><p2>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. <h1>This is a heading inside the p2 environment</h1>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. </p2><p3>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text.</p3></qml>";

    QPixmap pm("qt.bmp");
    t.provider().setImage("qt.bmp", pm);
    t.provider().setImage("marble01.bmp", QPixmap("marble01.bmp"));

    t.setContents(text);


    // t.setDocument("beispiel.qml");
    // t.setDocument("heading.qml");

    //t.setPaperPixmap( QPixmap("marble01.bmp"));

    debug("title ist %s", t.title().ascii() );

    a.setMainWidget(&t);
    t.show();

    return a.exec();
}

