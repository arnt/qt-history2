/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtextlayout_p.h"
#include "qtextengine_p.h"

#include <qfont.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qstackarray.h>

#include "qfontengine_p.h"

QRect QTextItem::rect() const
{
    QScriptItem& si = eng->items[itm];
    return QRect( si.x, si.y - si.ascent, si.width, si.ascent+si.descent );
}

int QTextItem::x() const
{
    return eng->items[itm].x;
}

int QTextItem::y() const
{
    return eng->items[itm].y;
}

int QTextItem::width() const
{
    return eng->items[itm].width;
}

int QTextItem::ascent() const
{
    return eng->items[itm].ascent;
}

int QTextItem::descent() const
{
    return eng->items[itm].descent;
}

void QTextItem::setWidth( int w )
{
    eng->items[itm].width = w;
}

void QTextItem::setAscent( int a )
{
    eng->items[itm].ascent = a;
}

void QTextItem::setDescent( int d )
{
    eng->items[itm].descent = d;
}

int QTextItem::from() const
{
    return eng->items[itm].position;
}

int QTextItem::length() const
{
    return eng->length(itm);
}

int QTextItem::custom() const
{
    return eng->items[itm].custom;
}


int QTextItem::cursorToX( int *cPos, Edge edge ) const
{
    int pos = *cPos;
    QScriptItem *si = &eng->items[itm];

    eng->shape( itm );
    QGlyphLayout *glyphs = eng->glyphs(si);
    unsigned short *logClusters = eng->logClusters( si );

    int l = eng->length( itm );
    if ( pos > l )
	pos = l;
    if ( pos < 0 )
	pos = 0;

    int glyph_pos = pos == l ? si->num_glyphs : logClusters[pos];
    if ( edge == Trailing ) {
	// trailing edge is leading edge of next cluster
	while ( glyph_pos < si->num_glyphs && !glyphs[glyph_pos].attributes.clusterStart )
	    glyph_pos++;
    }

    int x = 0;
    bool reverse = eng->items[itm].analysis.bidiLevel % 2;

    if ( reverse ) {
	for ( int i = si->num_glyphs-1; i >= glyph_pos; i-- )
	    x += glyphs[i].advance;
    } else {
	for ( int i = 0; i < glyph_pos; i++ )
	    x += glyphs[i].advance;
    }
//     qDebug("cursorToX: pos=%d, gpos=%d x=%d", pos, glyph_pos, x );
    *cPos = pos;
    return x;
}

int QTextItem::xToCursor( int x, CursorPosition cpos ) const
{
    QScriptItem *si = &eng->items[itm];
    eng->shape( itm );
    QGlyphLayout *glyphs = eng->glyphs( si );
    unsigned short *logClusters = eng->logClusters( si );

    int l = eng->length( itm );
    bool reverse = si->analysis.bidiLevel % 2;
    if ( x < 0 )
	return reverse ? l : 0;


    if ( reverse ) {
	int width = 0;
	for ( int i = 0; i < si->num_glyphs; i++ ) {
	    width += glyphs[i].advance;
	}
	x = -x + width;
    }
    int cp_before = 0;
    int cp_after = 0;
    int x_before = 0;
    int x_after = 0;

    int lastCluster = 0;
    for ( int i = 1; i <= l; i++ ) {
	int newCluster = i < l ? logClusters[i] : si->num_glyphs;
	if ( newCluster != lastCluster ) {
	    // calculate cluster width
	    cp_before = cp_after;
	    x_before = x_after;
	    cp_after = i;
	    for ( int j = lastCluster; j < newCluster; j++ )
		x_after += glyphs[j].advance;
	    // 		qDebug("cluster boundary: lastCluster=%d, newCluster=%d, x_before=%d, x_after=%d",
	    // 		       lastCluster, newCluster, x_before, x_after );
	    if ( x_after > x )
		break;
	    lastCluster = newCluster;
	}
    }

    bool before = ( cpos == OnCharacters || (x - x_before) < (x_after - x) );

//     qDebug("got cursor position for %d: %d/%d, x_ba=%d/%d using %d",
// 	   x, cp_before,cp_after,  x_before, x_after,  before ? cp_before : cp_after );

    return before ? cp_before : cp_after;

}


