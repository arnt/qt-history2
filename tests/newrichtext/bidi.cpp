#include <qstring.h>

#include "qtextdata.h"

#define BIDI_DEBUG 1//2
#if (BIDI_DEBUG >= 1)
#include <iostream>

static const char *directions[] = {
    "DirL", "DirR", "DirEN", "DirES", "DirET", "DirAN", "DirCS", "DirB", "DirS", "DirWS", "DirON",
    "DirLRE", "DirLRO", "DirAL", "DirRLE", "DirRLO", "DirPDF", "DirNSM", "DirBN"
};

#endif

struct BidiStatus {
    BidiStatus() {
	eor = QChar::DirON;
	lastStrong = QChar::DirON;
	last = QChar:: DirON;
	dir = QChar::DirON;
    }
    QChar::Direction eor;
    QChar::Direction lastStrong;
    QChar::Direction last;
    QChar::Direction dir;
};

struct BidiControl {
    struct Context {
	unsigned char level : 6;
	int override : 1;
	int unused : 1;
    };

    BidiControl( bool rtl )
	: cCtx( 0 ) {
	ctx[0].level = (rtl ? 1 : 0);
	ctx[0].override = FALSE;
    }

    void embed( int level, bool override = FALSE ) {
	if ( ctx[cCtx].level < 61 && cCtx < 61 ) {
	    ++cCtx;
	    ctx[cCtx].level = level;
	    ctx[cCtx].override = override;
	}
    }
    void pdf() {
	if ( cCtx ) --cCtx;
    }

    uchar level() const {
	return ctx[cCtx].level;
    }
    bool override() const {
	return ctx[cCtx].override;
    }
    QChar::Direction basicDirection() {
	return (ctx[0].level ? QChar::DirR : QChar:: DirL );
    }
    QChar::Direction direction() {
	return (ctx[cCtx].level ? QChar::DirR : QChar:: DirL );
    }

    Context ctx[63];
    unsigned int cCtx : 8;
};

static QChar::Direction basicDirection( const QString &str )
{
    int len = str.length();
    int pos = 0;
    const QChar *uc = str.unicode() + pos;
    while( pos < len ) {
	switch( uc->direction() )
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
    return QChar::DirL;
}




static void appendItems(ScriptItemArray &items, int &start, int &stop, BidiControl &control, QChar::Direction dir,
			const QChar *text ) {
    if ( start > stop ) {
	qWarning( "Bidi: appendItems() internal error" );
	return;
    }

    int level = control.level();

    if(dir != QChar::DirON) {
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
    }

#if (BIDI_DEBUG >= 1)
    qDebug("new run: dir=%s from %d, to %d level = %d\n", directions[dir], start, stop, level);
#endif
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, text[start] );
    ScriptItem item;
    item.position = start;
    item.analysis.script = script;
    item.analysis.bidiLevel = level;
    item.analysis.override = control.override();
    item.analysis.linkBefore = FALSE;
    item.analysis.linkAfter = FALSE;
    item.analysis.reserved = 0;

    items.append( item );
    for ( int i = start+1; i <= stop; i++ ) {

	QFont::Script s;
	SCRIPT_FOR_CHAR( s, text[i] );
	if ( s != script && !text[i].isSpace() ) {
	    ScriptItem item;
	    item.position = i;
	    item.analysis.script = s;
	    item.analysis.bidiLevel = level;
	    item.analysis.override = control.override();
	    item.analysis.linkBefore = FALSE;
	    item.analysis.linkAfter = FALSE;
	    item.analysis.reserved = 0;

	    items.append( item );
	    script = s;
	    start = i+1;
	}
    }

    ++stop;
    start = stop;
}


