/****************************************************************************
**
** Implementation of some internal classes.
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

#include "private/qcomplextext_p.h"

#ifndef QT_NO_COMPLEXTEXT
#include "private/qrichtext_p.h"
#include "private/qfontdata_p.h"
#include "private/qunicodetables_p.h"
#include "qfontmetrics.h"
#include "qrect.h"

#include <stdlib.h>

// -----------------------------------------------------

enum Shape {
    XIsolated,
    XFinal,
    XInitial,
    XMedial
};

/* a small helper class used internally to resolve Bidi embedding levels.
   Each line of text caches the embedding level at the start of the line for faster
   relayouting
*/
QBidiContext::QBidiContext( uchar l, QChar::Direction e, QBidiContext *p, bool o )
    : level(l) , override(o), dir(e)
{
    if ( p )
	p->ref();
    parent = p;
    count = 0;
}

QBidiContext::~QBidiContext()
{
    if( parent && parent->deref() )
	delete parent;
}



#define BIDI_DEBUG 0//2
#if (BIDI_DEBUG >= 1)
#include <iostream>

static const char *directions[] = {
    "DirL", "DirR", "DirEN", "DirES", "DirET", "DirAN", "DirCS", "DirB", "DirS", "DirWS", "DirON",
    "DirLRE", "DirLRO", "DirAL", "DirRLE", "DirRLO", "DirPDF", "DirNSM", "DirBN"
};

#endif

static QChar::Direction basicDirection(const QString &str, int start = 0)
{
    int len = str.length();
    int pos = start > len ? len -1 : start;
    const QChar *uc = str.unicode() + pos;
    while( pos < len ) {
	switch( direction( *uc ) )
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
	++uc;
    }
    if ( start != 0 )
	return basicDirection( str );
    return QChar::DirL;
}

