#include "qtextlayout.h"

#include <qpainter.h>
#include <qregexp.h>
#include <iostream.h>

//#define BIDI_DEBUG 2


QChar::Direction basicDirection(const QRichTextString &text)
{
    int pos = 0;
    while( pos < text.length() ) {
	switch( text.at(pos).c.direction() )
	{
	case QChar::DirL:
	case QChar::DirLRO:
	case QChar::DirLRE:
	    return QChar::DirL;
	case QChar::DirR:
	case QChar::DirAL:
	case QChar::DirRLO:
	case QChar::DirRLE:
	    return QChar::DirR;
	default:
	    break;
	}
	++pos;
    }
    return QChar::DirL;
}


/* a small helper class used internally to resolve Bidi embedding levels.
   Each line of text caches the embedding level at the start of the line for faster
   relayouting
*/
QBidiContext::QBidiContext(unsigned char l, QChar::Direction e, QBidiContext *p, bool o)
    : level(l) , override(o), dir(e)
{
    if(p) {
	p->ref();
	parent = p;
    }
    count = 0;
}

QBidiContext::~QBidiContext()
{
    if(parent) parent->deref();
}

void QBidiContext::ref() const
{
    count++;
}

void QBidiContext::deref() const
{
    count--;
    if(count <= 0) delete this;
}

// ----------------------------------------------------------

QRichTextString::QRichTextString( const QString &str, QRichTextFormat *f )
{
    data = 0;
    len = maxLen = 0;
    insert(0, str, f);
}

QRichTextString::QRichTextString()
{
    data = 0;
    len = maxLen = 0;
}

void QRichTextString::insert( int index, const QString &s, QRichTextFormat *f )
{
    if( maxLen < len + s.length() )
	setLength(len + s.length());
    if ( index < len ) {
	memmove( data + index + s.length(), data + index,
		 sizeof( Char ) * ( len - index ) );
    }
    for ( int i = 0; i < (int)s.length(); ++i ) {
	data[ (int)index + i ].x = 0;
	data[ (int)index + i ].lineStart = 0;
#if defined(_WS_X11_)
	//### workaround for broken courier fonts on X11
	if ( s[ i ] == QChar( 0x00a0U ) )
	    data[ (int)index + i ].c = ' ';
	else
	    data[ (int)index + i ].c = s[ i ];
#else
	data[ (int)index + i ].c = s[ i ];
#endif
	data[ (int)index + i ].format = f;
    }
    cache.insert( index, s );
    len += s.length();
}

void QRichTextString::truncate( int index )
{
    if(index < len) {
	len = index;
	cache.truncate( index );
    }
}

void QRichTextString::remove( int index, int length )
{
    memmove( data + index, data + index + length,
	     sizeof( Char ) * ( len - index - length ) );
    len -= length;
    cache.remove( index, len );
}

void QRichTextString::setFormat( int index, QRichTextFormat *f, bool useCollection )
{
    if ( useCollection && data[ index ].format )
	data[ index ].format->removeRef();
    data[ index ].format = f;
}

// ====================================================================


/*
  used internally.
  Represents one line of text in a Rich Text drawing area
*/
QTextRow::QTextRow(const QRichTextString &t, int from, int length, QTextRow *previous)
    :  start(from), len(length), text(t), prev(previous), reorderedText()
{
    next = 0;

    endEmbed = 0;
    if ( prev ) {
	bidiStatus = prev->bidiStatus;
	startEmbed = prev->startEmbedding();
    } else {
	if( basicDirection(text) == QChar::DirL )
	    startEmbed = new QBidiContext( 0, QChar::DirL );
	else
	    startEmbed = new QBidiContext( 1, QChar::DirR );
    }
    startEmbed->ref();
    xPos = yPos = w = h = 0;

    hasComplexText();
    bidiReorderLine();
}

QTextRow::~QTextRow()
{
    // ### care about previous/next????

    startEmbed->deref();
    endEmbed->deref();
}

