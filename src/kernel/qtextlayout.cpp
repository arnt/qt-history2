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
#include <qtextformat.h>

#include "qfontengine_p.h"

QRect QTextItem::rect() const
{
    QScriptItem& si = eng->items[itm];
    return QRect( 0, - si.ascent, si.width, si.ascent+si.descent );
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

int QTextItem::format() const
{
    return eng->items[itm].format;
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
{ d = new QTextEngine(); }

QTextLayout::QTextLayout(const QString& string)
{
    d = new QTextEngine();
    d->setText(string);
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

// ####### go away!
void QTextLayout::setText( const QString& string, const QFont& fnt )
{
    delete d;
    d = new QTextEngine( (string.isNull() ? (const QString&)QString::fromLatin1("") : string), fnt.d );
}

void QTextLayout::setFormatCollection(const QTextFormatCollection *formats )
{
    d->setFormatCollection(formats);
}

void QTextLayout::setText( const QString& string )
{
    d->setText(string);
}

QString QTextLayout::text() const
{
    return d->string;
}

void QTextLayout::setInlineObjectInterface(QTextInlineObjectInterface *iface)
{
    d->setInlineObjectInterface(iface);
}

/* add an additional item boundary eg. for style change */

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

void QTextLayout::setFormat(int from, int length, int format)
{
    if (!d->items)
	d->itemize(QTextEngine::Full);
    d->setFormat(from, length, format);
}

void QTextLayout::setTextFlags(int textFlags)
{
    d->textFlags = textFlags;
    d->lines.clear();
}


void QTextLayout::setPalette(const QPalette &p)
{
    d->pal = new QPalette(p);
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
    d->textFlags = textFlags;
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

    if (!d->items.size()) {
	// ##### use block font
	if (d->fnt) {
	    QFontEngine *e = d->fnt->engineForScript(QFont::Latin);
	    line.ascent = e->ascent();
	    line.descent = e->descent();
	}
	d->lines.append(line);
	return QTextLine(0, d);
    }

    // ########## Readd tab support.
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
    int spacew = 0;
    while (item < d->items.size()) {
	d->shape(item);
	const QScriptItem &current = d->items[item];
	line.ascent = qMax(line.ascent, current.ascent);
	line.descent = qMax(line.descent, current.descent);

	if (current.isObject) {
	    if (line.length && line.textWidth + current.width > line.width && !(d->textFlags & Qt::SingleLine))
		goto found;

	    line.textWidth += current.width;
	    line.length++;

	    ++item;
	    continue;
	}

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
	    int lastNonSpace = next;
	    if (!(d->textFlags & (Qt::SingleLine|Qt::IncludeTrailingSpaces))) {
		while (lastNonSpace >= pos && itemAttrs[lastNonSpace].whiteSpace)
		    --lastNonSpace;
	    }

	    int gp = logClusters[pos];
	    int gs = lastNonSpace < pos ? -1 : logClusters[lastNonSpace];
	    int ge = logClusters[next];

	    int tmpw = 0;
	    while (gp <= gs) {
		tmpw += glyphs[gp].advance;
		++gp;
	    }
	    int nextspacew = 0;
	    while (gp <= ge) {
		nextspacew += glyphs[gp].advance;
		++gp;
	    }
//  	    qDebug("possible break at %d, chars (%d-%d): width %d, spacew=%d, nextspacew=%d",
//  		   current.position + next, pos, next, tmpw, spacew,  nextspacew);

	    if (line.length && line.textWidth + tmpw > line.width && !(d->textFlags & Qt::SingleLine))
		goto found;

	    line.textWidth += tmpw + spacew;
	    spacew = nextspacew;
	    line.length += next - pos + 1;

	    if (d->string[current.position+pos] == QChar_linesep)
		goto found;

	    pos = next + 1;
	} while (pos < length);
	++item;
    }
 found:
//     qDebug("line length = %d, ascent=%d, descent=%d, textWidth=%d", line.length, line.ascent, line.descent, line.textWidth);
//     qDebug("        : '%s'", d->string.mid(line.from, line.length).utf8());

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

QTextLine QTextLayout::findLine(int pos) const
{
    for (int i = 0; i < d->lines.size(); ++i) {
	const QScriptLine& line = d->lines[i];
	if (line.from + line.length > pos)
	    return QTextLine(i, d);
    }
    return QTextLine();
}


QRect QTextLayout::boundingRect() const
{
    QRect r;
    for (int i = 0; i < d->lines.size(); ++i) {
	const QScriptLine &si = d->lines[i];
	r |= QRect(si.x, si.y, si.width, si.ascent + si.descent);
    }
    return r;
}

QRegion QTextLayout::region() const
{
    // ####
    return QRegion();
}

static void drawSelection(QPainter *p, QPalette *pal, QTextLayout::SelectionType type,
			  const QRect &rect, const QTextLine &line, const QPoint &pos)
{
    p->save();
    p->setClipRect(rect, QPainter::CoordPainter);
    QColor bg;
    QColor text;
    switch(type) {
    case QTextLayout::Highlight:
	bg = pal->highlight();
	text = pal->highlightedText();
	break;
    case QTextLayout::ImText:
	int h1, s1, v1, h2, s2, v2;
	pal->color( QPalette::Base ).getHsv( &h1, &s1, &v1 );
	pal->color( QPalette::Background ).getHsv( &h2, &s2, &v2 );
	bg.setHsv( h1, s1, ( v1 + v2 ) / 2 );
	break;
    case QTextLayout::ImSelection:
	bg = pal->text();
	text = pal->background();
	break;
    }
    p->fillRect(rect, bg);
    if (text.isValid())
	p->setPen(text);
    line.draw(p, pos.x(), pos.y());
    p->restore();
    return;
}


void QTextLayout::draw(QPainter *p, const QPoint &pos, int cursorPos, const Selection *selections, int nSelections, const QRect &cr) const
{
    Q_ASSERT(numLines() != 0);

    int clipy = INT_MIN;
    int clipe = INT_MAX;
    if (cr.isValid()) {
	clipy = cr.y() - pos.y();
	clipe = clipy + cr.height();
    }

    for ( int i = 0; i < d->lines.size(); i++ ) {
	QTextLine l(i, d);
	const QScriptLine &sl = d->lines[i];

	if (sl.y > clipe || sl.y + sl.ascent + sl.descent < clipy)
	    continue;

	int from = sl.from;
	int length = sl.length;

	l.draw(p, pos.x(), pos.y());
	if (selections) {
	    for (int j = 0; j < nSelections; ++j) {
		const Selection &s = selections[j];
		if (s.type() != Highlight)
		    continue;
		if (!d->pal)
		    continue;

		if (s.from() + s.length() > from && s.from() < from+length) {
		    QRect highlight = QRect(QPoint(pos.x() + l.cursorToX(qMax(s.from(), from)),
						   pos.y() + sl.y),
					    QPoint(pos.x() + l.cursorToX(qMin(s.from() + s.length(), from+length)) - 1,
						   pos.y() + sl.y + sl.ascent + sl.descent));
		    drawSelection(p, d->pal, (QTextLayout::SelectionType)s.type(), highlight, l, pos);
		}
	    }
	}
	if (sl.from <= cursorPos && sl.from + sl.length >= cursorPos) {
	    int x = l.cursorToX(cursorPos);
	    p->drawLine(pos.x() + x, pos.y() + sl.y, pos.x() + x, pos.y() + sl.y + sl.ascent + sl.descent );
	}
    }

}


QRect QTextLine::rect() const
{
    const QScriptLine& si = eng->lines[i];
    return QRect( si.x, si.y, si.width, si.ascent+si.descent );
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
    // ##########################
}

int QTextLine::from() const
{
    return eng->lines[i].from;
}

int QTextLine::length() const
{
    return eng->lines[i].length;
}

void QTextLine::draw( QPainter *p, int x, int y, int *underlinePositions ) const
{
    const QScriptLine &line = eng->lines[i];

    if (!line.length)
	return;

    int lineEnd = line.from + line.length;
    // don't draw trailing spaces or take them into the layout.
    const QCharAttributes *attributes = eng->attributes();
    while (lineEnd > line.from && attributes[lineEnd-1].whiteSpace)
	--lineEnd;

    int firstItem = eng->findItem(line.from);
    int lastItem = eng->findItem(lineEnd - 1);
    int nItems = lastItem-firstItem+1;

    x += line.x;
    y += line.y + line.ascent;

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

	if (si.isObject && eng->inlineObjectIface && eng->formats) {
	    QTextFormat format = eng->formats->format(si.format);
	    eng->inlineObjectIface->drawItem(p, QPoint(x, y), QTextItem(item, eng), format);
	}

	if ( si.isTab || si.isObject ) {
	    x += si.width;
	    continue;
	}
	unsigned short *logClusters = eng->logClusters(&si);
	QGlyphLayout *glyphs = eng->glyphs(&si);

	int start = qMax(line.from, si.position);
	int gs = logClusters[start-si.position];
	int end;
	int ge;
	if (lineEnd < si.position + eng->length(item)) {
	    end = lineEnd;
	    ge = logClusters[end-si.position];
	} else {
	    end = si.position + eng->length(item);
	    ge = si.num_glyphs;
	}

	QFontEngine *fe = eng->fontEngine(si);
	Q_ASSERT( fe );
	if (eng->formats) {
	    QTextFormat f = eng->formats->format(si.format);
	    Q_ASSERT(f.isCharFormat());
	    QTextCharFormat chf = f.toCharFormat();
	    QColor c = chf.color();
	    p->setPen(c);
	}
	QGlyphFragment gf;
	gf.analysis = si.analysis;
	gf.hasPositioning = si.hasPositioning;
	gf.ascent = si.ascent;
	gf.descent = si.descent;
	gf.num_glyphs = ge - gs + 1;
	gf.glyphs = glyphs + gs;
	gf.font = fe;

	int *ul = underlinePositions;
	if (ul)
	    while (*ul != -1 && *ul < start)
		++ul;
	do {
	    int gtmp = ge;
	    if (ul && *ul != -1 && *ul < end)
		gtmp = logClusters[*ul-si.position];

	    gf.num_glyphs = gtmp - gs;
	    gf.glyphs = glyphs + gs;
	    int w = 0;
	    while (gs < gtmp) {
		w += glyphs[gs].advance;
		++gs;
	    }
	    gf.width = w;
	    p->drawGlyphs(QPoint(x, y), gf);
	    x += w;
	    if (ul && *ul != -1 && *ul < end) {
		// draw underline
		gtmp = (*ul == end-1) ? ge : logClusters[*ul+1-si.position];
		gf.num_glyphs = gtmp - gs;
		gf.glyphs = glyphs + gs;
		w = 0;
		while (gs < gtmp) {
		    w += glyphs[gs].advance;
		    ++gs;
		}
		gf.width = w;
		p->drawGlyphs(QPoint(x, y), gf, Qt::Underline);
		x += w;
		++ul;
	    }
	} while (gs < ge);

    }
}



int QTextLine::cursorToX( int *cPos, Edge edge ) const
{
    if (!i && !eng->items.size()) {
	*cPos = 0;
	return eng->lines[0].x;
    }

    int pos = *cPos;

    int itm = eng->findItem(pos);
    QScriptItem *si = &eng->items[itm];
    pos -= si->position;

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

    // add the items left of the cursor

    const QScriptLine &line = eng->lines[i];
    int lineEnd = line.from + line.length;
    // don't draw trailing spaces or take them into the layout.
    const QCharAttributes *attributes = eng->attributes();
    while (lineEnd > line.from && attributes[lineEnd-1].whiteSpace)
	--lineEnd;

    int firstItem = eng->findItem(line.from);
    int lastItem = eng->findItem(lineEnd - 1);
    int nItems = lastItem-firstItem+1;

    x += line.x;

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
	if (item == itm)
	    break;
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

	while (gs <= ge) {
	    x += glyphs[gs].advance;
	    ++gs;
	}
    }

    *cPos = pos + si->position;
    return x;
}

int QTextLine::xToCursor( int x, CursorPosition cpos ) const
{
    const QScriptLine &line = eng->lines[i];

    if (!line.length)
	return line.from;

    int lineEnd = line.from + line.length;
    // don't draw trailing spaces or take them into the layout.
    const QCharAttributes *attributes = eng->attributes();
    while (lineEnd > line.from && attributes[lineEnd-1].whiteSpace)
	--lineEnd;

    int firstItem = eng->findItem(line.from);
    int lastItem = eng->findItem(lineEnd - 1);
    int nItems = lastItem-firstItem+1;

    x -= line.x;

    // ########## Justification
    if (eng->textFlags & Qt::AlignRight)
	x -= line.width - line.textWidth;
    else if (eng->textFlags & Qt::AlignHCenter)
	x -= (line.width - line.textWidth)/2;

    QStackArray<int> visualOrder(nItems);
    QStackArray<unsigned char> levels(nItems);
    for (int i = 0; i < nItems; ++i)
	levels[i] = eng->items[i+firstItem].analysis.bidiLevel;
    QTextEngine::bidiReorder(nItems, levels, visualOrder);

    int gl_before = 0;
    int gl_after = 0;
    int it_before = 0;
    int it_after = 0;
    int x_before = 0xffff;
    int x_after = 0xffff;


    for (int i = 0; i < nItems; ++i) {
	int item = visualOrder[i]+firstItem;
	QScriptItem &si = eng->items[item];

	if ( si.isTab || si.isObject ) {
	    x -= si.width;
	    continue;
	}
	int start = qMax(line.from, si.position);
	int end = qMin(lineEnd, si.position + eng->length(item));

	unsigned short *logClusters = eng->logClusters(&si);

	int gs = logClusters[start-si.position];
	int ge = logClusters[end-si.position-1];

	QGlyphLayout *glyphs = eng->glyphs(&si);

	while (1) {
	    if (glyphs[gs].attributes.clusterStart) {
		if (x > 0) {
		    gl_before = gs;
		    it_before = item;
		    x_before = x;
		}
		if (x <= 0) {
		    gl_after = gs;
		    it_after = item;
		    x_after = -x;
		    goto end;
		}
	    }
	    if (gs > ge)
		break;
	    x -= glyphs[gs].advance;
	    ++gs;
	}
    }

 end:

    int item;
    int glyph;
    if (cpos == OnCharacters || x_before < x_after) {
	item = it_before;
	glyph = gl_before;
    } else {
	item = it_after;
	glyph = gl_after;
    }

    // find the corresponding cursor position
    const QScriptItem &si = eng->items[item];
    unsigned short *logClusters = eng->logClusters(&si);
    int i;
    for (i = 0; i < eng->length(item); ++i)
	if (logClusters[i] == glyph)
	    break;
    return si.position + i;
}
