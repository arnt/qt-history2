#include <qtextlayout.h>
#include <qtextengine.h>

#include <qfont.h>
#include <qapplication.h>
#include <qpainter.h>



int QTextItem::width() const
{
    return engine->items[item].width;
}

int QTextItem::ascent() const
{
    return engine->items[item].ascent;
}

int QTextItem::descent() const
{
    return engine->items[item].descent;
}

int QTextItem::baselineAdjustment() const
{
    return engine->items[item].baselineAdjustment;
}


void QTextItem::setWidth( int w )
{
    engine->items[item].width = w;
}

void QTextItem::setAscent( int a )
{
    engine->items[item].ascent = a;
}

void QTextItem::setDescent( int d )
{
    engine->items[item].descent = d;
}


void QTextItem::setBaselineAdjustment( int adjust )
{
    engine->items[item].baselineAdjustment = adjust;
}


void QTextItem::setFont( const QFont & f )
{
    engine->setFont( item, f.d );
}


int QTextItem::cursorToX( int *cPos, Edge edge = Leading )
{
    int pos = *cPos;
    QScriptItem &si = engine->items[item];

    const QShapedItem *shaped = si.shaped;
    if ( !shaped )
	shaped = engine->shape( item );

    int l = engine->length( item );
    if ( pos > l )
	pos = l;
    if ( pos < 0 )
	pos = 0;

    int glyph_pos = pos == l ? shaped->num_glyphs : shaped->logClusters[pos];
    if ( edge == Trailing ) {
	// trailing edge is leading edge of next cluster
	while ( glyph_pos < shaped->num_glyphs && !shaped->glyphAttributes[glyph_pos].clusterStart )
	    glyph_pos++;
    }

    int x = 0;
    bool reverse = engine->items[item].analysis.bidiLevel % 2;

    if ( reverse ) {
	for ( int i = shaped->num_glyphs-1; i >= glyph_pos; i-- )
	    x += shaped->advances[i].x;
    } else {
	for ( int i = 0; i < glyph_pos; i++ )
	    x += shaped->advances[i].x;
    }
//     qDebug("cursorToX: pos=%d, gpos=%d x=%d", pos, glyph_pos, x );
    *cPos = pos;
    return x;
}

int QTextItem::xToCursor( int x )
{
    QScriptItem &si = engine->items[item];
    const QShapedItem *shaped = si.shaped;
    if ( !shaped )
	shaped = engine->shape( item );

    int l = engine->length( item );
    bool reverse = si.analysis.bidiLevel % 2;
    if ( x < 0 )
	return reverse ? l : 0;


    if ( reverse ) {
	int width = 0;
	for ( int i = 0; i < shaped->num_glyphs; i++ ) {
	    width += shaped->advances[i].x;
	}
	x = -x + width;
    }
    int cp_before = 0;
    int cp_after = 0;
    int x_before = 0;
    int x_after = 0;

    int lastCluster = 0;
    for ( int i = 1; i <= l; i++ ) {
	int newCluster = i < l ? shaped->logClusters[i] : shaped->num_glyphs;
	if ( newCluster != lastCluster ) {
	    // calculate cluster width
	    cp_before = cp_after;
	    x_before = x_after;
	    cp_after = i;
	    for ( int j = lastCluster; j < newCluster; j++ )
		x_after += shaped->advances[j].x;
	    // 		qDebug("cluster boundary: lastCluster=%d, newCluster=%d, x_before=%d, x_after=%d",
	    // 		       lastCluster, newCluster, x_before, x_after );
	    if ( x_after > x )
		break;
	    lastCluster = newCluster;
	}
    }

    bool before = (x - x_before) < (x_after - x);
//     qDebug("got cursor position for %d: %d/%d, x_ba=%d/%d using %d",
// 	   x, cp_before,cp_after,  x_before, x_after,  before ? cp_before : cp_after );

    return before ? cp_before : cp_after;

}


bool QTextItem::isRightToLeft() const
{
    return (engine->items[item].analysis.bidiLevel % 2);
}

bool QTextItem::isObject() const
{
    return (engine->string.at(engine->items[item].position).unicode() == 0xfffc);
}




QTextLayout::QTextLayout( const QString &string, QPainter *p = 0 )
{
    QFontPrivate *f = p ? p->font().d : QApplication::font().d;
    d = new QTextEngine( string, f );
}

QTextLayout::QTextLayout( const QString &string, const QFont &fnt )
{
    d = new QTextEngine( string, fnt.d );
}

QTextLayout::~QTextLayout()
{
    delete d;
}

/* add an additional item boundary eg. for style change */
void QTextLayout::setBoundary( int strPos )
{
    d->items.split( strPos );
}


bool QTextLayout::validCursorPosition( int strPos )
{
    return d->attributes()[strPos].charStop;
}


int QTextLayout::numItems() const
{
    return d->items.size();
}