// creates the next Script items.
static void bidiItemize( const QString &text, ScriptItemArray &items, QChar::Direction dir = QChar::DirON,
			 const QRTFormatArray *formats = 0 )
{
    if ( dir == QChar::DirON )
	dir = basicDirection( text );
#if (BIDI_DEBUG >= 2)
    qDebug("basicDir = %s", directions[dir] );
#endif

    BidiControl control( dir == QChar::DirR );

    int sor = 0;
    int eor = 0;

    // ### should get rid of this!
    bool first = TRUE;

    BidiStatus status;
    int length = text.length();
    const QChar *unicode = text.unicode();
    int current = 0;

    while ( current <= length ) {

	QChar::Direction dirCurrent;
	if ( current == (int)length )
	    dirCurrent = control.basicDirection();
	else
	    dirCurrent = QTextData::direction( unicode[current] );

#if (BIDI_DEBUG >= 2)
	cout << "pos=" << current << " dir=" << directions[dir]
	     << " current=" << directions[dirCurrent] << " last=" << directions[status.last]
	     << " eor=" << eor << "/" << directions[status.eor] << " lastStrong="
	     << directions[status.lastStrong]
	     << " level=" << (int)control.level() << endl;
#endif

	switch(dirCurrent) {

	    // embedding and overrides (X1-X9 in the BiDi specs)
	case QChar::DirRLE:
	case QChar::DirRLO:
	case QChar::DirLRE:
	case QChar::DirLRO:
	    {
		bool rtl = (dirCurrent == QChar::DirRLE || dirCurrent == QChar::DirRLO );
		bool override = (dirCurrent == QChar::DirLRO || dirCurrent == QChar::DirRLO );

		uchar level = control.level();
		if( (level%2 != 0) == rtl  )
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    if ( !first )
			appendItems( items, sor, eor, control, dir, unicode );
		    dir = QChar::DirON; status.eor = QChar::DirON;
		    QChar::Direction edir = (rtl ? QChar::DirR : QChar::DirL );
		    control.embed( edir, override );
		    status.last = edir;
		    status.lastStrong = edir;
		}
		break;
	    }
	case QChar::DirPDF:
	    {
		if ( !first )
		    appendItems( items, sor, eor, control, dir, unicode );
		dir = QChar::DirON; status.eor = QChar::DirON;
		status.last = control.direction();
		control.pdf();
		if ( control.override() )
		    dir = control.direction();
		else
		    dir = QChar::DirON;
		status.lastStrong = control.direction();
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
			appendItems( items, sor, eor, control, dir, unicode );
			dir = QTextData::direction( unicode[eor] ); status.eor = dir;
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
			if( control.direction() == QChar::DirR ) {
			    if(status.eor != QChar::DirR) {
				// AN or EN
				appendItems( items, sor, eor, control, dir, unicode );
				status.eor = QChar::DirON;
				dir = QChar::DirR;
			    }
			    else
				eor = current - 1;
			    appendItems( items, sor, eor, control, dir, unicode );
			    dir = QTextData::direction( unicode[eor] ); status.eor = dir;
			} else {
			    if(status.eor != QChar::DirL) {
				appendItems( items, sor, eor, control, dir, unicode );
				status.eor = QChar::DirON;
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
			appendItems( items, sor, eor, control, dir, unicode );
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
			if(control.direction() == QChar::DirR || status.lastStrong == QChar::DirR) {
			    appendItems( items, sor, eor, control, dir, unicode );
			    dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			    eor = current;
			} else {
			    eor = current - 1;
			    appendItems( items, sor, eor, control, dir, unicode );
			    dir = QChar::DirON; status.eor = QChar::DirON;
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
	    // if last strong was AL change EN to AL
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
			    appendItems( items, sor, eor, control, dir, unicode );
			    status.eor = QChar::DirON;
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
			if ( !first )
			    appendItems( items, sor, eor, control, dir, unicode );
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
			    appendItems( items, sor, eor, control, dir, unicode );
			    dir = QChar::DirON; status.eor = QChar::DirEN;
			    dir = QChar::DirAN;
			}
			else if( status.eor == QChar::DirL ||
				 (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			    eor = current; status.eor = dirCurrent;
			} else {
			    // numbers on both sides, neutrals get right to left direction
			    if(dir != QChar::DirL) {
				appendItems( items, sor, eor, control, dir, unicode );
				dir = QChar::DirON; status.eor = QChar::DirON;
				eor = current - 1;
				dir = QChar::DirR;
				appendItems( items, sor, eor, control, dir, unicode );
				dir = QChar::DirON; status.eor = QChar::DirON;
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
		    if ( !first )
			appendItems( items, sor, eor, control, dir, unicode );
		    dir = QChar::DirON; status.eor = QChar::DirAN;
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
			appendItems( items, sor, eor, control, dir, unicode );
			dir = QChar::DirON; status.eor = QChar::DirAN;
			dir = QChar::DirAN;
		    } else if( status.eor == QChar::DirL ||
			       (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			eor = current; status.eor = dirCurrent;
		    } else {
			// numbers on both sides, neutrals get right to left direction
			if(dir != QChar::DirL) {
			    appendItems( items, sor, eor, control, dir, unicode );
			    dir = QChar::DirON; status.eor = QChar::DirON;
			    eor = current - 1;
			    dir = QChar::DirR;
			    appendItems( items, sor, eor, control, dir, unicode );
			    dir = QChar::DirON; status.eor = QChar::DirAN;
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

	//cout << "     after: dir=" << //        dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << control.direction() << endl;

	if(current >= (int)length) break;

	// set status.last as needed.
	switch(dirCurrent) {
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
	appendItems( items, sor, eor, control, dir, unicode );


}

static void bidiReorder( int numItems, const Q_UINT8 *levels, int *visualOrder )
{

    // first find highest and lowest levels
    uchar levelLow = 128;
    uchar levelHigh = 0;
    int i = 0;
    while ( i < numItems ) {
	//printf("level = %d\n", r->level);
	if ( levels[i] > levelHigh )
	    levelHigh = levels[i];
	if ( levels[i] < levelLow )
	    levelLow = levels[i];
	i++;
    }

    // implements reordering of the line (L2 according to BiDi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // reversing is only done up to the lowest odd level
    if(!(levelLow%2)) levelLow++;

#if (BIDI_DEBUG >= 1)
    cout << "reorderLine: lineLow = " << (uint)levelLow << ", lineHigh = " << (uint)levelHigh << endl;
#endif

    int count = numItems - 1;
    for ( i = 0; i < numItems; i++ )
	visualOrder[i] = i;

    while(levelHigh >= levelLow) {
	int i = 0;
	while ( i < count ) {
	    while(i < count && levels[i] < levelHigh) i++;
	    int start = i;
	    while(i <= count && levels[i] >= levelHigh) i++;
	    int end = i-1;

	    if(start != end) {
		cout << "reversing from " << start << " to " << end << endl;
		for(int j = 0; j < (end-start+1)/2; j++) {
		    int tmp = visualOrder[start+j];
		    visualOrder[start+j] = visualOrder[end-j];
		    visualOrder[end-j] = tmp;
		}
	    }
	    i++;
	}
	levelHigh--;
    }

#if (BIDI_DEBUG >= 1)
    cout << "visual order is:" << endl;
    for ( i = 0; i < numItems; i++ )
	cout << visualOrder[i] << endl;
#endif
}