bool QTextItem::isRightToLeft() const
{
    return (eng->items[itm].analysis.bidiLevel % 2);
}

bool QTextItem::isObject() const
{
    return eng->items[itm].isObject;
}

bool QTextItem::isSpace() const
{
    return eng->items[itm].isSpace;
}

bool QTextItem::isTab() const
{
    return eng->items[itm].isTab;
}


QTextLayout::QTextLayout()
    :d(0)
{}

QTextLayout::QTextLayout(const QString& string)
{
    d = new QTextEngine(string, 0);
}

QTextLayout::QTextLayout( const QString& string, QPainter *p )
{
    QFontPrivate *f = p ? (p->font().d) : QApplication::font().d;
    d = new QTextEngine( (string.isNull() ? (const QString&)QString::fromLatin1("") : string), f );
}

QTextLayout::QTextLayout( const QString& string, const QFont& fnt )
{
    d = new QTextEngine( (string.isNull() ? (const QString&)QString::fromLatin1("") : string), fnt.d );
}

QTextLayout::~QTextLayout()
{
    delete d;
}

void QTextLayout::enableKerning(bool enable)
{
    d->enableKerning(enable);
}

void QTextLayout::setText( const QString& string, const QFont& fnt )
{
    delete d;
    d = new QTextEngine( (string.isNull() ? (const QString&)QString::fromLatin1("") : string), fnt.d );
}

void QTextLayout::setText( const QString& string )
{
    delete d;
    d = new QTextEngine( (string.isNull() ? (const QString&)QString::fromLatin1("") : string), 0 );
}

/* add an additional item boundary eg. for style change */
void QTextLayout::setBoundary( int strPos )
{
    if ( strPos <= 0 || strPos >= (int)d->string.length() )
	return;

    int itemToSplit = 0;
    while ( itemToSplit < d->items.size() && d->items[itemToSplit].position <= strPos )
	itemToSplit++;
    itemToSplit--;
    if ( d->items[itemToSplit].position == strPos ) {
	// already a split at the requested position
	return;
    }
    // we have to ensure we get correct shaping for arabic and other
    // complex languages so we have to call shape _before_ we split the item.
    d->shape(itemToSplit);
    d->splitItem( itemToSplit, strPos - d->items[itemToSplit].position );
}


int QTextLayout::numItems() const
{
    return d->items.size();
}

QTextItem QTextLayout::itemAt( int i ) const
{
    return QTextItem( i, d );
}


QTextItem QTextLayout::findItem( int strPos ) const
{
    if ( strPos == 0 && d->items.size() )
	return QTextItem( 0, d );
    // ## TODO use bsearch
    for ( int i = d->items.size()-1; i >= 0; --i ) {
	if ( d->items[i].position < strPos )
	    return QTextItem( i, d );
    }
    return QTextItem();
}

void QTextLayout::setProperty(int from, int length, const QFont &f, int custom)
{
    d->setProperty(from, length, f.d, custom);
}

void QTextLayout::beginLayout( QTextLayout::LayoutMode m, int textFlags )
{
    d->items.clear();
    QTextEngine::Mode mode = QTextEngine::Full;
    if (m == NoBidi)
	mode = QTextEngine::NoBidi;
    else if (m == SingleLine)
	mode = QTextEngine::SingleLine;
    d->itemize( mode );
    d->currentItem = 0;
    d->firstItemInLine = -1;
    d->textFlags = textFlags;
}

void QTextLayout::beginLine( int width )
{
    d->lineWidth = width;
    d->widthUsed = 0;
    d->firstItemInLine = -1;
}

bool QTextLayout::atEnd() const
{
    return d->currentItem >= d->items.size();
}

QTextItem QTextLayout::nextItem()
{
    d->currentItem++;

    if ( d->currentItem >= d->items.size() )
	return QTextItem();

    d->shape( d->currentItem );
    return QTextItem( d->currentItem, d );
}

QTextItem QTextLayout::currentItem()
{
    if ( d->currentItem >= d->items.size() )
	return QTextItem();

    d->shape( d->currentItem );
    return QTextItem( d->currentItem, d );
}