void QTextRow::paint(QPainter &painter, int _x, int _y)
{
    printf("QTextRow::paint\n");
    // no rich text formatting....
    // ### no alignment
#if 0
    painter.drawText(xPos + _x, yPos + _y, reorderedText.toString() );
#else
    QString buffer;
    QRichTextFormat *lastFormat = 0;
    QRichTextString::Char *chr;
    int cw;
    int i = 0;
    int bw = 0;
    int startX = 0;
    int y = yPos + _y;
    for ( ; i < reorderedText.length(); i++ ) {
	chr = &reorderedText.at( i );
	cw = chr->format->width( chr->c );

#if 0
	// check for cursor mark
	if ( cursor && this == cursor->parag() && i == cursor->index() ) {
	    curx = chr->x;
	    curh = h;
	    cury = cy;
	}
#endif	

	// first time - start again...
	if ( !lastFormat ) {
	    lastFormat = chr->format;
	    startX = chr->x;
	    buffer += chr->c;
	    bw = cw;
	    continue;
	}
	
#if 0
	// check if selection state changed
	bool selectionChange = FALSE;
	if ( drawSelections ) {
	    for ( int j = 0; j < doc->numSelections; ++j ) {
		selectionChange = selectionStarts[ j ] == i || selectionEnds[ j ] == i;
		if ( selectionChange )
		    break;
	    }
	}
#endif	
	QColorGroup cg;
	// if something (format, etc.) changed, draw what we have so far
	if ( chr->format != lastFormat || buffer == "\t" || chr->c == '\t' ) { // ### || selectionChange ) {
	    drawBuffer( painter, buffer, startX, y, bw, h, false, //drawSelections,
			     lastFormat, i, 0, 0, cg );
			     //			     lastFormat, i, selectionStarts, selectionEnds, cg );
	    buffer = chr->c;
	    lastFormat = chr->format;
	    startX = chr->x;
	    bw = cw;
	} else {
	    buffer += chr->c;
	    bw += cw;
	}
    }

    if ( !buffer.isEmpty() ) {
#if 0
	bool selectionChange = FALSE;
	if ( drawSelections ) {
	    for ( int j = 0; j < doc->numSelections; ++j ) {
		selectionChange = selectionStarts[ j ] == i || selectionEnds[ j ] == i;
		if ( selectionChange )
		    break;
	    }
	}
#endif
	//	drawParagBuffer( painter, buffer, startX, y, bw, h, drawSelections,
	//		 lastFormat, i, selectionStarts, selectionEnds, cg );
	drawBuffer( painter, buffer, startX, y, bw, h, false, 
			 lastFormat, i, 0, 0, QColorGroup() );
    }

#endif
}

void QTextRow::drawBuffer( QPainter &painter, const QString &buffer, int startX, int y,
			   int bw, int h, bool drawSelections,
			   QRichTextFormat *lastFormat, int i, int *selectionStarts,
			   int *selectionEnds, const QColorGroup &cg )
{	
    painter.setPen( QPen( lastFormat->color() ) );
    painter.setFont( lastFormat->font() );
#if 0
    if ( drawSelections ) {
	for ( int j = 0; j < doc->numSelections; ++j ) {
	    if ( i > selectionStarts[ j ] && i <= selectionEnds[ j ] ) {
		if ( doc->invertSelectionText( j ) )
		    painter.setPen( QPen( cg.color( QColorGroup::HighlightedText ) ) );
		painter.fillRect( startX, y, bw, h, doc->selectionColor( j ) );
	    }
	}
    }
#endif
    printf("painting %s to %d/%d\n", buffer.latin1(), startX, y);
    if ( buffer != "\t" )
	painter.drawText( startX, y /*+ baseLine*/, buffer );
}

	
void QTextRow::setPosition(int _x, int _y)
{
    xPos = _x;
    yPos = _y;
}

void QTextRow::setBoundingRect(const QRect &r)
{
    xPos = r.x();
    yPos = r.y();
    w = r.width();
    h = r.height();
}

QRect QTextRow::boundingRect()
{
    return QRect(xPos, yPos, w, h);
}

bool QTextRow::hasComplexText()
{
    complexText = false;
    if(startEmbed->level) {
	// need to apply BiDi
	complexText = true;
	return true;
    }

    int i = len;
    while(i) {
	if(text.at(i).c.row() > 0x04) {
	    complexText = true;
	    return true;
	}
	--i;
    }
    endEmbed = startEmbed;
    endEmbed->ref();
    return false;
}

struct QBidiRun {
    QBidiRun(int _start, int _stop, QBidiContext *context, QChar::Direction dir) {
	start = _start;
	stop = _stop;
	if(dir == QChar::DirON) dir = context->dir;

	level = context->level;

	// add level of run (cases I1 & I2)
	if( level % 2 ) {
	    if(dir == QChar::DirL || dir == QChar::DirAN)
		level++;
	} else {
	    if( dir == QChar::DirR )
		level++;
	    else if( dir == QChar::DirAN )
		level += 2;
	}
	printf("new run: level = %d\n", level);
    }

    int start;
    int stop;
    // explicit + implicit levels here
    uchar level;
};


