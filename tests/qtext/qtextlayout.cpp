#include "qtextlayout.h"

#include <qpainter.h>
#include <qregexp.h>
#include <iostream.h>

//#define BIDI_DEBUG 2


QChar::Direction basicDirection(const QString &text)
{
    const QChar *ch = text.unicode();
    int pos = 0;
    while( pos < text.length() ) {
	switch( (ch + pos)->direction() )
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

/*
  used internally.
  Represents one line of text in a Rich Text drawing area
*/
QTextLine::QTextLine(const QString &t, int from, int length, QTextLine *previous)
    :  start(from), len(length), text(t), prev(previous)
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

QTextLine::~QTextLine()
{
    // ### care about previous/next????

    startEmbed->deref();
    endEmbed->deref();
}

void QTextLine::paint(QPainter *p, int _x, int _y)
{
    // no rich text formatting....
    // ### no alignment
    // ### should be reordered text
    p->drawText(xPos + _x, yPos + _y, reorderedText );
}

void QTextLine::setPosition(int _x, int _y)
{
    xPos = _x;
    yPos = _y;
}

void QTextLine::setBoundingRect(const QRect &r)
{
    xPos = r.x();
    yPos = r.y();
    w = r.width();
    h = r.height();
}

QRect QTextLine::boundingRect()
{
    return QRect(xPos, yPos, w, h);
}

bool QTextLine::hasComplexText()
{
    complexText = false;
    if(startEmbed->level) {
	// need to apply BiDi
	complexText = true;
	return true;
    }

    const QChar *ch = text.unicode() + start;
    int i = len;
    while(i) {
	if(ch->row() > 0x04) {
	    complexText = true;
	    return true;
	}
	--i;
	++ch;
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
void QTextLine::bidiReorderLine()
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
	    dirCurrent = text[current].direction();

	
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

    reorderedText = "";
    r = runs.first();
    while ( r ) {
	if(r->level %2) {
	    // odd level, need to reverse the string
	    int pos = r->stop;
	    while(pos >= r->start) {
		reorderedText += text[pos];
		pos--;
	    }
	} else {
	    int pos = r->start;
	    while(pos <= r->stop) {
		reorderedText += text[pos];
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
void QTextArea::appendParagraph(const QString &str)
{
    paragraphs.append( createParagraph(str, paragraphs.last()) );
}

/*!
  insert a new paragraph at position pos to the Textarea
*/
void QTextArea::insertParagraph(const QString &str, int pos)
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

QParagraph *QTextArea::createParagraph(const QString &text, QParagraph *before)
{
    return new QParagraph(text, this, before);
}


int QTextArea::lineWidth(int, int, int) const
{
    return width;
}

QRect QTextArea::lineRect(int x, int y, int h) const
{
    return QRect(x, y, width, h);
}

void QTextArea::paint(QPainter *p, int x, int y)
{
    QListIterator<QParagraph> it(paragraphs);
    while(it.current()) {
	(*it)->paint(p, x, y);
	++it;
    }
}



// --------------------------------------------------------



QParagraph::QParagraph(const QString &t, QTextArea *a, QParagraph *lastPar)
{
    area = a;
    first = last = 0;
    text = t;
    text.compose();
    
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
    return QPoint( last->x(), last->y() + last->height() );
}

void QParagraph::layout()
{
    int pos = 0;
    int lineLength = 0;

    while( (lineLength = findLineBreak(pos)) != 0 ) {
	   addLine(pos, lineLength);
	   pos += lineLength;
    }

}


int QParagraph::findLineBreak(int pos)
{
    printf("findLineBreak\n");
    int start = pos;
    QFontMetrics fm(QApplication::font());

    const QChar *ch = text.unicode();                               

    int x = xPos;
    int y = yPos;
    if ( last ) {
	x += last->x();
	y += last->y() + last->height();
    }
    printf("new line at %d/%d\n", x, y);
    int width = area->lineWidth(x, y);
    int pos2 = pos;

    while(1) {
	while(pos2 < text.length() && !(ch+pos2)->isSpace()) {
	    pos2++;
	}
	
	// we know the string is not going to get modified....
	width -= fm.width(QConstString(const_cast<QChar *>(text.unicode() + pos), pos2 - pos).string());
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
    QTextLine *line = new QTextLine(text, start, length, last);

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

void QParagraph::paint(QPainter *p, int x, int y)
{
    // #### add a check if we need to paint at all!!!

    x += xPos;
    y += yPos;
    QTextLine *line = first;
    while(line) {
	line->paint(p, x, y);
	line = line->nextLine();
    }
}

