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
#if defined(Q_WS_X11)
	//### workaround for broken courier fonts on X11
	if ( s[ i ] == QChar( 0x00a0U ) )
	    data[ (int)index + i ].c = ' ';
	else
	    data[ (int)index + i ].c = s[ i ];
#else
	data[ (int)index + i ].c = s[ i ];
#endif
	data[ (int)index + i ].setFormat( f );
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

void QRichTextString::setFormat( int index, QRichTextFormat *f, bool /*useCollection*/ )
{
    data[ index ].setFormat( f );
}

QRichTextString::Char::~Char()
{
    if ( f )
	f->removeRef();
}

void QRichTextString::Char::setFormat(QRichTextFormat *fmt)
{
    if ( f )
	f->removeRef();
    f = fmt;
    if ( f )
	f->addRef();
}

QRichTextString::QRichTextString(const QRichTextString &other)
{
    cache = other.cache;
    len = other.len;
    maxLen = len;
    data = new Char[len];
    memcpy(data, other.data, len*sizeof(Char));
}

QRichTextString &QRichTextString::operator = (const QRichTextString &other)
{
    delete [] data;
    cache = other.cache;
    len = other.len;
    maxLen = len;
    data = new Char[len];
    memcpy(data, other.data, len*sizeof(Char));
    return *this;
}

QRichTextString::Char &QRichTextString::at( int i ) const
{
    return data[ i ];
}

QString QRichTextString::toString() const
{
    return cache;
}

int QRichTextString::length() const
{
    return len;
}

void QRichTextString::append(Char c)
{
    if ( len + 1 >= maxLen ) {
	if ( maxLen < 4 )
	    maxLen = 4;
	setLength(2*maxLen);
    }
    data[len] = c;
    cache += c.c;
    len++;
}

inline void QRichTextString::setLength(int newLen)
{
    Char *newData = new Char[newLen];
    memcpy(newData, data, len*sizeof(Char));
    delete [] data;
    data = newData;
    maxLen = newLen;
}


void QRichTextString::clear()
{
    delete [] data;
    data = 0;
    len = maxLen = 0;
}


// ====================================================================


/*
  used internally.
  Represents one line of text in a Rich Text drawing area
*/
QTextRow::QTextRow(QRichTextString *t, int from, int length, QTextRow *previous, int base, int w)
    :  start(from), len(length), text(t), reorderedText(), p(previous)
{
    bl = base;
    tw = w;
    n = 0;
    startEmbed = endEmbed = 0;

    layout();
}

QTextRow::QTextRow( QRichTextString *t, QTextRow *prev )
{
    text = t;
    p = prev;
    startEmbed = 0;
    endEmbed = 0;
    n = 0;
    start = 0;
    len = 0;
    tw = 0;
}

QTextRow::~QTextRow()
{
    // ### care about previous/next????

    if(startEmbed) startEmbed->deref();
    if(endEmbed) endEmbed->deref();
}

void QTextRow::layout()
{
    if(endEmbed) endEmbed->deref();
    endEmbed = 0;
    if(startEmbed) startEmbed->deref();
    if ( p ) {
	bidiStatus = p->bidiStatus;
	startEmbed = p->endEmbedding();
    } else {
	if( basicDirection(*text) == QChar::DirL )
	    startEmbed = new QBidiContext( 0, QChar::DirL );
	else
	    startEmbed = new QBidiContext( 1, QChar::DirR );
    }
    startEmbed->ref();
    checkComplexText();
    // ### only reorder when needed
    bidiReorderLine();
}