// collects one line of the paragraph and transforms it to visual order
void QTextRow::bidiReorderLine()
{
    printf("doing BiDi reordering from %d to %d!\n", start, len+start);

    QList<QBidiRun> runs;
    runs.setAutoDelete(true);

    QBidiContext *context = startEmbed;
    context->ref();

    QBidiStatus status;
    if ( prev )
	status = prev->bidiStatus;
    QChar::Direction dir = QChar::DirON;

    int sor = start;
    int eor = start;

    int current = start;
    while(current < start + len - 1) {
	QChar::Direction dirCurrent;
	if(current == text.length()) {
	    QBidiContext *c = context;
	    while ( c->parent )
		c = c->parent;
	    dirCurrent = c->dir;
	} else
	    dirCurrent = text.at(current).c.direction();

	
#if BIDI_DEBUG > 1
	cout << "directions: dir=" << dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << context->dir << " level =" << (int)context->level << endl;
#endif
	
	switch(dirCurrent) {

	    // embedding and overrides (X1-X9 in the BiDi specs)
	case QChar::DirRLE:
	    {
		unsigned char level = context->level;
		if(level%2) // we have an odd level
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    runs.append( new QBidiRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirR, context);
		    context->ref();
		    status.last = QChar::DirR;
		    status.lastStrong = QChar::DirR;
		}
		break;
	    }
	case QChar::DirLRE:
	    {
		unsigned char level = context->level;
		if(level%2) // we have an odd level
		    level++;
		else
		    level += 2;
		if(level < 61) {
		    runs.append( new QBidiRun(sor, eor, context, dir) );	
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirL, context);
		    context->ref();
		    status.last = QChar::DirL;
		    status.lastStrong = QChar::DirL;
		}
		break;
	    }
	case QChar::DirRLO:
	    {
		unsigned char level = context->level;
		if(level%2) // we have an odd level
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    runs.append( new QBidiRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirR, context, true);
		    context->ref();
		    dir = QChar::DirR;
		    status.last = QChar::DirR;
		    status.lastStrong = QChar::DirR;
		}
		break;
	    }
	case QChar::DirLRO:
	    {
		unsigned char level = context->level;
		if(level%2) // we have an odd level
		    level++;
		else
		    level += 2;
		if(level < 61) {
		    runs.append( new QBidiRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirL, context, true);
		    context->ref();
		    dir = QChar::DirL;
		    status.last = QChar::DirL;
		    status.lastStrong = QChar::DirL;
		}
		break;
	    }
	case QChar::DirPDF:
	    {
		QBidiContext *c = context->parent;
		if(c) {
		    runs.append( new QBidiRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    status.last = context->dir;
		    context->deref();
		    context = c;
		    if(context->override)
			dir = context->dir;
		    else
			dir = QChar::DirON;
		    status.lastStrong = context->dir;
		}		
		break;
	    }
	
	    // strong types
	case QChar::DirL:
	    if(dir == QChar::DirON)
		dir = QChar::DirL;
	    switch(status.last)
		{
		case QChar::DirL:
		    eor = current; status.eor = QChar::DirL; break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirEN:
		case QChar::DirAN:
		    runs.append( new QBidiRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirCS:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if(dir != QChar::DirL) {
			//last stuff takes embedding dir
			if( context->dir == QChar::DirR ) {
			    if(status.eor != QChar::DirR) {
				// AN or EN
				runs.append( new QBidiRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirR;
			    }
			    else
				eor = current - 1;
			    runs.append( new QBidiRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			} else {
			    if(status.eor == QChar::DirR) {
				runs.append( new QBidiRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirL;
			    } else {
				eor = current; status.eor = QChar::DirL; break;
			    }
			}
		    } else {
			eor = current; status.eor = QChar::DirL;
		    }
		default:
		    break;
		}
	    status.lastStrong = QChar::DirL;
	    break;
	case QChar::DirAL:
	case QChar::DirR:
	    if(dir == QChar::DirON) dir = QChar::DirR;
	    switch(status.last)
		{
		case QChar::DirR:
		case QChar::DirAL:
		    eor = current; status.eor = QChar::DirR; break;
		case QChar::DirL:
		case QChar::DirEN:
		case QChar::DirAN:
		    runs.append( new QBidiRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirCS:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if( status.eor != QChar::DirR && status.eor != QChar::DirAL ) {
			//last stuff takes embedding dir
			if(context->dir == QChar::DirR || status.lastStrong == QChar::DirR) {
			    runs.append( new QBidiRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			    eor = current;
			} else {
			    eor = current - 1;
			    runs.append( new QBidiRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			}
		    } else {
			eor = current; status.eor = QChar::DirR;
		    }
		default:
		    break;
		}
	    status.lastStrong = dirCurrent;
	    break;

	    // weak types:

	case QChar::DirNSM:
	    // ### if @sor, set dir to dirSor
	    break;
	case QChar::DirEN:
	    if(status.lastStrong != QChar::DirAL) {
		// if last strong was AL change EN to AL
		if(dir == QChar::DirON) {
		    if(status.lastStrong == QChar::DirL)
			dir = QChar::DirL;
		    else
			dir = QChar::DirAN;
		}
		switch(status.last)
		    {
		    case QChar::DirEN:
		    case QChar::DirL:
		    case QChar::DirET:
			eor = current;
			status.eor = dirCurrent;
			break;
		    case QChar::DirR:
		    case QChar::DirAL:
		    case QChar::DirAN:
			runs.append( new QBidiRun(sor, eor, context, dir) );
			++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			dir = QChar::DirAN; break;
		    case QChar::DirES:
		    case QChar::DirCS:
			if(status.eor == QChar::DirEN) {
			    eor = current; status.eor = QChar::DirEN; break;
			}
		    case QChar::DirBN:
		    case QChar::DirB:
		    case QChar::DirS:
		    case QChar::DirWS:
		    case QChar::DirON:		
			if(status.eor == QChar::DirR) {
			    // neutrals go to R
			    eor = current - 1;
			    runs.append( new QBidiRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirAN;
			}
			else if( status.eor == QChar::DirL ||
				 (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			    eor = current; status.eor = dirCurrent;
			} else {
			    // numbers on both sides, neutrals get right to left direction
			    if(dir != QChar::DirL) {
				runs.append( new QBidiRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				eor = current - 1;
				dir = QChar::DirR;
				runs.append( new QBidiRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirAN;
			    } else {
				eor = current; status.eor = dirCurrent;
			    }
			}
		    default:
			break;
		    }
		break;
	    }
	case QChar::DirAN:
	    dirCurrent = QChar::DirAN;
	    if(dir == QChar::DirON) dir = QChar::DirAN;
	    switch(status.last)
		{
		case QChar::DirL:
		case QChar::DirAN:
		    eor = current; status.eor = QChar::DirAN; break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirEN:
		    runs.append( new QBidiRun(sor, eor, context, dir) );
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirCS:
		    if(status.eor == QChar::DirAN) {
			eor = current; status.eor = QChar::DirR; break;
		    }
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:		
		    if(status.eor == QChar::DirR) {
			// neutrals go to R
			eor = current - 1;
			runs.append( new QBidiRun(sor, eor, context, dir) );
			++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			dir = QChar::DirAN;
		    } else if( status.eor == QChar::DirL ||
			       (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			eor = current; status.eor = dirCurrent;
		    } else {
			// numbers on both sides, neutrals get right to left direction
			if(dir != QChar::DirL) {
			    runs.append( new QBidiRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    eor = current - 1;
			    dir = QChar::DirR;
			    runs.append( new QBidiRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirAN;
			} else {
			    eor = current; status.eor = dirCurrent;
			}
		    }
		default:
		    break;
		}
	    break;
	case QChar::DirES:
	case QChar::DirCS:
	    break;
	case QChar::DirET:
	    if(status.last == QChar::DirEN) {
		dirCurrent = QChar::DirEN;
		eor = current; status.eor = dirCurrent;
		break;
	    }
	    break;

	    // boundary neutrals should be ignored
	case QChar::DirBN:
	    break;
	    // neutrals
	case QChar::DirB:
	    // ### what do we do with newline and paragraph seperators that come to here?
	    break;
	case QChar::DirS:
	    // ### implement rule L1
	    break;
	case QChar::DirWS:
	case QChar::DirON:
	    break;
	default:
	    break;
	}

	//cout << "     after: dir=" << //        dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << context->dir << endl;

	if(current >= text.length()) break;
	
	// set status.last as needed.
	switch(dirCurrent)
	    {
	    case QChar::DirET:
	    case QChar::DirES:
	    case QChar::DirCS:
	    case QChar::DirS:
	    case QChar::DirWS:
	    case QChar::DirON:
		switch(status.last)
		    {
		    case QChar::DirL:
		    case QChar::DirR:
		    case QChar::DirAL:
		    case QChar::DirEN:
		    case QChar::DirAN:
			status.last = dirCurrent;
			break;
		    default:
			status.last = QChar::DirON;
		    }
		break;
	    case QChar::DirNSM:
	    case QChar::DirBN:
		// ignore these
		break;
	    default:
		status.last = dirCurrent;
	    }

	++current;
    }

#ifdef BIDI_DEBUG
    cout << "reached end of paragraph current=" << current << ", eor=" << eor << endl;
#endif
    eor = current;

    runs.append( new QBidiRun(sor, eor, context, dir) );

    if(endEmbed)
	endEmbed->deref();
    endEmbed = context;
    // both commands below together give a noop...
    //endEmbed->ref();
    //context->deref();

    // reorder line according to run structure...

    // first find highest and lowest levels
    uchar levelLow = 128;
    uchar levelHigh = 0;
    QBidiRun *r = runs.first();
    while ( r ) {
	printf("level = %d\n", r->level);
	if ( r->level > levelHigh )
	    levelHigh = r->level;
	if ( r->level < levelLow )
	    levelLow = r->level;
	r = runs.next();
    }

    // implements reordering of the line (L2 according to BiDi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // reversing is only done up to the lowest odd level
    if(!levelLow%2) levelLow++;

#ifdef BIDI_DEBUG
    cout << "reorderLine: lineLow = " << (uint)levelLow << ", lineHigh = " << (uint)levelHigh << endl;
    cout << "logical order is:" << endl;
    QListIterator<QBidiRun> it2(runs);
    QBidiRun *r2;
    for ( ; (r2 = it2.current()); ++it2 )
	cout << "    " << r2 << "  start=" << r2->start << "  stop=" << r2->stop << "  level=" << (uint)r2->level << endl;
#endif

    int count = runs.count() - 1;

    while(levelHigh >= levelLow)
    {
	int i = 0;
	while ( i < count )
	{
	    while(i < count && runs.at(i)->level < levelHigh) i++;
	    int start = i;
	    while(i <= count && runs.at(i)->level >= levelHigh) i++;
	    int end = i-1;

	    if(start != end)
	    {
		//cout << "reversing from " << start << " to " << end << endl;
		for(int j = 0; j < (end-start+1)/2; j++)
		{
		    QBidiRun *first = runs.take(start+j);
		    QBidiRun *last = runs.take(end-j-1);
		    runs.insert(start+j, last);
		    runs.insert(end-j, first);
		}
	    }
	    i++;
	    if(i >= count) break;
	}
	levelHigh--;
    }

#ifdef BIDI_DEBUG
    cout << "visual order is:" << endl;
    QListIterator<QBidiRun> it3(runs);
    QBidiRun *r3;
    for ( ; (r3 = it3.current()); ++it3 )
    {
	cout << "    " << r3 << endl;
    }
#endif

    // now construct the reordered string out of the runs...

    reorderedText.clear();
    r = runs.first();
    int x = 0;
    while ( r ) {
	if(r->level %2) {
	    // odd level, need to reverse the string
	    int pos = r->stop;
	    while(pos >= r->start) {
		QRichTextString::Char c = text.at(pos);
		c.x = x;
		x += c.format->width(c.c);
		reorderedText.append( c );
		pos--;
	    }
	} else {
	    int pos = r->start;
	    while(pos <= r->stop) {
		QRichTextString::Char c = text.at(pos);
		c.x = x;
		x += c.format->width(c.c);
		reorderedText.append( c );
		pos++;
	    }
	}
	r = runs.next();
    }
}

// ========================================================


/*!
  create a rich text drawing area
*/
QTextArea::QTextArea()
{
    // ###
    width = 300;
    paragraphs.setAutoDelete(true);
}

QTextArea::~QTextArea()
{
}

QTextArea::QTextArea(int w)
    : width(w)
{
    paragraphs.setAutoDelete(true);
}

/*!
  append a new paragraph of text to the Textarea
*/
void QTextArea::appendParagraph(const QRichTextString &str)
{
    paragraphs.append( createParagraph(str, paragraphs.last()) );
}

/*!
  insert a new paragraph at position pos to the Textarea
*/
void QTextArea::insertParagraph(const QRichTextString &str, int pos)
{
    paragraphs.insert( pos, createParagraph( str, paragraphs.at(pos) ) );

    // ### relayout the following paragraphs
}

/*!
  remove the paragraph at position pos from the textarea.
*/
void QTextArea::removeParagraph(int pos)
{
    //paragraphs.remove(pos);
}

QParagraph *QTextArea::createParagraph(const QRichTextString &text, QParagraph *before)
{
    return new QParagraph(text, this, before);
}

QRect QTextArea::lineRect(int x, int y, int h) const
{
    return QRect(x, y, width, 10000);
}

void QTextArea::paint(QPainter &p, int x, int y)
{
    printf("QTextarea::paint\n");
    QListIterator<QParagraph> it(paragraphs);
    while(it.current()) {
	(*it)->paint(p, x, y);
	++it;
    }
}



// --------------------------------------------------------



QParagraph::QParagraph(const QRichTextString &t, QTextArea *a, QParagraph *lastPar)
    : text(t)
{
    area = a;
    first = last = 0;

    //    text.compose();

    // get last paragraph so we know where we want to place the next line
    if ( lastPar ) {
	QPoint p = lastPar->nextLine();
	xPos = p.x();
	yPos = p.y();
    } else {
	xPos = yPos = 0;
    }

    layout();
}

QParagraph::~QParagraph()
{
}

QPoint QParagraph::nextLine() const
{
    if( !last )
	return QPoint(0, 0);
    return QPoint( last->x(), last->y() + last->height() );
}

void QParagraph::layout()
{
    int pos = 0;
    int lineLength = 0;
#if 1
    QRichTextFormatterBreakWords formatter(area);
    formatter.format(this, 0);
#else
    while( (lineLength = findLineBreak(pos)) != 0 ) {
	   addLine(pos, lineLength);
	   pos += lineLength;
    }
#endif
}


int QParagraph::findLineBreak(int pos)
{
    printf("findLineBreak start=%d, text.length=%d\n", pos, text.length());
    int start = pos;
    QFontMetrics fm(QApplication::font());


    int x = xPos;
    int y = yPos;
    if ( last ) {
	x += last->x();
	y += last->y() + last->height();
    }
    printf("new line at %d/%d\n", x, y);
    QRect lineRect = area->lineRect(x, y);
    int width = lineRect.width();
    int pos2 = pos;

    while(1) {
	while(pos2 < text.length() && !(text.at(pos2).c.isSpace())) {
	    pos2++;
	}
	
	// we know the string is not going to get modified....
	width -= fm.width(QConstString(const_cast<QChar *>(text.toString().unicode() + pos), pos2 - pos).string());
	if(width < 0) return pos - start;
	if(pos2 < text.length()) {
	    width -= fm.width(' ');
	    pos2++;	
	    pos = pos2;
	} else {
	    return pos2 - start;
	}
    }
}


void QParagraph::addLine(int start, int length)
{
    printf("addline %d %d\n", start, length);
    QTextRow *line = new QTextRow(text, start, length, last);

    QFontMetrics fm(QApplication::font());
    int height = fm.height();

    int x = xPos;
    int y = yPos;
    if ( last ) {
	x += last->x();
	y += last->y() + last->height();
    }
    QRect r = area->lineRect(x, y, height);
    bRect |= r;
    // make is relative to the paragraph
    r.moveBy( - xPos, -yPos );

    line->setBoundingRect(r);

    if( !first )
	first = line;
    if ( last )
	last->setNextLine(line);
    last = line;
}

void QParagraph::paint(QPainter &p, int x, int y)
{
    printf("QParagraph::paint\n");
    // #### add a check if we need to paint at all!!!

    x += xPos;
    y += yPos;
    QTextRow *line = first;
    while(line) {
	line->paint(p, x, y);
	line = line->nextLine();
    }
}

// ======================================================================


QRichTextFormatCollection::QRichTextFormatCollection()
{
    defFormat = new QRichTextFormat( QApplication::font(),
				     QApplication::palette().color( QPalette::Normal, QColorGroup::Text ) );
    lastFormat = cres = 0;
    cflags = -1;
    cKey.setAutoDelete( TRUE );
    cachedFormat = 0;
}

QRichTextFormat *QRichTextFormatCollection::format( QRichTextFormat *f )
{
    if ( f->parent() == this ) {
#ifdef DEBUG_COLLECTION
	qDebug( "need '%s', best case!", f->key().latin1() );
#endif
	lastFormat = f;
	lastFormat->addRef();
	return lastFormat;
    }

    if ( f == lastFormat || ( lastFormat && f->key() == lastFormat->key() ) ) {
#ifdef DEBUG_COLLECTION
	qDebug( "need '%s', good case!", f->key().latin1() );
#endif
	lastFormat->addRef();
	return lastFormat;
    }

    QRichTextFormat *fm = cKey.find( f->key() );
    if ( fm ) {
#ifdef DEBUG_COLLECTION
	qDebug( "need '%s', normal case!", f->key().latin1() );
#endif
	lastFormat = fm;
	lastFormat->addRef();
	return lastFormat;
    }

#ifdef DEBUG_COLLECTION
    qDebug( "need '%s', worst case!", f->key().latin1() );
#endif
    lastFormat = new QRichTextFormat( *f );
    lastFormat->collection = this;
    cKey.insert( lastFormat->key(), lastFormat );
    return lastFormat;
}

QRichTextFormat *QRichTextFormatCollection::format( QRichTextFormat *of, QRichTextFormat *nf, int flags )
{
    if ( cres && kof == of->key() && knf == nf->key() && cflags == flags ) {
#ifdef DEBUG_COLLECTION
	qDebug( "mix of '%s' and '%s, best case!", of->key().latin1(), nf->key().latin1() );
#endif
	cres->addRef();
	return cres;
    }

    cres = new QRichTextFormat( *of );
    kof = of->key();
    knf = nf->key();
    cflags = flags;
    if ( flags & QRichTextFormat::Bold )
	cres->fn.setBold( nf->fn.bold() );
    if ( flags & QRichTextFormat::Italic )
	cres->fn.setItalic( nf->fn.italic() );
    if ( flags & QRichTextFormat::Underline )
	cres->fn.setUnderline( nf->fn.underline() );
    if ( flags & QRichTextFormat::Family )
	cres->fn.setFamily( nf->fn.family() );
    if ( flags & QRichTextFormat::Size )
	cres->fn.setPointSize( nf->fn.pointSize() );
    if ( flags & QRichTextFormat::Color )
	cres->col = nf->col;
    cres->update();

    QRichTextFormat *fm = cKey.find( cres->key() );
    if ( !fm ) {
#ifdef DEBUG_COLLECTION
	qDebug( "mix of '%s' and '%s, worst case!", of->key().latin1(), nf->key().latin1() );
#endif
	cres->collection = this;
	cKey.insert( cres->key(), cres );
    } else {
#ifdef DEBUG_COLLECTION
	qDebug( "mix of '%s' and '%s, good case!", of->key().latin1(), nf->key().latin1() );
#endif
	delete cres;
	cres = fm;
	cres->addRef();
    }
					
    return cres;
}

QRichTextFormat *QRichTextFormatCollection::format( const QFont &f, const QColor &c )
{
    if ( cachedFormat && cfont == f && ccol == c ) {
#ifdef DEBUG_COLLECTION
	qDebug( "format of font and col '%s' - best case", cachedFormat->key().latin1() );
#endif
	cachedFormat->addRef();
	return cachedFormat;
    }

    QString key = QRichTextFormat::getKey( f, c );
    cachedFormat = cKey.find( key );
    cfont = f;
    ccol = c;

    if ( cachedFormat ) {
#ifdef DEBUG_COLLECTION
	qDebug( "format of font and col '%s' - good case", cachedFormat->key().latin1() );
#endif
	cachedFormat->addRef();
	return cachedFormat;
    }

    cachedFormat = new QRichTextFormat( f, c );
    cachedFormat->collection = this;
    cKey.insert( cachedFormat->key(), cachedFormat );
#ifdef DEBUG_COLLECTION
    qDebug( "format of font and col '%s' - worst case", cachedFormat->key().latin1() );
#endif
    return cachedFormat;
}

void QRichTextFormatCollection::remove( QRichTextFormat *f )
{
    if ( lastFormat == f )
	lastFormat = 0;
    if ( cres == f )
	cres = 0;
    if ( cachedFormat == f )
	cachedFormat = 0;
    cKey.remove( f->key() );
}

void QRichTextFormatCollection::debug()
{
#ifdef DEBUG_COLLECTION
    qDebug( "------------ QRichTextFormatCollection: debug --------------- BEGIN" );
    QDictIterator<QRichTextFormat> it( cKey );
    for ( ; it.current(); ++it ) {
	qDebug( "format '%s' (%p): refcount: %d", it.current()->key().latin1(),
		it.current(), it.current()->ref );
    }
    qDebug( "------------ QRichTextFormatCollection: debug --------------- END" );
#endif
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void QRichTextFormat::setBold( bool b )
{
    if ( b == fn.bold() )
	return;
    fn.setBold( b );
    update();
}

void QRichTextFormat::setItalic( bool b )
{
    if ( b == fn.italic() )
	return;
    fn.setItalic( b );
    update();
}

void QRichTextFormat::setUnderline( bool b )
{
    if ( b == fn.underline() )
	return;
    fn.setUnderline( b );
    update();
}

void QRichTextFormat::setFamily( const QString &f )
{
    if ( f == fn.family() )
	return;
    fn.setFamily( f );
    update();
}

void QRichTextFormat::setPointSize( int s )
{
    if ( s == fn.pointSize() )
	return;
    fn.setPointSize( s );
    update();
}

void QRichTextFormat::setFont( const QFont &f )
{
    if ( f == fn )
	return;
    fn = f;
    update();
}

void QRichTextFormat::setColor( const QColor &c )
{
    if ( c == col )
	return;
    col = c;
}

static int makeLogicFontSize( int s )
{
    int defSize = QApplication::font().pointSize();
    if ( s < defSize - 4 )
	return 1;
    if ( s < defSize )
	return 2;
    if ( s < defSize + 4 )
	return 3;
    if ( s < defSize + 8 )
	return 4;
    if ( s < defSize + 12 )
	return 5;
    if (s < defSize + 16 )
	return 6;
    return 7;
}

static QRichTextFormat *defaultFormat = 0;

QString QRichTextFormat::makeFormatChangeTags( QRichTextFormat *f ) const
{
    if ( !defaultFormat )
	defaultFormat = new QRichTextFormat( QApplication::font(),
					     QApplication::palette().color( QPalette::Normal, QColorGroup::Text ) );

    QString tag;
    if ( f ) {
	if ( f->font() != defaultFormat->font() ||
	     f->color().rgb() != defaultFormat->color().rgb() )
	    tag += "</font>";
	if ( f->font() != defaultFormat->font() ) {
	    if ( f->font().underline() && f->font().underline() != defaultFormat->font().underline() )
		tag += "</u>";
	    if ( f->font().italic() && f->font().italic() != defaultFormat->font().italic() )
		tag += "</i>";
	    if ( f->font().bold() && f->font().bold() != defaultFormat->font().bold() )
		tag += "</b>";
	}
    }

    if ( font() != defaultFormat->font() ) {
	if ( font().bold() && font().bold() != defaultFormat->font().bold() )
	    tag += "<b>";
	if ( font().italic() && font().italic() != defaultFormat->font().italic() )
	    tag += "<i>";
	if ( font().underline() && font().underline() != defaultFormat->font().underline() )
	    tag += "<u>";
    }
    if ( font() != defaultFormat->font() ||
	 color().rgb() != defaultFormat->color().rgb() ) {
	tag += "<font ";
	if ( font().family() != defaultFormat->font().family() )
	    tag +="face=\"" + fn.family() + "\" ";
	if ( font().pointSize() != defaultFormat->font().pointSize() )
	    tag +="size=\"" + QString::number( makeLogicFontSize( fn.pointSize() ) ) + "\" ";
	if ( color().rgb() != defaultFormat->color().rgb() )
	    tag +="color=\"" + col.name() + "\" ";
	tag += ">";
    }

    return tag;
}

QString QRichTextFormat::makeFormatEndTags() const
{
    if ( !defaultFormat )
	defaultFormat = new QRichTextFormat( QApplication::font(),
					     QApplication::palette().color( QPalette::Normal, QColorGroup::Text ) );

    QString tag;
    if ( font() != defaultFormat->font() ||
	 color().rgb() != defaultFormat->color().rgb() )
	tag += "</font>";
    if ( font() != defaultFormat->font() ) {
	if ( font().underline() && font().underline() != defaultFormat->font().underline() )
	    tag += "</u>";
	if ( font().italic() && font().italic() != defaultFormat->font().italic() )
	    tag += "</i>";
	if ( font().bold() && font().bold() != defaultFormat->font().bold() )
	    tag += "</b>";
    }
    return tag;
}

// ============================================================================

QRichTextFormatter::QRichTextFormatter( QTextArea *a )
    : area( a )
{
}

void QRichTextFormatter::addLine(QParagraph *p, int from, int to, int height)
{
    printf("addline %d %d\n", from, to - from);
    QTextRow *line = new QTextRow(*p->string(), from, to - from, p->lastRow());

    int x = p->x();
    int y = p->y();
    if ( p->lastRow() ) {
	x += p->lastRow()->x();
	y += p->lastRow()->y() + p->lastRow()->height();
    }
    QRect r = area->lineRect(x, y, height);
    // ####
    //bRect |= r;
    // make is relative to the paragraph
    r.moveBy( - p->x(), -p->y() );

    line->setBoundingRect(r);

    if( !p->firstRow() )
	p->setFirstRow(line);
    if ( p->lastRow() )
	p->lastRow()->setNextLine(line);
    p->setLastRow( line );

}

// ============================================================================

QRichTextFormatterBreakWords::QRichTextFormatterBreakWords( QTextArea *a )
    : QRichTextFormatter( a )
{
}


int QRichTextFormatterBreakWords::format( QParagraph *parag, int start )
{
    QRichTextString::Char *c = 0;

    // #########################################
    // Should be optimized so that we start formatting
    // really at start (this means the last line begin before start)
    // and not always at the beginnin of the parag!
    start = 0;
    if ( start == 0 ) {
	c = &parag->string()->at( 0 );
    }
    // #########################################

    int i = start;
    int lastSpace = -1;
    int tmpBaseLine = 0, tmph = 0;

    QRect lineRect = area->lineRect(parag->x() ,parag->y());
    int w = lineRect.width();
    printf("new line at %d/%d\n", parag->x(), parag->y());
    int h = 0;
    int x = 0;

    for ( ; i < parag->string()->length(); ++i ) {

	
	c = &parag->string()->at( i );
#if 0
	// ####
	if ( i > 0 && x > left ) {
	    c->lineStart = 0;
	} else {
	    c->lineStart = 1;
	}
#endif
	int ww = 0;
	if ( c->c.unicode() >= 32 || c->c == '\t' ) {
	    ww = c->format->width( c->c );
	} else {
	    ww = c->format->width( ' ' );
	}
	
	if ( x + ww > w ) {
	    if ( lastSpace != -1 )
		i = lastSpace;
	    // ### add baseline
	    addLine(parag, start, i, h);
	    start = i;
	    int xPos = parag->x();
	    int yPos = parag->y();
	    if ( parag->lastRow() ) {
		xPos += parag->lastRow()->x();
		yPos += parag->lastRow()->y() + parag->lastRow()->height();	
	    }
	    lineRect = area->lineRect(xPos, yPos);
	    int w = lineRect.width();
	    printf("new line at %d/%d\n", xPos, yPos);
	    int h = 0;
	    x  = 0;
	    lastSpace = -1;
	    continue;
	} else if ( c->c == ' ' ) {
	    tmpBaseLine = QMAX( tmpBaseLine, c->format->ascent() );
	    tmph = QMAX( tmph, c->format->height() );
	    h = QMAX( h, tmph );
	    lastSpace = i;
	    // ### cache lineheight and baseline for the chars up to here!!!
	} else {
	    tmpBaseLine = QMAX( tmpBaseLine, c->format->ascent() );
	    tmph = QMAX( tmph, c->format->height() );
	}
	
	c->x = x;
	x += ww;
    }

#if 0
    // ############## unefficient!!!!!!!!!!!!!!!!!!!!!! - rewrite that!!!!
    if ( parag->alignment() & Qt::AlignHCenter || parag->alignment() & Qt::AlignRight ) {
	int last = 0;
	QMap<int, QRichTextParag::LineStart*>::Iterator it = parag->lineStartList().begin();
	while ( TRUE ) {
	    it++;
	    int i = 0;
	    if ( it == parag->lineStartList().end() )
		i = parag->length() - 1;
	    else
		i = it.key() - 1;
	    c = &parag->string()->at( i );
	    int lw = c->x + c->format->width( c->c );
	    int diff = w - lw;
	    if ( parag->alignment() & Qt::AlignHCenter )
		diff /= 2;
	    for ( int j = last; j <= i; ++j )
		parag->string()->at( j ).x += diff;
	    last = i + 1;
	    if ( it == parag->lineStartList().end() )
		break;
	}
    }

    y += h + doc->paragSpacing( parag );
    return y;
#endif

    return 0;
}