QTextItem QTextLayout::itemAt( int i )
{
    return QTextItem( i, d );
}


void QTextLayout::beginLayout()
{
    d->items.clear();
    d->itemize();
    d->currentItem = -1;
    d->firstItemInLine = -1;
}

void QTextLayout::beginLine( int width )
{
    d->lineWidth = width;
    d->widthUsed = 0;
    d->firstItemInLine = -1;
}

bool QTextLayout::hasNextItem() const
{
    return d->currentItem < d->items.size() - 1;
}

QTextItem QTextLayout::nextItem()
{
    d->currentItem++;
    d->shape( d->currentItem );
    return QTextItem( d->currentItem, d );
}

/* ## maybe also currentItem() */
void QTextLayout::setLineWidth( int newWidth )
{
    d->lineWidth = newWidth;
}

int QTextLayout::availableWidth() const
{
    return d->lineWidth - d->widthUsed;
}


/* returns true if completely added */
QTextLayout::Result QTextLayout::addCurrentItem()
{
    QScriptItem &current = d->items[d->currentItem];

    if ( d->firstItemInLine == -1 )
	d->firstItemInLine = d->currentItem;

    int width = d->lineWidth - d->widthUsed;
    if ( current.width < width ) {
	// simple case, we can add it as is, there's nothing to do for now
	d->widthUsed += current.width;
	return Ok;
    }

    // have to split it.

    // line breaks are always done in logical order

    int length = d->length( d->currentItem );
    // this ensures it's really shaped
    QShapedItem *shaped = d->shape( d->currentItem );

    int lastBreak = 0;
    int splitGlyph = 0;

    int lastWidth = 0;
    int w = 0;
    int lastCluster = 0;
    int clusterStart = 0;

    const QCharAttributes *attrs = d->attributes();

    for ( int i = 1; i <= length; i++ ) {
	int newCluster = i < length ? shaped->logClusters[i] : shaped->num_glyphs;
	if ( newCluster != lastCluster ) {
	    // calculate cluster width
	    int x = 0;
	    for ( int j = lastCluster; j < newCluster; j++ )
		x += shaped->advances[j].x;
	    lastWidth += x;
	    if ( w + lastWidth > width )
		break;
	    lastCluster = newCluster;
	    clusterStart = i;
	    if ( attrs[i + current.position].softBreak ) {
		lastBreak = i;
		splitGlyph = newCluster;
		w += lastWidth;
		lastWidth = 0;
	    }
	}
    }

    if ( lastBreak == 0 )
	return Error;

    // we have the break position
    d->items.split( lastBreak + d->items[d->currentItem].position );

    // split the shapedItem
    QShapedItem *split = new QShapedItem;
    split->ownGlyphs = FALSE;
    split->num_glyphs = shaped->num_glyphs - splitGlyph - 1;
    split->glyphs = shaped->glyphs + splitGlyph;
    split->offsets = shaped->offsets + splitGlyph;
    split->advances = shaped->advances + splitGlyph;
    split->glyphAttributes = shaped->glyphAttributes + splitGlyph;
    split->logClusters = shaped->logClusters+lastBreak;
    for ( int i = 0; i < length-lastBreak; i++ )
	split->logClusters[i] -= splitGlyph;

    shaped->num_glyphs = splitGlyph;
    d->items[d->currentItem+1].shaped = split;

    d->widthUsed += w;

    return Split;
}


void QTextLayout::endLine( int x, int y, Qt::AlignmentFlags alignment )
{
    if ( d->firstItemInLine == -1 )
	return;

    // position the objects in the line
    int available = d->lineWidth - d->widthUsed;

    // ### FIXME
    if ( alignment == Qt::AlignAuto )
	alignment = Qt::AlignLeft;

    int numRuns = d->currentItem - d->firstItemInLine + 1;
    Q_UINT8 _levels[128];
    int _visual[128];
    Q_UINT8 *levels = _levels;
    int *visual = _visual;
    if ( numRuns > 127 ) {
	levels = new Q_UINT8[numRuns];
	visual = new int[numRuns];
    }
    int i;
    for ( i = 0; i < numRuns; i++ )
	levels[i] = d->items[i].analysis.bidiLevel;
    d->bidiReorder( numRuns, levels, visual );

    if ( alignment & Qt::AlignJustify ) {
	// #### justify items
    }

    if ( alignment & Qt::AlignRight )
	x += available;
    else if ( alignment & Qt::AlignCenter )
	x += available/2;

    for ( i = 0; i < numRuns; i++ ) {
	QScriptItem &si = d->items[d->firstItemInLine+visual[i]];
	si.x = x;
	si.y = y;
	x += si.width;
    }
}

void QTextLayout::endLayout()
{
    for ( int i = 0; i < d->items.size(); i++ ) {
	QScriptItem &si = d->items[i];
	delete si.shaped;
	si.shaped = 0;
    }
}


