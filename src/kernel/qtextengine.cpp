#include "qtextengine_p.h"

#include "qscriptengine_p.h"

#include <stdlib.h>

#include <qfont.h>
#include "qfontdata_p.h"
#include "qfontengine_p.h"

#include <qstring.h>

#include <private/qunicodetables_p.h>
#include <qfont.h>

// -----------------------------------------------------------------------------------------------------
//
// The BiDi algorithm
//
// -----------------------------------------------------------------------------------------------------


#define BIDI_DEBUG 0//2
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
	unsigned char override : 1;
	unsigned char unused : 1;
    };

    inline BidiControl( bool rtl )
	: cCtx( 0 ), singleLine( FALSE ) {
	ctx[0].level = (rtl ? 1 : 0);
	ctx[0].override = FALSE;
    }

    inline void embed( int level, bool override = FALSE ) {
	if ( ctx[cCtx].level < 61 && cCtx < 61 ) {
	    (void) ++cCtx;
	    ctx[cCtx].level = level;
	    ctx[cCtx].override = override;
	}
    }
    inline void pdf() {
	if ( cCtx ) (void) --cCtx;
    }

    inline uchar level() const {
	return ctx[cCtx].level;
    }
    inline bool override() const {
	return ctx[cCtx].override;
    }
    inline QChar::Direction basicDirection() {
	return (ctx[0].level ? QChar::DirR : QChar:: DirL );
    }
    inline uchar baseLevel() {
	return ctx[0].level;
    }
    inline QChar::Direction direction() {
	return (ctx[cCtx].level ? QChar::DirR : QChar:: DirL );
    }

    Context ctx[63];
    unsigned int cCtx : 8;
    bool singleLine : 8;
};

static QChar::Direction basicDirection( const QString &str )
{
    int len = str.length();
    int pos = 0;
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
    return QChar::DirL;
}