/* ## maybe also currentItem() */
void QTextLayout::setLineWidth( int newWidth )
{
    d->lineWidth = newWidth;
}

int QTextLayout::lineWidth() const
{
    return d->lineWidth;
}

int QTextLayout::widthUsed() const
{
    return d->widthUsed;
}

int QTextLayout::availableWidth() const
{
    return d->lineWidth - d->widthUsed;
}


/* returns true if completely added */
QTextLayout::Result QTextLayout::addCurrentItem()
{
    if ( d->firstItemInLine == -1 )
	d->firstItemInLine = d->currentItem;
    QScriptItem &current = d->items[d->currentItem];
    d->shape( d->currentItem );
    d->widthUsed += current.width;
//      qDebug("trying to add item %d with width %d, remaining %d", d->currentItem, current.width, d->lineWidth-d->widthUsed );

    d->currentItem++;

    return (d->widthUsed <= d->lineWidth
	    || (d->currentItem < d->items.size() && d->items[d->currentItem].isSpace)) ? Ok : LineFull;
}

QTextLayout::Result QTextLayout::endLine( int x, int y, int alignment,
					  int *ascent, int *descent, int *lineLeft, int *lineRight )
{
    int available = d->lineWidth;
    int numRuns = 0;
    int numSpaceItems = 0;
    Q_UINT8 _levels[128];
    int _visual[128];
    Q_UINT8 *levels = _levels;
    int *visual = _visual;
    int i;
    QTextLayout::Result result = LineEmpty;

//     qDebug("endLine x=%d, y=%d, first=%d, current=%d lw=%d wu=%d", x,  y, d->firstItemInLine, d->currentItem, d->lineWidth, d->widthUsed );
    if ( d->firstItemInLine == -1 )
	goto end;

    if ( !(alignment & (Qt::SingleLine|Qt::IncludeTrailingSpaces))
	&& d->currentItem > d->firstItemInLine && d->items[d->currentItem-1].isSpace ) {
	int i = d->currentItem-1;
	while ( i > d->firstItemInLine && d->items[i].isSpace ) {
	    numSpaceItems++;
	    d->widthUsed -= d->items[i--].width;
	}
    }

    if ( (alignment & (Qt::WordBreak|Qt::BreakAnywhere)) &&
	 d->widthUsed > d->lineWidth ) {
	// find linebreak

	// even though we removed trailing spaces the line was too wide. We'll have to break at an earlier
	// position. To not confuse the layouting below, reset the number of space items
	numSpaceItems = 0;


	bool breakany = alignment & Qt::BreakAnywhere;

	const QCharAttributes *attrs = d->attributes();
	int w = 0;
	int itemWidth = 0;
	int breakItem = d->firstItemInLine;
	int breakPosition = -1;
#if 0
	// we iterate backwards or forward depending on what we guess is closer
	if ( d->widthUsed - d->lineWidth < d->lineWidth ) {
	    // backwards search should be faster

	} else
#endif
	{
	    int tmpWidth = 0;
	    int swidth = 0;
	    // forward search is probably faster
	    for ( int i = d->firstItemInLine; i < d->currentItem; i++ ) {
		const QScriptItem *si = &d->items[i];
		int length = d->length( i );
		const QCharAttributes *itemAttrs = attrs + si->position;

		QGlyphLayout *glyphs = d->glyphs( si );
		unsigned short *logClusters = d->logClusters( si );

		int lastGlyph = 0;
		int tmpItemWidth = 0;

//    		qDebug("looking for break in item %d, isSpace=%d", i, si->isSpace );
		if(si->isSpace && !(alignment & (Qt::SingleLine|Qt::IncludeTrailingSpaces))) {
		    swidth += si->width;
		} else {
		    tmpWidth += swidth;
		    swidth = 0;
		    for ( int pos = 0; pos < length; pos++ ) {
 			//qDebug("advance=%d, tmpWidth=%d, softbreak=%d, whitespace=%d",
 		    //	   *advances, tmpWidth, itemAttrs->softBreak, itemAttrs->whiteSpace );
			int glyph = logClusters[pos];
			if ( lastGlyph != glyph ) {
			    while ( lastGlyph < glyph )
				tmpItemWidth += glyphs[lastGlyph++].advance;
			    if ( w + tmpWidth + tmpItemWidth > d->lineWidth ) {
// 				qDebug("found break at w=%d, tmpWidth=%d, tmpItemWidth=%d", w, tmpWidth, tmpItemWidth);
				d->widthUsed = w;
				goto found;
			    }
			}
			if ( (itemAttrs->softBreak ||
			      ( breakany && itemAttrs->charStop ) ) &&
			     (i != d->firstItemInLine || pos != 0) ) {
			    if ( breakItem != i )
				itemWidth = 0;
			    if (itemAttrs->softBreak)
				breakany = FALSE;
			    breakItem = i;
			    breakPosition = pos;
//   			    qDebug("found possible break at item %d, position %d (absolute=%d), w=%d, tmpWidth=%d, tmpItemWidth=%d", breakItem, breakPosition, d->items[breakItem].position+breakPosition, w, tmpWidth, tmpItemWidth);
			    w += tmpWidth + tmpItemWidth;
			    itemWidth += tmpItemWidth;
			    tmpWidth = 0;
			    tmpItemWidth = 0;
			}
			itemAttrs++;
		    }
		    while ( lastGlyph < si->num_glyphs )
			tmpItemWidth += glyphs[lastGlyph++].advance;
		    tmpWidth += tmpItemWidth;
		    if ( w + tmpWidth > d->lineWidth ) {
			d->widthUsed = w;
			goto found;
		    }
		}
	    }
	}

    found:
	// no valid break point found
	if ( breakPosition == -1 )
	    goto nobreak;

//  	qDebug("linebreak at item %d, position %d, wu=%d", breakItem, breakPosition, d->widthUsed );
	// split the line
	if ( breakPosition > 0 ) {
// 	    int length = d->length( breakItem );

//  	    qDebug("splitting item, itemWidth=%d", itemWidth);
	    // not a full item, need to break
	    d->splitItem( breakItem, breakPosition );
	    d->currentItem = breakItem+1;
	} else {
	    d->currentItem = breakItem;
	}
    }

    result = Ok;

 nobreak:
    // position the objects in the line
    available -= d->widthUsed;

    numRuns = d->currentItem - d->firstItemInLine - numSpaceItems;
    if ( numRuns > 127 ) {
	levels = new Q_UINT8[numRuns];
	visual = new int[numRuns];
    }

//     qDebug("reordering %d runs, numSpaceItems=%d", numRuns, numSpaceItems );
    for ( i = 0; i < numRuns; i++ ) {
	levels[i] = d->items[i+d->firstItemInLine].analysis.bidiLevel;
// 	qDebug("    level = %d", d->items[i+d->firstItemInLine].analysis.bidiLevel );
    }
    d->bidiReorder( numRuns, levels, visual );

 end:
    // ### FIXME
    if ( alignment & Qt::AlignJustify ) {
	// #### justify items
	alignment = Qt::AlignAuto;
    }
    if ( (alignment & Qt::AlignHorizontal_Mask) == Qt::AlignAuto )
	alignment = Qt::AlignLeft;
    if ( alignment & Qt::AlignRight )
	x += available;
    else if ( alignment & Qt::AlignHCenter )
	x += available/2;


    int asc = ascent ? *ascent : 0;
    int desc = descent ? *descent : 0;

    for ( i = 0; i < numRuns; i++ ) {
	QScriptItem &si = d->items[d->firstItemInLine+visual[i]];
	asc = qMax( asc, si.ascent );
	desc = qMax( desc, si.descent );
    }

    int left = x;
    for ( i = 0; i < numRuns; i++ ) {
	QScriptItem &si = d->items[d->firstItemInLine+visual[i]];
//  	qDebug("positioning item %d with width %d (from=%d/length=%d) at %d", d->firstItemInLine+visual[i], si.width, si.position,
// 	       d->length(d->firstItemInLine+visual[i]), x );
	si.x = x;
	si.y = y + asc;
	x += si.width;
    }
    int right = x;

    if ( numSpaceItems ) {
	if ( d->items[d->firstItemInLine+numRuns].analysis.bidiLevel % 2 ) {
	    x = left;
	    for ( i = 0; i < numSpaceItems; i++ ) {
		QScriptItem &si = d->items[d->firstItemInLine + numRuns + i];
		x -= si.width;
		si.x = x;
		si.y = y + asc;
	    }
	} else {
	    for ( i = 0; i < numSpaceItems; i++ ) {
		QScriptItem &si = d->items[d->firstItemInLine + numRuns + i];
		si.x = x;
		si.y = y + asc;
		x += si.width;
	    }
	}
    }

    if ( lineLeft )
	*lineLeft = left;
    if ( lineRight )
	*lineRight = right;
    if ( ascent )
	*ascent = asc;
    if ( descent )
	*descent = desc;

    if (levels != _levels)
	delete []levels;
    if (visual != _visual)
	delete []visual;

    return result;
}