// transforms one line of the paragraph to visual order
// the caller is responisble to delete the returned list of QTextRuns.
QPtrList<QTextRun> *QComplexText::bidiReorderLine( QBidiControl *control, const QString &text, int start, int len,
						   QChar::Direction basicDir )
{
    int last = start + len - 1;
    //printf("doing BiDi reordering from %d to %d!\n", start, last);

    QPtrList<QTextRun> *runs = new QPtrList<QTextRun>;
    runs->setAutoDelete(TRUE);

    QBidiContext *context = control->context;
    if ( !context ) {
	// first line
	if( start != 0 )
	    qDebug( "bidiReorderLine::internal error");
	if( basicDir == QChar::DirR || (basicDir == QChar::DirON && text.isRightToLeft() ) ) {
	    context = new QBidiContext( 1, QChar::DirR );
	    control->status.last = QChar::DirR;
	    control->status.lastStrong = QChar::DirR;
	} else {
	    context = new QBidiContext( 0, QChar::DirL );
	    control->status.last = QChar::DirL;
	    control->status.lastStrong = QChar::DirL;
	}
    }
#if (BIDI_DEBUG >= 2)
    qDebug("basicDir = %s", directions[basicDir] );
#endif

    QBidiStatus status = control->status;
    QChar::Direction dir = QChar::DirON;

    int sor = start;
    int eor = start;
    bool first = TRUE;

    int length = text.length();
    const QChar *unicode = text.unicode();

    int current = start;
    while(current <= last) {
	QChar::Direction dirCurrent;
	if(current == (int)length) {
	    QBidiContext *c = context;
	    while ( c->parent )
		c = c->parent;
	    dirCurrent = c->dir;
	} else if ( current == last ) {
	    dirCurrent = ( basicDir != QChar::DirON ? basicDir : basicDirection( text, current ) );
	} else
	    dirCurrent = ::direction( unicode[current] );


#if (BIDI_DEBUG >= 2)
	cout << "pos=" << current << " dir=" << directions[dir]
	     << " current=" << directions[dirCurrent] << " last=" << directions[status.last]
	     << " eor=" << eor << "/" << directions[status.eor] << " lastStrong="
	     << directions[status.lastStrong]
	     << " embedding=" << directions[context->dir]
	     << " level=" << (int)context->level << endl;
#endif

	switch(dirCurrent) {

	    // embedding and overrides (X1-X9 in the BiDi specs)
	case QChar::DirRLE:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    if ( !first ) {
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor;
			sor = eor;
		    }
		    dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirR, context);
		    status.last = QChar::DirR;
		    status.lastStrong = QChar::DirR;
		}
		break;
	    }
	case QChar::DirLRE:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level++;
		else
		    level += 2;
		if(level < 61) {
		    if ( !first ) {
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor;
			sor = eor;
		    }
		    dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirL, context);
		    status.last = QChar::DirL;
		    status.lastStrong = QChar::DirL;
		}
		break;
	    }
	case QChar::DirRLO:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    if ( !first ) {
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor;
			sor = eor;

		    }
		    dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirR, context, TRUE);
		    dir = QChar::DirR;
		    status.last = QChar::DirR;
		    status.lastStrong = QChar::DirR;
		}
		break;
	    }
	case QChar::DirLRO:
	    {
		uchar level = context->level;
		if(level%2) // we have an odd level
		    level++;
		else
		    level += 2;
		if(level < 61) {
		    if ( !first ) {
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor;
			sor = eor;
		    }
		    dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new QBidiContext(level, QChar::DirL, context, TRUE);
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
		    if ( !first ) {
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor;
			sor = eor;
		    }
		    dir = QChar::DirON; status.eor = QChar::DirON;
		    status.last = context->dir;
		    if( context->deref() ) delete context;
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
		    if ( !first ) {
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor;
			sor = eor;
			dir = direction( unicode[eor] ); status.eor = dir;
		    }
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
				runs->append( new QTextRun(sor, eor, context, dir) );
				++eor; sor = eor; status.eor = QChar::DirON;
				dir = QChar::DirR;
			    }
			    eor = current - 1;
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = direction( unicode[eor] ); status.eor = dir;
			} else {
			    if(status.eor != QChar::DirL) {
				runs->append( new QTextRun(sor, eor, context, dir) );
				++eor; sor = eor; status.eor = QChar::DirON;
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
		case QChar::DirL:
		case QChar::DirEN:
		case QChar::DirAN:
		    if ( !first ) {
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor;
			sor = eor;
			dir = QChar::DirON; status.eor = QChar::DirON;
			break;
		    }
		case QChar::DirR:
		case QChar::DirAL:
		    eor = current; status.eor = QChar::DirR; break;
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
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			    eor = current;
			} else {
			    eor = current - 1;
			    runs->append( new QTextRun(sor, eor, context, dir) );
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
	    // if last strong was AL change EN to AN
	    if(status.lastStrong != QChar::DirAL) {
		if(dir == QChar::DirON) {
		    if(status.lastStrong == QChar::DirL)
			dir = QChar::DirL;
		    else
			dir = QChar::DirEN;
		}
		switch(status.last)
		    {
		    case QChar::DirET:
			if ( status.lastStrong == QChar::DirR || status.lastStrong == QChar::DirAL ) {
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; status.eor = QChar::DirON;
			    dir = QChar::DirAN;
			}
			// fall through
		    case QChar::DirEN:
		    case QChar::DirL:
			eor = current;
			status.eor = dirCurrent;
			break;
		    case QChar::DirR:
		    case QChar::DirAL:
		    case QChar::DirAN:
			if ( !first ) {
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor;
			    sor = eor;
			}
			status.eor = QChar::DirEN;
			dir = QChar::DirAN; break;
		    case QChar::DirES:
		    case QChar::DirCS:
			if(status.eor == QChar::DirEN || dir == QChar::DirAN) {
			    eor = current; break;
			}
		    case QChar::DirBN:
		    case QChar::DirB:
		    case QChar::DirS:
		    case QChar::DirWS:
		    case QChar::DirON:
			if(status.eor == QChar::DirR) {
			    // neutrals go to R
			    eor = current - 1;
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirEN;
			    dir = QChar::DirAN;
			}
			else if( status.eor == QChar::DirL ||
				 (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			    eor = current; status.eor = dirCurrent;
			} else {
			    // numbers on both sides, neutrals get right to left direction
			    if(dir != QChar::DirL) {
				runs->append( new QTextRun(sor, eor, context, dir) );
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				eor = current - 1;
				dir = QChar::DirR;
				runs->append( new QTextRun(sor, eor, context, dir) );
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
		    if ( !first ) {
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor;
			sor = eor;
		    }
		    dir = QChar::DirON; status.eor = QChar::DirAN;
		    break;
		case QChar::DirCS:
		    if(status.eor == QChar::DirAN) {
			eor = current; break;
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
			runs->append( new QTextRun(sor, eor, context, dir) );
			++eor; sor = eor; status.eor = QChar::DirAN;
			dir = QChar::DirAN;
		    } else if( status.eor == QChar::DirL ||
			       (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			eor = current; status.eor = dirCurrent;
		    } else {
			// numbers on both sides, neutrals get right to left direction
			if(dir != QChar::DirL) {
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    eor = current - 1;
			    dir = QChar::DirR;
			    runs->append( new QTextRun(sor, eor, context, dir) );
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirAN;
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

	if(current >= (int)length) break;

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
	    case QChar::DirEN:
		if ( status.last == QChar::DirL ) {
		    status.last = QChar::DirL;
		    break;
		}
		// fall through
	    default:
		status.last = dirCurrent;
	    }

	first = FALSE;
	++current;
    }

#if (BIDI_DEBUG >= 1)
    cout << "reached end of line current=" << current << ", eor=" << eor << endl;
#endif
    eor = current - 1; // remove dummy char

    if ( sor <= eor )
	runs->append( new QTextRun(sor, eor, context, dir) );

    // reorder line according to run structure...

    // first find highest and lowest levels
    uchar levelLow = 128;
    uchar levelHigh = 0;
    QTextRun *r = runs->first();
    while ( r ) {
	//printf("level = %d\n", r->level);
	if ( r->level > levelHigh )
	    levelHigh = r->level;
	if ( r->level < levelLow )
	    levelLow = r->level;
	r = runs->next();
    }

    // implements reordering of the line (L2 according to BiDi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // reversing is only done up to the lowest odd level
    if(!(levelLow%2)) levelLow++;

#if (BIDI_DEBUG >= 1)
    cout << "reorderLine: lineLow = " << (uint)levelLow << ", lineHigh = " << (uint)levelHigh << endl;
    cout << "logical order is:" << endl;
    QPtrListIterator<QTextRun> it2(*runs);
    QTextRun *r2;
    for ( ; (r2 = it2.current()); ++it2 )
	cout << "    " << r2 << "  start=" << r2->start << "  stop=" << r2->stop << "  level=" << (uint)r2->level << endl;
#endif

    int count = runs->count() - 1;

    while(levelHigh >= levelLow)
    {
	int i = 0;
	while ( i < count )
	{
	    while(i < count && runs->at(i)->level < levelHigh) i++;
	    int start = i;
	    while(i <= count && runs->at(i)->level >= levelHigh) i++;
	    int end = i-1;

	    if(start != end)
	    {
		//cout << "reversing from " << start << " to " << end << endl;
		for(int j = 0; j < (end-start+1)/2; j++)
		{
		    QTextRun *first = runs->take(start+j);
		    QTextRun *last = runs->take(end-j-1);
		    runs->insert(start+j, last);
		    runs->insert(end-j, first);
		}
	    }
	    i++;
	    if(i >= count) break;
	}
	levelHigh--;
    }

#if (BIDI_DEBUG >= 1)
    cout << "visual order is:" << endl;
    QPtrListIterator<QTextRun> it3(*runs);
    QTextRun *r3;
    for ( ; (r3 = it3.current()); ++it3 )
    {
	cout << "    " << r3 << endl;
    }
#endif

    control->setContext( context );
    control->status = status;

    return runs;
}


QString QComplexText::bidiReorderString( const QString &str, QChar::Direction basicDir )
{

// ### fix basic direction
    QBidiControl control;
    int lineStart = 0;
    int lineEnd = 0;
    int len = str.length();
    QString visual;
    visual.setUnicode( 0, len );
    QChar *vch = (QChar *)visual.unicode();
    const QChar *ch = str.unicode();
    const QChar *str_uc = ch;
    while( lineStart < len ) {
	lineEnd = lineStart;
	while( lineEnd <= len ) {
	    if ( lineEnd < len && *ch == '\n' )
		break;
	    ch++;
	    lineEnd++;
	}
	QPtrList<QTextRun> *runs = bidiReorderLine( &control, str, lineStart, lineEnd - lineStart, basicDir );

	// reorder the content of the line, and output to visual
	QTextRun *r = runs->first();
	while ( r ) {
	    if(r->level %2) {
		// odd level, need to reverse the string
		int pos = r->stop;
		while(pos >= r->start) {
		    *vch = mirroredChar( str_uc[pos] );
		    vch++;
		    pos--;
		}
	    } else {
		int pos = r->start;
		while(pos <= r->stop) {
		    *vch = str_uc[pos];
		    vch++;
		    pos++;
		}
	    }
	    r = runs->next();
	}
	if ( lineEnd < len && *ch == '\n' ) {
	    *vch = *ch;
	    vch++;
	    ch++;
	    lineEnd++;
	}
	lineStart = lineEnd;
	delete runs;
    }
    return visual;
}

QTextRun::QTextRun(int _start, int _stop, QBidiContext *context, QChar::Direction dir) {
    start = _start;
    stop = _stop;
    if(dir == QChar::DirON) dir = context->dir;

    level = context->level;

    // add level of run (cases I1 & I2)
    if( level % 2 ) {
	if(dir == QChar::DirL || dir == QChar::DirAN || dir == QChar::DirEN )
	    level++;
    } else {
	if( dir == QChar::DirR )
	    level++;
	else if( dir == QChar::DirAN || dir == QChar::DirEN )
	    level += 2;
    }
#if (BIDI_DEBUG >= 1)
    printf("new run: dir=%s from %d, to %d level = %d\n", directions[dir], _start, _stop, level);
#endif
}

#endif //QT_NO_COMPLEXTEXT
