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
    return QTextItem( i,  d );
}


void QTextLayout::beginLayout()
{

}

void QTextLayout::beginLine( int width )
{

}


QTextItem QTextLayout::nextItem()
{

}

/* ## maybe also currentItem() */
void QTextLayout::setLineWidth( int newWidth )
{

}

int QTextLayout::availableWidth() const
{

}


/* returns true if completely added */
bool QTextLayout::addCurrentItem()
{

}


void QTextLayout::endLine( int x, int y, Qt::AlignmentFlags alignment )
{

}

void QTextLayout::endLayout()
{

}