void QTextRow::paint(QPainter &painter, int _x, int _y, QTextAreaCursor *cursor, HAlignment hAlign)
{
    //        printf("QTextRow::paint reorderedText.length() = %d\n", reorderedText.length());

    QRichTextString::Char *chr;
    int cw;
    int i = 0;
    int bw = 0;

    switch(hAlign) {
    case AlignAuto:
    case AlignLeft:
    case AlignJustify:
	break;
    case AlignRight:
	_x += bRect.width() - tw;
    }

    QRichTextFormat *lastFormat = 0;
    int startX = 0;
    QString buffer;
    bw = cw;

    int curx = -1;
    int curh;

    for ( i = 0 ; i < reorderedText.length(); i++ ) {
	chr = &reorderedText.at( i );
	cw = chr->format()->width( chr->c );

	if(!lastFormat) {
	    lastFormat = chr->format();
	    //int startX = chr->x;
	    QString buffer = chr->c;
	    bw = cw;
	}
	
	// check for cursor mark
	if ( cursor && this == cursor->row() && i == cursor->visualIndex() ) {
	    curx = chr->x;
	    if(cursor->paragraph()->basicDirection() == QChar::DirR)
		curx += chr->format()->width(chr->c);
	    curh = chr->format()->height();
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
	if ( chr->format() != lastFormat || buffer == "\t" || chr->c == '\t' ) { // ### || selectionChange ) {
	    drawBuffer( painter, _x, _y, buffer, startX, bw, false, //drawSelections,
			     lastFormat, i, 0, 0, cg );
			     //			     lastFormat, i, selectionStarts, selectionEnds, cg );
	    buffer = chr->c;
	    lastFormat = chr->format();
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
	drawBuffer( painter, _x, _y, buffer, startX, bw, false,
			 lastFormat, i, 0, 0, QColorGroup() );
    }

    // if we should draw a cursor, draw it now
    if ( curx != -1 && cursor ) {
	printf("painting cursor at %d/%d\n", curx, _y);
	painter.fillRect( QRect( _x + bRect.x() + curx, _y + bRect.y(), 1, curh ), Qt::black );
    }
}

void QTextRow::drawBuffer( QPainter &painter, int x, int y, const QString &buffer, int startX,
			   int bw, bool drawSelections,
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
    //    printf("painting %s to %d/%d\n", buffer.latin1(), x + startX + bRect.x(), bRect.y() );
    if ( buffer != "\t" )
	painter.drawText( x + startX + bRect.x(), y + bRect.y() + bl, buffer );
}

	
void QTextRow::setPosition(int _x, int _y)
{
    bRect.moveTopLeft( QPoint(_x, _y) );
}

void QTextRow::setBoundingRect(const QRect &r)
{
    bRect = r;
}

QRect QTextRow::boundingRect()
{
    return bRect;
}

bool QTextRow::checkComplexText()
{
    complexText = false;
    if(startEmbed->level) {
	// need to apply BiDi
	complexText = true;
	return true;
    }

    int i = len;
    while(i) {
	if(text->at(i).c.row() > 0x04) {
	    complexText = true;
	    return true;
	}
	--i;
    }
    endEmbed = startEmbed;
    endEmbed->ref();
    return false;
}

int QTextRow::visualPosition(int logicalPosition) const
{
    int pos = ((QTextRow *)this)->bidiReorderLine(logicalPosition);
    printf("visualPosition(%d) = %d\n", logicalPosition, pos );
    return pos;
}

int QTextRow::logicalPosition(int visualPosition) const
{
    int pos = ((QTextRow *)this)->bidiReorderLine(visualPosition, FALSE);
    printf("logicalPosition(%d) = %d\n", visualPosition, pos );
    return pos;
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
int QTextRow::bidiReorderLine(int posToCheck = -1, bool logicalToVisual)
{
    //printf("doing BiDi reordering from %d to %d!\n", start, len+start);

    QPtrList<QBidiRun> runs;
    runs.setAutoDelete(true);

    QBidiContext *context = startEmbed;
    context->ref();

    QBidiStatus status;
    if ( p )
	status = p->bidiStatus;
    QChar::Direction dir = QChar::DirON;

    int sor = start;
    int eor = start;

    int current = start;
    while(current < start + len - 1) {
	QChar::Direction dirCurrent;
	if(current == text->length()) {
	    QBidiContext *c = context;
	    while ( c->parent )
		c = c->parent;
	    dirCurrent = c->dir;
	} else
	    dirCurrent = text->at(current).c.direction();

	
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
	    // ### what do we do with newline and paragraph separators that come to here?
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

	if(current >= text->length()) break;
	
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
    // #### segfaults without this call. There must be a hole in the refounting somewhere!!!
    endEmbed->ref();
    // both commands below together give a noop...
    //endEmbed->ref();
    //context->deref();

    // reorder line according to run structure...

    // first find highest and lowest levels
    uchar levelLow = 128;
    uchar levelHigh = 0;
    QBidiRun *r = runs.first();
    while ( r ) {
	//printf("level = %d\n", r->level);
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
    QPtrListIterator<QBidiRun> it2(runs);
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
    QPtrListIterator<QBidiRun> it3(runs);
    QBidiRun *r3;
    for ( ; (r3 = it3.current()); ++it3 )
    {
	cout << "    " << r3 << endl;
    }
#endif

    if(posToCheck != -1) {
	// just want to have logical<-->visual mapping
	r = runs.first();
	if(logicalToVisual) {
	    posToCheck += from();
	    int visual = 0;
	    while(r) {
		if(r->start <= posToCheck && r->stop >= posToCheck) {
		    if( r->level%2 ) // odd level
			visual += r->stop - posToCheck;
		    else
			visual += posToCheck - r->start;
		    return visual;
		}
		visual += r->stop - r->start + 1;
		r = runs.next();
	    }
	} else {
	    while(r) {
		if(r->stop - r->start + 1 >= posToCheck) {
		    if( r->level%2 )
			return r->stop - from() - posToCheck;
		    else
			return r->start - from() + posToCheck;
		}
		posToCheck -= r->stop - r->start +1;
		r = runs.next();
	    }
	}
	return -1;
    }
	
    // now construct the reordered string out of the runs...

    reorderedText.clear();
    r = runs.first();
    int x = 0;
    while ( r ) {
	if(r->level %2) {
	    // odd level, need to reverse the string
	    int pos = r->stop;
	    while(pos >= r->start) {
		QRichTextString::Char c = text->at(pos);
		c.x = x;
		x += c.format()->width(c.c);
		reorderedText.append( c );
		pos--;
	    }
	} else {
	    int pos = r->start;
	    while(pos <= r->stop) {
		QRichTextString::Char c = text->at(pos);
		c.x = x;
		x += c.format()->width(c.c);
		reorderedText.append( c );
		pos++;
	    }
	}
	r = runs.next();
    }
    return -1;
}

// ========================================================


/*!
  create a rich text drawing area
*/
QTextArea::QTextArea()
{
    // ###
    width = 300;
    first = last = 0;
}

QTextArea::~QTextArea()
{
    QParagraph *p = first;
    while( p ) {
	QParagraph *n = p->next();
	delete p;
	p = n;
    }
}

QTextArea::QTextArea(int w)
    : width(w)
{
}

/*!
  append a new paragraph of text to the Textarea
*/
void QTextArea::appendParagraph(const QRichTextString &str)
{
    QParagraph *p = createParagraph( str, last );
    if(!first)
	first = last = p;	
    else if(last) last->setNext(p);
    last = p;
}

/*!
  insert a new paragraph at position pos to the Textarea
*/
void QTextArea::insertParagraph(const QRichTextString &str, int pos)
{
#if 0
    // ### relayout the following paragraphs
#endif
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

void QTextArea::paint(QPainter &p, int x, int y, QTextAreaCursor *c)
{
    //printf("QTextarea::paint\n");
    QParagraph *par = first;
    while ( par ) {
	par->paint(p, x, y, c);
	par = par->next();
    }
}

QParagraph *QTextArea::firstParagraph() const
{
    return first;
}

QParagraph *QTextArea::lastParagraph() const
{
    return last;
}


// ==============================================================



QTextAreaCursor::QTextAreaCursor( QTextArea *a )
    : area( a )
{
    parag = area->firstParagraph();
    line = parag->first();
    idx = 0;
    tmpIndex = -1;
    visual1 = visual2 = line->visualPosition(idx);
}

void QTextAreaCursor::insert( const QString &s, bool checkNewLine )
{
    tmpIndex = -1;
    bool justInsert = TRUE;
    if ( checkNewLine )
	justInsert = ( s.find( '\n' ) == -1 );
    if ( justInsert ) {
	idx = parag->insert( idx + line->from(), s );
	idx -= line->from();
	while( idx > line->length() ) {
	    line = line->next();
	    idx -= line->length();
	}
    } else {
#if 0
	QStringList lst = QStringList::split( '\n', s, TRUE );
	QStringList::Iterator it = lst.begin();
	int y = parag->rect().y() + parag->rect().height();
	for ( ; it != lst.end(); ++it ) {
	    if ( it != lst.begin() ) {
		splitAndInsertEmtyParag( FALSE, FALSE );
		parag->setEndState( -1 );
		parag->prev()->format( -1, FALSE );
	    }
	    QString s = *it;
	    if ( s.isEmpty() )
		continue;
	    parag->insert( idx, s );
	    idx += s.length();
	}
	parag->format( -1, FALSE );
	int dy = parag->rect().y() + parag->rect().height() - y;
	QTextEditParag *p = parag->next();
	while ( p ) {
	    p->setParagId( p->prev()->paragId() + 1 );
	    p->move( dy );
	    p->invalidate( 0 );
	    p->setEndState( -1 );
	    p = p->next();
	}
#endif
    }
    visual1 = visual2 = line->visualPosition(idx);
}

void QTextAreaCursor::gotoLeft()
{
    tmpIndex = -1;
    if ( visual1 > 0 ) {
	    visual1--;
    } else if(parag->basicDirection() == QChar::DirL) {
	if ( line->prev() ) {
	    line = line->prev();
	    visual1 = line->length() - 1;
	} else if ( parag->prev() ) {
	    parag = parag->prev();
	    line = parag->last();
	    visual1 = line->length() - 1;
	}
    } else {
	if ( line->next() ) {
	    line = line->next();
	    visual1 = line->length() - 1;
	} else if ( parag->next() ) {
	    parag = parag->next();
	    line = parag->first();
	    visual1 = line->length() - 1;
	}
    }
    idx = line->logicalPosition(visual1);
}

void QTextAreaCursor::gotoRight()
{
    tmpIndex = -1;
    if ( visual1 < line->length() - 1 ) {
	visual1++;
    } else if ( parag->basicDirection() == QChar::DirL ) {
	if ( line->next() ) {
	    line = line->next();
	    visual1 = 0;
	} else if ( parag->next() ) {
	    parag = parag->next();
	    line = parag->first();
	    visual1 = 0;
	}
    } else {
	if ( line->prev() ) {
	    line = line->prev();
	    visual1 = 0;
	} else if ( parag->prev() ) {
	    parag = parag->prev();
	    line = parag->last();
	    visual1 = 0;
	}
    }
    idx = line->logicalPosition(visual1);
}

void QTextAreaCursor::gotoUp()
{
    if ( tmpIndex == -1 )
	tmpIndex = idx;
    if ( line->prev() ) {
	line = line->prev();
    } else if ( parag->prev() ) {
	parag = parag->prev();
	line = parag->last();
    } else
	return;
    idx = QMIN(line->length()-1, tmpIndex);
    visual1 = line->visualPosition(idx);
}

void QTextAreaCursor::gotoDown()
{
    if ( tmpIndex == -1 )
	tmpIndex = idx;
    if ( line->next() ) {
	line = line->next();
    } else if ( parag->next() ) {
	parag = parag->next();
	line = parag->first();
    } else
	return;
    idx = QMIN(line->length()-1, tmpIndex);
    visual1 = line->visualPosition(idx);
}

void QTextAreaCursor::gotoLineEnd()
{
    idx = line->length() - 1;
    visual1 = line->visualPosition(idx);
}

void QTextAreaCursor::gotoLineStart()
{
    idx = 0;
    visual1 = line->visualPosition(idx);
}

void QTextAreaCursor::gotoHome()
{
    tmpIndex = -1;
    parag = area->firstParagraph();
    idx = 0;
    visual1 = line->visualPosition(idx);
}

void QTextAreaCursor::gotoEnd()
{
    tmpIndex = -1;
    parag = area->lastParagraph();
    line = parag->last();
    idx = line->length() - 1;
    visual1 = line->visualPosition(idx);
}

void QTextAreaCursor::gotoPageUp()
{
#if 0
    tmpIndex = -1;
    QTextEditParag *s = parag;
    int h = view->visibleHeight();
    int y = s->rect().y();
    while ( s ) {
	if ( y - s->rect().y() >= h )
	    break;
	s = s->prev();
    }

    if ( !s )
	s = doc->firstParag();

    parag = s;
    idx = 0;
#endif
}

void QTextAreaCursor::gotoPageDown()
{
#if 0
    tmpIndex = -1;
    QTextEditParag *s = parag;
    int h = view->visibleHeight();
    int y = s->rect().y();
    while ( s ) {
	if ( s->rect().y() - y >= h )
	    break;
	s = s->next();
    }

    if ( !s )
	s = doc->lastParag();

    if ( !s->isValid() )
	return;

    parag = s;
    idx = 0;
#endif
}

void QTextAreaCursor::gotoWordLeft()
{
#if 0
    gotoLeft();
    tmpIndex = -1;
    QTextEditString *s = parag->parag();
    bool allowSame = FALSE;
    for ( int i = idx - 1; i >= 0; --i ) {
	if ( s->at( i ).c.isSpace() || s->at( i ).c == '\t' ) {
	    if ( !allowSame && s->at( i ).c == s->at( idx ).c )
		continue;
	    idx = i + 1;
	    return;
	}
	if ( !allowSame && s->at( i ).c != s->at( idx ).c )
	    allowSame = TRUE;
    }

    if ( parag->prev() ) {
	parag = parag->prev();
	idx = parag->length() - 1;
    } else {
	gotoLineStart();
    }
#endif
}

void QTextAreaCursor::gotoWordRight()
{
#if 0
    tmpIndex = -1;
    QTextEditParag *s = parag->string();
    bool allowSame = FALSE;
    for ( int i = idx + 1; i < (int)s->length(); ++i ) {
	if ( s->at( i ).c.isSpace() || s->at( i ).c == '\t' ) {
	    if ( !allowSame &&  s->at( i ).c == s->at( idx ).c )
		continue;
	    idx = i;
	    return;
	}
	if ( !allowSame && s->at( i ).c != s->at( idx ).c )
	    allowSame = TRUE;
    }

    if ( parag->next() ) {
	parag = parag->next();
	idx = 0;
    } else {
	gotoLineEnd();
    }
#endif
}

bool QTextAreaCursor::atParagStart()
{
    return (line == parag->first() && idx == 0);
}

bool QTextAreaCursor::atParagEnd()
{
    return (line == parag->last() && idx == line->length() - 1);
}

void QTextAreaCursor::splitAndInsertEmtyParag( bool ind, bool updateIds )
{
#if 0
    tmpIndex = -1;
    QTextEditFormat *f = 0;
    if ( !doc->syntaxHighlighter() )
	f = parag->at( idx )->format();

    if ( atParagStart() ) {
	QTextEditParag *p = parag->prev();
	QTextEditParag *s = new QTextEditParag( doc, p, parag, updateIds );
	s->append( " " );
	if ( f )
	    s->setFormat( 0, 1, f, TRUE );
	s->setType( parag->type() );
	s->setListDepth( parag->listDepth() );
	s->setAlignment( parag->alignment() );
	if ( ind ) {
	    s->indent();
	    s->format();
	    indent();
	    parag->format();
	}
    } else if ( atParagEnd() ) {
	QTextEditParag *n = parag->next();
	QTextEditParag *s = new QTextEditParag( doc, parag, n, updateIds );
	s->append( " " );
	if ( f )
	    s->setFormat( 0, 1, f, TRUE );
	s->setType( parag->type() );
	s->setListDepth( parag->listDepth() );
	s->setAlignment( parag->alignment() );
	if ( ind ) {
	    int oi, ni;
	    s->indent( &oi, &ni );
	    parag = s;
	    idx = ni;
	} else {
	    parag = s;
	    idx = 0;
	}
    } else {
	QString str = parag->string()->toString().mid( idx, 0xFFFFFF );
	parag->truncate( idx );
	QTextEditParag *n = parag->next();
	QTextEditParag *s = new QTextEditParag( doc, parag, n, updateIds );
	s->setType( parag->type() );
	s->setListDepth( parag->listDepth() );
	s->setAlignment( parag->alignment() );
	s->append( str );
	if ( f )
	    s->setFormat( 0, str.length(), f, TRUE );
	if ( ind ) {
	    int oi, ni;
	    s->indent( &oi, &ni );
	    parag = s;
	    idx = ni;
	} else {
	    parag = s;
	    idx = 0;
	}
    }
#endif
}

bool QTextAreaCursor::remove()
{
#if 0
    tmpIndex = -1;
    if ( !atParagEnd() ) {
	parag->remove( idx, 1 );
	return FALSE;
    } else if ( parag->next() ) {
	parag->join( parag->next() );
	return TRUE;
    }
    return FALSE;
#endif
}

void QTextAreaCursor::indent()
{
#if 0
    int oi = 0, ni = 0;
    parag->indent( &oi, &ni );
    if ( oi == ni )
	return;

    if ( idx >= oi )
	idx += ni - oi;
    else
	idx = ni;
#endif
}

bool QTextAreaCursor::checkOpenParen()
{
#if 0
    if ( !doc->isParenCheckingEnabled() )
	return FALSE;

    QTextEditParag::ParenList parenList = parag->parenList();

    QTextEditParag::Paren openParen, closedParen;
    QTextEditParag *closedParenParag = parag;

    int i = 0;
    int ignore = 0;
    bool foundOpen = FALSE;
    QChar c = parag->at( idx )->c;
    while ( TRUE ) {
	if ( !foundOpen ) {
	    if ( i >= (int)parenList.count() )
		goto aussi;
	    openParen = parenList[ i ];
	    if ( openParen.pos != idx ) {
		++i;
		continue;
	    } else {
		foundOpen = TRUE;
		++i;
	    }
	}
	
	if ( i >= (int)parenList.count() ) {
	    while ( TRUE ) {
		closedParenParag = closedParenParag->next();
		if ( !closedParenParag )
		    goto aussi;
		if ( closedParenParag->parenList().count() > 0 ) {
		    parenList = closedParenParag->parenList();
		    break;
		}
	    }
	    i = 0;
	}
	
	closedParen = parenList[ i ];
	if ( closedParen.type == QTextEditParag::Paren::Open ) {
	    ignore++;
	    ++i;
	    continue;
	} else {
	    if ( ignore > 0 ) {
		ignore--;
		++i;
		continue;
	    }
	
	    int id = QTextEditDocument::ParenMatch;
	    if ( c == '{' && closedParen.chr != '}' ||
		 c == '(' && closedParen.chr != ')' ||
		 c == '[' && closedParen.chr != ']' )
		id = QTextEditDocument::ParenMismatch;
	    doc->setSelectionStart( id, this );
	    int tidx = idx;
	    QTextEditParag *tparag = parag;
	    idx = closedParen.pos + 1;
	    parag = closedParenParag;
	    doc->setSelectionEnd( id, this );
	    parag = tparag;
	    idx = tidx;
	    return TRUE;
	}
	
	++i;
    }

#endif
 aussi:
    return FALSE;
}

bool QTextAreaCursor::checkClosedParen()
{
#if 0
    if ( !doc->isParenCheckingEnabled() )
	return FALSE;

    QTextEditParag::ParenList parenList = parag->parenList();

    QTextEditParag::Paren openParen, closedParen;
    QTextEditParag *openParenParag = parag;

    int i = parenList.count() - 1;
    int ignore = 0;
    bool foundClosed = FALSE;
    QChar c = parag->at( idx - 1 )->c;
    while ( TRUE ) {
	if ( !foundClosed ) {
	    if ( i < 0 )
		goto aussi;
	    closedParen = parenList[ i ];
	    if ( closedParen.pos != idx - 1 ) {
		--i;
		continue;
	    } else {
		foundClosed = TRUE;
		--i;
	    }
	}
	
	if ( i < 0 ) {
	    while ( TRUE ) {
		openParenParag = openParenParag->prev();
		if ( !openParenParag )
		    goto aussi;
		if ( openParenParag->parenList().count() > 0 ) {
		    parenList = openParenParag->parenList();
		    break;
		}
	    }
	    i = parenList.count() - 1;
	}
	
	openParen = parenList[ i ];
	if ( openParen.type == QTextEditParag::Paren::Closed ) {
	    ignore++;
	    --i;
	    continue;
	} else {
	    if ( ignore > 0 ) {
		ignore--;
		--i;
		continue;
	    }
	
	    int id = QTextEditDocument::ParenMatch;
	    if ( c == '}' && openParen.chr != '{' ||
		 c == ')' && openParen.chr != '(' ||
		 c == ']' && openParen.chr != '[' )
		id = QTextEditDocument::ParenMismatch;
	    doc->setSelectionStart( id, this );
	    int tidx = idx;
	    QTextEditParag *tparag = parag;
	    idx = openParen.pos;
	    parag = openParenParag;
	    doc->setSelectionEnd( id, this );
	    parag = tparag;
	    idx = tidx;
	    return TRUE;
	}
	
	--i;
    }
#endif
 aussi:
    return FALSE;
}

int QTextAreaCursor::index() const
{
    return idx;
}

void QTextAreaCursor::setIndex( int i )
{
    tmpIndex = -1;
    idx = i;
}

int QTextAreaCursor::visualIndex() const
{
    return visual1;
}


bool QTextAreaCursor::checkParens()
{
#if 0
    QChar c( string->at( idx )->c );
    if ( c == '{' || c == '(' || c == '[' ) {
	return checkOpenParen();
    } else if ( idx > 0 ) {
	c = string->at( idx - 1 )->c;
	if ( c == '}' || c == ')' || c == ']' ) {
	    return checkClosedParen();
	}
    }
#endif
    return FALSE;
}

void QTextAreaCursor::setParagraph( QParagraph *s )
{
    idx = 0;
    parag = s;
    tmpIndex = -1;
}

void QTextAreaCursor::checkIndex()
{
    if ( idx >= line->length() )
	idx = line->length() - 1;
}


// =======================================================================


QParagraph::QParagraph(const QRichTextString &t, QTextArea *a, QParagraph *lastPar)
    : text(t)
{
    area = a;
    firstRow = lastRow = 0;
    p = lastPar;
    n = 0;

    hAlign = AlignAuto;
    basicDir = QChar::DirON;

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

QChar::Direction QParagraph::basicDirection() const
{
    if(basicDir == QChar::DirON)
	basicDir = ::basicDirection(text);
    return basicDir;
}

QPoint QParagraph::nextLine() const
{
    if( !lastRow )
	return QPoint(0, 0);
    return QPoint( lastRow->x(), lastRow->y() + lastRow->height() );
}

void QParagraph::layout()
{
    int pos = 0;
    int lineLength = 0;
    QRichTextFormatterBreakWords formatter(area);
    formatter.format(this, 0);
}

void QParagraph::paint(QPainter &p, int x, int y, QTextAreaCursor *c)
{
    //printf("QParagraph::paint\n");
    // #### add a check if we need to paint at all!!!
    HAlignment align = hAlign;
    if(align == AlignAuto) {
	if(basicDirection() == QChar::DirL)
	    align = AlignLeft;
	else
	    align = AlignRight;
    }

    x += xPos;
    y += yPos;
    QTextRow *line = first();
    while(line) {
	line->paint(p, x, y, c, align);
	line = line->next();
    }
}

int QParagraph::insert(int idx, const QString &str)
{
    string()->insert(idx, str, string()->at(idx).format());
    QRichTextFormatterBreakWords formatter(area);
    formatter.format(this, idx);
    return idx + str.length();
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
	return lastFormat;
    }

    if ( f == lastFormat || ( lastFormat && f->key() == lastFormat->key() ) ) {
#ifdef DEBUG_COLLECTION
	qDebug( "need '%s', good case!", f->key().latin1() );
#endif
	return lastFormat;
    }

    QRichTextFormat *fm = cKey.find( f->key() );
    if ( fm ) {
#ifdef DEBUG_COLLECTION
	qDebug( "need '%s', normal case!", f->key().latin1() );
#endif
	lastFormat = fm;
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
    }
					
    return cres;
}

QRichTextFormat *QRichTextFormatCollection::format( const QFont &f, const QColor &c )
{
    if ( cachedFormat && cfont == f && ccol == c ) {
#ifdef DEBUG_COLLECTION
	qDebug( "format of font and col '%s' - best case", cachedFormat->key().latin1() );
#endif
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

void QRichTextFormatCollection::setDefaultFormat( QRichTextFormat *f )
{
    defFormat = f;
}

QRichTextFormat *QRichTextFormatCollection::defaultFormat() const
{
    return defFormat;
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

QRichTextFormat::QRichTextFormat( const QFont &f, const QColor &c )
    : fn( f ), col( c ), fm( new QFontMetrics( f ) )
{
    leftBearing = fm->minLeftBearing();
    rightBearing = fm->minRightBearing();
    hei = fm->height();
    asc = fm->ascent();
    dsc = fm->descent();
    for ( int i = 0; i < 65536; ++i )
	widths[ i ] = 0;
    generateKey();
}

QRichTextFormat::QRichTextFormat( const QRichTextFormat &f )
{
    fn = f.fn;
    col = f.col;
    fm = new QFontMetrics( fn );
    leftBearing = f.leftBearing;
    rightBearing = f.rightBearing;
    for ( int i = 0; i < 65536; ++i )
	widths[ i ] = f.widths[ i ];
    hei = f.hei;
    asc = f.asc;
    dsc = f.dsc;
    generateKey();
}

void QRichTextFormat::update()
{
    *fm = QFontMetrics( fn );
    leftBearing = fm->minLeftBearing();
    rightBearing = fm->minRightBearing();
    hei = fm->height();
    asc = fm->ascent();
    dsc = fm->descent();
    for ( int i = 0; i < 65536; ++i )
	widths[ i ] = 0;
    generateKey();
}

const QFontMetrics *QRichTextFormat::fontMetrics() const
{
    return fm;
}

QColor QRichTextFormat::color() const
{
    return col;
}

QFont QRichTextFormat::font() const
{
    return fn;
}

int QRichTextFormat::minLeftBearing() const
{
    return leftBearing;
}

int QRichTextFormat::minRightBearing() const
{
    return rightBearing;
}

int QRichTextFormat::width( const QChar &c ) const
{
    if ( c == '\t' )
	return 30;
    int w = widths[ c.unicode() ];
    if ( w == 0 ) {
	w = fm->width( c );
	( (QRichTextFormat*)this )->widths[ c.unicode() ] = w;
    }
    return w;
}

int QRichTextFormat::height() const
{
    return hei;
}

int QRichTextFormat::ascent() const
{
    return asc;
}

int QRichTextFormat::descent() const
{
    return dsc;
}

bool QRichTextFormat::operator==( const QRichTextFormat &f ) const
{
    return k == f.k;
}

QRichTextFormatCollection *QRichTextFormat::parent() const
{
    return collection;
}

void QRichTextFormat::addRef()
{
    ref++;
#ifdef DEBUG_COLLECTION
    qDebug( "add ref of '%s' to %d (%p)", k.latin1(), ref, this );
#endif
}

void QRichTextFormat::removeRef()
{
    ref--;
    if ( !collection )
	return;
#ifdef DEBUG_COLLECTION
    qDebug( "remove ref of '%s' to %d (%p)", k.latin1(), ref, this );
#endif
    if ( ref <= 0 )
	collection->remove( this );
}

QString QRichTextFormat::key() const
{
    return k;
}

void QRichTextFormat::generateKey()
{
    QTextOStream ts( &k );
    ts << fn.pointSize()
       << fn.weight()
       << (int)fn.underline()
       << (int)fn.italic()
       << col.pixel()
       << fn.family();
}

QString QRichTextFormat::getKey( const QFont &fn, const QColor &col )
{
    QString k;
    QTextOStream ts( &k );
    ts << fn.pointSize()
       << fn.weight()
       << (int)fn.underline()
       << (int)fn.italic()
       << col.pixel()
       << fn.family();
    return k;
}


// ============================================================================

QRichTextFormatter::QRichTextFormatter( QTextArea *a )
    : area( a )
{
}

QTextRow *QRichTextFormatter::newLine( QParagraph *p, QTextRow *previous )
{
    printf("allocating new line\n");
    QTextRow *r = new QTextRow(p->string(), previous);
    if(previous)
	previous->setNext(r);
    else {
	p->setFirst(r);
	p->setLast(r);
    }
    if( previous == p->last() ) {
	p->setLast(r);
    }
    return r;
}

QRect QRichTextFormatter::openLine(QParagraph *p, QTextRow *line, int from, int height)
{
    printf("open line at %d\n", from);
    line->setFrom(from);
    int x = p->x();
    int y = p->y();
    QTextRow *prev = line->prev();
    if ( prev ) {
	x += prev->x();
	y += prev->y() + prev->height();
    }
    return area->lineRect( x, y );
}

bool QRichTextFormatter::closeLine(QParagraph *p, QTextRow *line, int to, int height, int baseline, int width)
{
    printf("closeLine %d %d\n", line->from(), to - line->from());

    line->setLength( to - line->from() );
    line->setTextWidth(width);
    line->setBaseline(baseline);

    int x = p->x();
    int y = p->y();
    QTextRow *prev = line->prev();
    if ( prev ) {
	x += prev->x();
	y += prev->y() + prev->height();
    }
    QRect r = area->lineRect(x, y, height);

    printf("positioning line at %d/%d width=%d height=%d\n", r.x(), r.y(), r.width(), r.height() );
    // #### check for overflow because of too big height

    // make is relative to the paragraph
    r.moveBy( - p->x(), -p->y() );

    line->setBoundingRect(r);
    line->layout();
    return true;
}

// ============================================================================

QRichTextFormatterBreakWords::QRichTextFormatterBreakWords( QTextArea *a )
    : QRichTextFormatter( a )
{
}


int QRichTextFormatterBreakWords::format( QParagraph *parag, int start )
{
    QRichTextString::Char *c = 0;

    QTextRow *current = parag->last();
    if(!current) {
	// first time we layout the line.
	current = newLine(parag, 0);
	start = 0;
    }
    // find first line to layout.
    if(start != 0) {
	while(1) {
	    if(current->from() < start) {
		start = current->from();
		break;
	    }
	    current = current->prev();
	}
    } else {
	current = parag->first();
    }

    int i = start;
    int lastSpace = -1;
    int tmpBaseLine = 0, tmph = 0;
    int baseline = 0, width = 0;

    QRect lineRect = openLine( parag, current, start, 0 );
    int w = lineRect.width();
    int h = 0;
    int x = 0;

    for ( ; i < parag->string()->length(); ++i ) {
	
	c = &parag->string()->at( i );

	int ww = 0;
	// ### add support for object replacement character.
	if ( c->c.unicode() >= 32 || c->c == '\t' ) {
	    ww = c->format()->width( c->c );
	} else {
	    ww = c->format()->width( ' ' );
	}
	
	if ( x + ww > w ) {
	    if ( lastSpace != -1 )
		i = lastSpace;
	    else {
		h = QMAX( h, tmph );
		baseline = QMAX( baseline, tmpBaseLine );
		width = x;
	    }
	    closeLine(parag, current, i, h, baseline, width);
	    start = i;
	    if(!current->next())
		current = newLine(parag, current);
	    else
		current = current->next();
	    lineRect = openLine(parag, current, start, 0);
	    w = lineRect.width();
	    h = 0;
	    tmph = 0;
	    tmpBaseLine = 0;
	    baseline = 0;
	    x  = 0;
	    lastSpace = -1;
	    continue;
	} else if ( c->c == ' ' ) {
	    tmpBaseLine = QMAX( tmpBaseLine, c->format()->ascent() );
	    tmph = QMAX( tmph, c->format()->height() );
	    h = QMAX( h, tmph );
	    baseline = QMAX( baseline, tmpBaseLine );
	    lastSpace = i+1;
	    width = x;
	    // ### cache lineheight and baseline for the chars up to here!!!
	} else {
	    tmpBaseLine = QMAX( tmpBaseLine, c->format()->ascent() );
	    tmph = QMAX( tmph, c->format()->height() );
	}
	
	c->x = x;
	x += ww;
    }
    width = x;
    h = QMAX( h, tmph );
    baseline = QMAX( baseline, tmpBaseLine );
    closeLine(parag, current, i, h, baseline, width);

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
	    int lw = c->x + c->format()->width( c->c );
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