void QTextLayout::endLayout()
{
    // nothing to do currently
}


int QTextLayout::nextCursorPosition( int oldPos, CursorMode mode ) const
{
//     qDebug("looking for next cursor pos for %d", oldPos );
    const QCharAttributes *attributes = d->attributes();
    int len = d->string.length();
    if ( oldPos >= len )
	return oldPos;
    oldPos++;
    if ( mode == SkipCharacters ) {
	while ( oldPos < len && !attributes[oldPos].charStop )
	    oldPos++;
    } else {
	while ( oldPos < len && !attributes[oldPos].wordStop && !attributes[oldPos-1].whiteSpace )
	    oldPos++;
    }
//     qDebug("  -> %d",  oldPos );
    return oldPos;
}

int QTextLayout::previousCursorPosition( int oldPos, CursorMode mode ) const
{
//     qDebug("looking for previous cursor pos for %d", oldPos );
    const QCharAttributes *attributes = d->attributes();
    if ( oldPos <= 0 )
	return 0;
    oldPos--;
    if ( mode == SkipCharacters ) {
	while ( oldPos && !attributes[oldPos].charStop )
	    oldPos--;
    } else {
	while ( oldPos && !attributes[oldPos].wordStop && !attributes[oldPos-1].whiteSpace )
	    oldPos--;
    }
//     qDebug("  -> %d",  oldPos );
    return oldPos;
}