static void appendItems(QScriptItemArray &items, int &start, int &stop, BidiControl &control, QChar::Direction dir,
			const QChar *text ) {
    if ( start > stop ) {
	// #### the algorithm is currently not really safe against this. Still needs fixing.
// 	qWarning( "Bidi: appendItems() internal error" );
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
    QFont::Script script = (QFont::Script)scriptForChar( text[start].unicode() );
    QScriptItem item;
    item.position = start;
    item.analysis.script = script;
    item.analysis.bidiLevel = level;
    item.analysis.override = control.override();
    item.analysis.reserved = 0;

    items.append( item );
    for ( int i = start+1; i <= stop; i++ ) {

	unsigned short uc = text[i].unicode();
	QFont::Script s = (QFont::Script)scriptForChar( uc );

	QChar::Category category = ::category( uc );
	if ( !control.singleLine && uc == 0xfffcU || uc == 0x2028U ) {
	    item.analysis.bidiLevel = level;
	    item.analysis.script = QFont::Latin;
	    item.isObject = TRUE;
	    s = QFont::NoScript;
	} else if ( !control.singleLine && ( (uc >= 9 && uc <=13) ||
			  (category >= QChar::Separator_Space && category <= QChar::Separator_Paragraph ) ) ) {
	    item.analysis.script = QFont::Latin;
	    item.isSpace = TRUE;
	    item.isTab = ( uc == '\t' );
	    item.analysis.bidiLevel = item.isTab ? control.baseLevel() : level;
	    s = QFont::NoScript;
	} else if ( s != script && category != QChar::Mark_NonSpacing ) {
	    item.analysis.script = s;
	    item.analysis.bidiLevel = level;
	} else {
	    continue;
	}

	item.position = i;
	items.append( item );
	script = s;
	start = i+1;
	item.isSpace = item.isTab = item.isObject = FALSE;
    }

    ++stop;
    start = stop;
}


// creates the next QScript items.
static void bidiItemize( const QString &text, QScriptItemArray &items, bool rightToLeft, int mode )
{
    BidiControl control( rightToLeft );
    if ( mode & QTextEngine::SingleLine )
	control.singleLine = TRUE;

    QChar::Direction dir = rightToLeft ? QChar::DirR : QChar::DirL;

    int sor = 0;
    int eor = 0;

    // ### should get rid of this!
    bool first = TRUE;

    int length = text.length();

    if ( !length )
	return;

    const QChar *unicode = text.unicode();
    int current = 0;

    BidiStatus status;
    QChar::Direction sdir = direction( *unicode );
    if ( sdir != QChar::DirL && sdir != QChar::DirR && sdir != QChar::DirEN && sdir != QChar::DirAN )
	sdir = QChar::DirON;
    status.eor = sdir;
    status.lastStrong = sdir;
    status.last = sdir;
    status.dir = sdir;

    while ( current <= length ) {

	QChar::Direction dirCurrent;
	if ( current == (int)length )
	    dirCurrent = control.basicDirection();
	else
	    dirCurrent = direction( unicode[current] );

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
			dir = eor < length ? direction( unicode[eor] ) : control.basicDirection();
			status.eor = dir;
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
			    eor = current - 1;
			    appendItems( items, sor, eor, control, dir, unicode );
			    dir = eor < length ? direction( unicode[eor] ) : control.basicDirection();
			    status.eor = dir;
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
		//cout << "reversing from " << start << " to " << end << endl;
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


#if defined( Q_WS_X11 ) || defined ( Q_WS_QWS )
# include "qtextengine_unix.cpp"
#elif defined( Q_WS_WIN )
# include "qtextengine_win.cpp"
#elif defined( Q_WS_MAC )
# include "qtextengine_mac.cpp"
#endif



QTextEngine::QTextEngine( const QString &str, QFontPrivate *f )
    : string( str ), fnt( f ), direction( QChar::DirON ), haveCharAttributes( FALSE ), widthOnly( FALSE )
{
#ifdef Q_WS_WIN
    if ( !resolvedUsp10 )
	resolveUsp10();
#endif
    if ( fnt ) fnt->ref();

    num_glyphs = QMAX( 16, str.length()*3/2 );
    int space_charAttributes = (sizeof(QCharAttributes)*str.length()+sizeof(void*)-1)/sizeof(void*);
    int space_glyphs = (sizeof(glyph_t)*num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_advances = (sizeof(advance_t)*num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_offsets = (sizeof(offset_t)*num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_logClusters = (sizeof(unsigned short)*num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_glyphAttributes = (sizeof(GlyphAttributes)*num_glyphs+sizeof(void*)-1)/sizeof(void*);

    allocated = space_charAttributes + space_glyphs + space_advances +
		space_offsets + space_logClusters + space_glyphAttributes;
    memory = (void **)::malloc( allocated*sizeof( void * ) );
    memset( memory, 0, allocated*sizeof( void * ) );

    void **m = memory;
    m += space_charAttributes;
    glyphPtr = (glyph_t *) m;
    m += space_glyphs;
    advancePtr = (advance_t *) m;
    m += space_advances;
    offsetsPtr = (offset_t *) m;
    m += space_offsets;
    logClustersPtr = (unsigned short *) m;
    m += space_logClusters;
    glyphAttributesPtr = (GlyphAttributes *) m;

    used = 0;
}

QTextEngine::~QTextEngine()
{
    if ( fnt ) fnt->deref();
    free( memory );
    allocated = 0;
}

void QTextEngine::reallocate( int totalGlyphs )
{
    int new_num_glyphs = totalGlyphs;
    int space_charAttributes = (sizeof(QCharAttributes)*string.length()+sizeof(void*)-1)/sizeof(void*);
    int space_glyphs = (sizeof(glyph_t)*new_num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_advances = (sizeof(advance_t)*new_num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_offsets = (sizeof(offset_t)*new_num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_logClusters = (sizeof(unsigned short)*new_num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_glyphAttributes = (sizeof(GlyphAttributes)*new_num_glyphs+sizeof(void*)-1)/sizeof(void*);

    int newAllocated = space_charAttributes + space_glyphs + space_advances +
		space_offsets + space_logClusters + space_glyphAttributes;
    void ** newMemory = (void **)::malloc( newAllocated*sizeof( void * ) );

    void **nm = newMemory;
    void **m = memory;
    memcpy( nm, m, num_glyphs*sizeof(QCharAttributes) );
    m += space_charAttributes;
    nm += space_charAttributes;
    memcpy( nm, m, num_glyphs*sizeof(glyph_t) );
    glyphPtr = (glyph_t *) nm;
    m += space_glyphs;
    nm += space_glyphs;
    memcpy( nm, m, num_glyphs*sizeof(advance_t) );
    advancePtr = (advance_t *) nm;
    m += space_advances;
    nm += space_advances;
    memcpy( nm, m, num_glyphs*sizeof(offset_t) );
    offsetsPtr = (offset_t *) nm;
    m += space_offsets;
    nm += space_offsets;
    memcpy( nm, m, num_glyphs*sizeof(unsigned short) );
    logClustersPtr = (unsigned short *) nm;
    m += space_logClusters;
    nm += space_logClusters;
    memcpy( nm, m, num_glyphs*sizeof(GlyphAttributes) );
    glyphAttributesPtr = (GlyphAttributes *) nm;

    free( memory );
    memory = newMemory;
    allocated = newAllocated;
    num_glyphs = new_num_glyphs;
}

const QCharAttributes *QTextEngine::attributes()
{
    // ### fix for uniscribe
    QCharAttributes *charAttributes = (QCharAttributes *) memory;
    if ( haveCharAttributes )
	return charAttributes;

    if ( !items.d )
	itemize();

    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem &si = items[i];
	int from = si.position;
	int len = length( i );
	int script = si.analysis.script;
	Q_ASSERT( script < QFont::NScripts );
	scriptEngines[si.analysis.script].charAttributes( script, string, from, len, charAttributes );
    }
    haveCharAttributes = TRUE;
    return charAttributes;
}

int QTextEngine::width( int from, int len ) const
{
    int w = 0;

//     qDebug("QTextEngine::width( from = %d, len = %d ), numItems=%d, strleng=%d", from,  len, items.size(), string.length() );
    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem *si = &items[i];
	int pos = si->position;
	int ilen = length( i );
// 	qDebug("item %d: from %d len %d", i, pos, ilen );
	if ( pos >= from + len )
	    break;
	if ( pos + ilen > from ) {
	    if ( !si->num_glyphs )
		shape( i );

	    advance_t *advances = this->advances( si );
	    unsigned short *logClusters = this->logClusters( si );

// 	    fprintf( stderr, "  logclusters:" );
// 	    for ( int k = 0; k < ilen; k++ )
// 		fprintf( stderr, " %d", logClusters[k] );
// 	    fprintf( stderr, "\n" );
	    // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
	    int charFrom = from - pos;
	    if ( charFrom < 0 )
		charFrom = 0;
	    int glyphStart = logClusters[charFrom];
	    if ( charFrom > 0 && logClusters[charFrom-1] == glyphStart )
		while ( charFrom < ilen && logClusters[charFrom] == glyphStart )
		    charFrom++;
	    if ( charFrom < ilen ) {
		glyphStart = logClusters[charFrom];
		int charEnd = from + len - 1 - pos;
		if ( charEnd >= ilen )
		    charEnd = ilen-1;
		int glyphEnd = logClusters[charEnd];
		while ( charEnd < ilen && logClusters[charEnd] == glyphEnd )
		    charEnd++;
		glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];

// 		qDebug("char: start=%d end=%d / glyph: start = %d, end = %d", charFrom, charEnd, glyphStart, glyphEnd );
		for ( int i = glyphStart; i < glyphEnd; i++ )
		    w += advances[i];
	    }
	}
    }
//     qDebug("   --> w= %d ", w );
    return w;
}

glyph_metrics_t QTextEngine::boundingBox( int from,  int len ) const
{
    glyph_metrics_t gm;

    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem *si = &items[i];
	int pos = si->position;
	int ilen = length( i );
	if ( pos > from + len )
	    break;
	if ( pos + len > from ) {
	    if ( !si->num_glyphs )
		shape( i );
	    advance_t *advances = this->advances( si );
	    unsigned short *logClusters = this->logClusters( si );
	    glyph_t *glyphs = this->glyphs( si );
	    offset_t *offsets = this->offsets( si );

	    // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
	    int charFrom = from - pos;
	    if ( charFrom < 0 )
		charFrom = 0;
	    int glyphStart = logClusters[charFrom];
	    if ( charFrom > 0 && logClusters[charFrom-1] == glyphStart )
		while ( charFrom < ilen && logClusters[charFrom] == glyphStart )
		    charFrom++;
	    if ( charFrom < ilen ) {
		glyphStart = logClusters[charFrom];
		int charEnd = from + len - 1 - pos;
		if ( charEnd >= ilen )
		    charEnd = ilen-1;
		int glyphEnd = logClusters[charEnd];
		while ( charEnd < ilen && logClusters[charEnd] == glyphEnd )
		    charEnd++;
		glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];
		if ( glyphStart <= glyphEnd  ) {
		    QFontEngine *fe = si->fontEngine;
		    glyph_metrics_t m = fe->boundingBox( glyphs+glyphStart, advances+glyphStart,
						       offsets+glyphStart, glyphEnd-glyphStart );
		    gm.x = QMIN( gm.x, m.x + gm.xoff );
		    gm.y = QMIN( gm.y, m.y + gm.yoff );
		    gm.width = QMAX( gm.width, m.width+gm.xoff );
		    gm.height = QMAX( gm.height, m.height+gm.yoff );
		    gm.xoff += m.xoff;
		    gm.yoff += m.yoff;
		}
	    }
	}
    }
    return gm;
}