bool QTextLayout::validCursorPosition( int pos ) const
{
    const QCharAttributes *attributes = d->attributes();
    if ( pos < 0 || pos > (int)d->string.length() )
	return FALSE;
    return attributes[pos].charStop;
}

#define QChar_linesep QChar(0x2028U)

QTextLine QTextLayout::createLine(int from, int y, int x1, int x2)
{
    QScriptLine line;
    line.x = x1;
    line.width = x2-x1;
    line.y = y;
    line.from = from;
    line.length = 0;
    line.textWidth = 0;
    line.ascent = 0;
    line.descent = 0;

    // ######
    // Readd tab support. Add explicit newlines
    bool breakany = d->textFlags & Qt::BreakAnywhere;

    // add to the line until it's filled up.
    if (from < 0 || from >= d->string.length())
	return QTextLine();

    int item;
    for ( item = d->items.size()-1; item > 0; --item ) {
	if ( d->items[item].position <= from )
	    break;
    }

    const QCharAttributes *attributes = d->attributes();

//     qDebug("from: %d:   item=%d, total %d width available %d", from, item, d->items.size(), line.width);
    while (item < d->items.size()) {
	d->shape(item);
	const QScriptItem &current = d->items[item];
	line.ascent = qMax(line.ascent, current.ascent);
	line.descent = qMax(line.descent, current.descent);

	int length = d->length(item);

	const QCharAttributes *itemAttrs = attributes + current.position;
	QGlyphLayout *glyphs = d->glyphs(&current);
	unsigned short *logClusters = d->logClusters(&current);

	int pos = qMax(0, from - current.position);

	do {
	    int next = pos+1;
	    while (next < length && !itemAttrs[next].softBreak && !(breakany && itemAttrs[next].charStop))
		++next;
	    if (itemAttrs[next].softBreak)
		breakany = false;
	    --next;
	    int s = next;
	    if (!(d->textFlags & (Qt::SingleLine|Qt::IncludeTrailingSpaces)))
		while (s > pos && itemAttrs[s].whiteSpace)
		    --s;

	    int gp = logClusters[pos];
	    int gs = logClusters[s];
	    int ge = logClusters[next];

	    int tmpw = 0;
	    while (gp <= gs) {
		tmpw += glyphs[gp].advance;
		++gp;
	    }
	    int spacew = 0;
	    while (gp <= ge) {
		spacew += glyphs[gp].advance;
		++gp;
	    }
// 	    qDebug("possible break at %d, chars (%d-%d): width %d, spacew=%d",
// 		   current.position + next, pos, next, tmpw, spacew);

	    if (line.length && line.textWidth + tmpw > line.width) {
// 		qDebug("found break");
		goto found;
	    }
	    line.textWidth += tmpw + spacew;
	    line.length += next - pos + 1;
	    if (d->string[current.position+pos] == QChar_linesep) {
// 		qDebug("found hard break");
		goto found;
	    }
	    pos = next + 1;
    } while (pos < length);
	++item;
    }
 found:
//     qDebug("line length = %d, ascent=%d, descent=%d", line.length, line.ascent, line.descent);

    int l = d->lines.size();
    d->lines.append(line);
    return QTextLine(l, d);
}

int QTextLayout::numLines() const
{
    return d->lines.size();
}

QTextLine QTextLayout::lineAt(int i) const
{
    return QTextLine(i, d);
}


QRect QTextLine::rect() const
{
    const QScriptLine& si = eng->lines[i];
    return QRect( si.x, si.y - si.ascent, si.width, si.ascent+si.descent );
}

int QTextLine::x() const
{
    return eng->lines[i].x;
}

int QTextLine::y() const
{
    return eng->lines[i].y;
}

int QTextLine::width() const
{
    return eng->lines[i].width;
}

int QTextLine::ascent() const
{
    return eng->lines[i].ascent;
}

int QTextLine::descent() const
{
    return eng->lines[i].descent;
}

int QTextLine::textWidth() const
{
    return eng->lines[i].textWidth;
}

void QTextLine::adjust(int y, int x1, int x2)
{
}

int QTextLine::from() const
{
    return eng->lines[i].from;
}

int QTextLine::length() const
{
    return eng->lines[i].length;
}


void QTextLine::draw( QPainter *p, int x, int y )
{
    const QScriptLine &line = eng->lines[i];

    if (!line.length)
	return;

    int firstItem = eng->findItem(line.from);
    int lastItem = eng->findItem(line.from + line.length - 1);
    int nItems = lastItem-firstItem+1;

    int lineEnd = line.from + line.length;

    x += line.x;
    y += line.y;

    // ########## Justification
    if (eng->textFlags & Qt::AlignRight)
	x += line.width - line.textWidth;
    else if (eng->textFlags & Qt::AlignHCenter)
	x += (line.width - line.textWidth)/2;

    QStackArray<int> visualOrder(nItems);
    QStackArray<unsigned char> levels(nItems);
    for (int i = 0; i < nItems; ++i)
	levels[i] = eng->items[i+firstItem].analysis.bidiLevel;
    QTextEngine::bidiReorder(nItems, levels, visualOrder);

    for (int i = 0; i < nItems; ++i) {
	int item = visualOrder[i]+firstItem;
	QScriptItem &si = eng->items[item];

	if ( si.isTab || si.isObject ) {
	    x += si.width;
	    continue;
	}
	int start = qMax(line.from, si.position);
	int end = qMin(lineEnd, si.position + eng->length(item));

	unsigned short *logClusters = eng->logClusters(&si);

	int gs = logClusters[start-si.position];
	int ge = logClusters[end-si.position-1];

	QGlyphLayout *glyphs = eng->glyphs(&si);

	QFontEngine *fe = si.font();
	Q_ASSERT( fe );

	QGlyphFragment gf;
	gf.analysis = si.analysis;
	gf.hasPositioning = si.hasPositioning;
	gf.ascent = si.ascent;
	gf.descent = si.descent;
	gf.width = si.width;
	gf.num_glyphs = ge - gs + 1;
	gf.glyphs = glyphs + gs;
	fe->draw( p, x, y+line.ascent, gf, 0 /*textFlags*/ );
	while (gs <= ge) {
	    x += glyphs[gs].advance;
	    ++gs;
	}
	++item;
    }
}
