#include "qtextlayout.h"

#include <stdlib.h>

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





// There are no typos here... really...
static const unsigned char extendedLatinTable[128] = {
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,

    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::Latin,
    QFont::Latin,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,

    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::Latin,
    QFont::Latin,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,

    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::Latin,
    QFont::Latin,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::Latin,

    QFont::Latin,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::Latin,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::Latin,
    QFont::Latin,

    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_15,
    QFont::LatinExtendedA_15,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,

    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_3,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,

    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_4,
    QFont::LatinExtendedA_14,
    QFont::LatinExtendedA_14,
    QFont::LatinExtendedA_14,
    QFont::LatinExtendedA_14,
    QFont::LatinExtendedA_15,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::LatinExtendedA_2,
    QFont::Latin
};

static const unsigned char otherScripts [116] = {
#define SCRIPTS_02 0
    0xaf, QFont::Latin, 0xff, QFont::SpacingModifiers, 			// row 0x02, index 0
#define SCRIPTS_03 4
    0x6f, QFont::CombiningMarks, 0xff, QFont::Greek, 			// row 0x03, index 4
#define SCRIPTS_05 8
    0x2f, QFont::Cyrillic, 0x8f, QFont::Armenian, 0xff, QFont::Hebrew,	// row 0x05, index 8
#define SCRIPTS_07 14
    0x4f, QFont::Syriac, 0x7f, QFont::Unicode, 0xbf, QFont::Thaana,
    0xff, QFont::Unicode, 						// row 0x07, index 14
#define SCRIPTS_10 22
    0x9f, QFont::Myanmar, 0xff, QFont::Georgian,			// row 0x10, index 20
#define SCRIPTS_13 26
    0x7f, QFont::Ethiopic, 0x9f, QFont::Unicode, 0xff, QFont::Cherokee,	// row 0x13, index 24
#define SCRIPTS_16 32
    0x7f, QFont::CanadianAboriginal, 0x9f, QFont::Ogham,
    0xff, QFont::Runic, 						// row 0x16 index 30
#define SCRIPTS_17 38
//     0x1f, QFont::Tagalog, 0x3f, QFont::Hanunoo, 0x5f, QFont::Buhid,
//     0x7f, QFont::Tagbanwa, 0xff, QFont::Khmer,				// row 0x17, index 36
    0x1f, QFont::Unicode, 0x3f, QFont::Unicode, 0x5f, QFont::Unicode,
    0x7f, QFont::Unicode, 0xff, QFont::Khmer,				// row 0x17, index 36
#define SCRIPTS_18 48
    0xaf, QFont::Mongolian, 0xff, QFont::Unicode,		       	// row 0x18, index 46
#define SCRIPTS_20 52
    0x6f, QFont::Unicode, 0x9f, QFont::NumberForms,
    0xab, QFont::CurrencySymbols, 0xac, QFont::LatinExtendedA_15,
    0xcf, QFont::CurrencySymbols, 0xff, QFont::CombiningMarks,		// row 0x20, index 50
#define SCRIPTS_21 64
    0x4f, QFont::LetterlikeSymbols, 0x8f, QFont::NumberForms,
    0xff, QFont::MathematicalOperators,					// row 0x21, index 62
#define SCRIPTS_24 70
    0x5f, QFont::TechnicalSymbols, 0xff, QFont::EnclosedAndSquare,	// row 0x24, index 68
#define SCRIPTS_2e 74
    0x7f, QFont::Unicode, 0xff, QFont::Han,				// row 0x2e, index 72
#define SCRIPTS_30 78
    0x3f, QFont::Han, 0x9f, QFont::Hiragana, 0xff, QFont::Katakana,	// row 0x30, index 76
#define SCRIPTS_31 84
    0x2f, QFont::Bopomofo, 0x8f, QFont::Hangul, 0x9f, QFont::Han,
    0xff, QFont::Unicode,						// row 0x31, index 82
#define SCRIPTS_fb 92
    0x06, QFont::Latin, 0x1c, QFont::Unicode, 0x4f, QFont::Hebrew,
    0xff, QFont::Arabic,						// row 0xfb, index 90
#define SCRIPTS_fe 100
    0x1f, QFont::Unicode, 0x2f, QFont::CombiningMarks, 0x6f, QFont::Unicode,
    0xff, QFont::Arabic,						// row 0xfe, index 98
#define SCRIPTS_ff 108
    0x5e, QFont::Katakana, 0x60, QFont::Unicode,
    0x9f, QFont::KatakanaHalfWidth, 0xff, QFont::Unicode		// row 0xff, index 106
};

// (uc-0x0900)>>7
static const unsigned char indicScripts [] =
{
    QFont::Devanagari, QFont::Bengali,
    QFont::Gurmukhi, QFont::Gujarati,
    QFont::Oriya, QFont::Tamil,
    QFont::Telugu, QFont::Kannada,
    QFont::Malayalam, QFont::Sinhala,
    QFont::Thai, QFont::Lao
};


#define SCRIPTS_LATINEXT 0x7f
#define SCRIPTS_INDIC 0x7e
// 0x80 + x: x is the offset into the otherScripts table
static const unsigned char scriptTable[256] =
{
    QFont::LatinBasic, SCRIPTS_LATINEXT, 0x80+SCRIPTS_02, 0x80+SCRIPTS_03,
    QFont::Cyrillic, 0x80+SCRIPTS_05, QFont::Arabic, 0x80+SCRIPTS_07,
    QFont::Unicode, SCRIPTS_INDIC, SCRIPTS_INDIC, SCRIPTS_INDIC,
    SCRIPTS_INDIC, SCRIPTS_INDIC, SCRIPTS_INDIC, QFont::Tibetan,

    0x80+SCRIPTS_10, QFont::Hangul, QFont::Ethiopic, 0x80+SCRIPTS_13,
    QFont::CanadianAboriginal, QFont::CanadianAboriginal, 0x80+SCRIPTS_16, 0x80+SCRIPTS_17,
    0x80+SCRIPTS_18, QFont::Unicode, QFont::Unicode, QFont::Unicode,
    QFont::Unicode, QFont::Unicode, QFont::Latin, QFont::Greek,

    0x80+SCRIPTS_20, 0x80+SCRIPTS_21, QFont::MathematicalOperators, QFont::TechnicalSymbols,
    0x80+SCRIPTS_24, QFont::GeometricSymbols, QFont::MiscellaneousSymbols, QFont::MiscellaneousSymbols,
    QFont::Braille, QFont::Unicode, QFont::Unicode, QFont::Unicode,
    QFont::Unicode, QFont::Unicode, 0x80+SCRIPTS_2e, QFont::Han,

    0x80+SCRIPTS_30, 0x80+SCRIPTS_31, QFont::EnclosedAndSquare, QFont::EnclosedAndSquare,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,


    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Yi, QFont::Yi, QFont::Yi, QFont::Yi, QFont::Yi, QFont::Unicode, QFont::Unicode, QFont::Unicode,
    QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,

    QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,
    QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,

    QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,
    QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,

    QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,
    QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode,

    QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode,
    QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode,

    QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode,
    QFont::Unicode, QFont::Han, QFont::Han, 0x80+SCRIPTS_fb, QFont::Arabic, QFont::Arabic, 0x80+SCRIPTS_fe, 0x80+SCRIPTS_ff
};

static inline QFont::Script scriptForChar( ushort uc )
{
    unsigned char script = scriptTable[(uc>>8)];
    if ( script >= SCRIPTS_INDIC ) {
	if ( script == SCRIPTS_LATINEXT ) {
	    unsigned char cell = uc &0xff;
	    if ( cell < 0x80 )
		script = extendedLatinTable[cell];
	    else
		script = QFont::Latin;
	} else if ( script == SCRIPTS_INDIC ) {
	    script = indicScripts[ (uc-0x0900)>>7 ];
	} else {
	    // 0x80 + SCRIPTS_xx
	    unsigned char index = script-0x80;
	    unsigned char cell = uc &0xff;
	    while( otherScripts[index] < cell )
		index += 2;
	    script = otherScripts[index+1];
	}
    }
    return (QFont::Script)script;
}

#ifdef Q_WS_X11
#define SCRIPT_FOR_CHAR( script, c ) 	\
do { 						\
    unsigned short _uc = (c).unicode(); 		\
    if ( _uc < 0x100 ) {				\
	script = QFont::LatinBasic;		\
    } else { 					\
        script = ::scriptForChar( _uc ); 	\
    } 						\
} while( FALSE )

#else
#define SCRIPT_FOR_CHAR( script, c ) script = ::scriptForChar( c )
#endif



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
    QFont::Script script = QFont::UnknownScript;
    for ( int i = start; i <= stop; i++ ) {

	QFont::Script s;
	SCRIPT_FOR_CHAR( s, text[i] );
	if ( s != script ) {
	    ScriptItem item;
	    item.position = start;
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
	    dirCurrent = unicode[current].direction();

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
			dir = unicode[eor].direction(); status.eor = dir;
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
			    dir = unicode[eor].direction(); status.eor = dir;
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




ScriptItemArray::~ScriptItemArray()
{
    free( d );
}


void ScriptItemArray::itemize( const QRTString &string )
{
    if ( !d ) {
	int size = string.formats.numFormats() + 1;
	d = (ScriptItemArrayPrivate *)malloc( sizeof( ScriptItemArrayPrivate ) +
					      sizeof( ScriptItem ) * size );
	d->alloc = size;
	d->size = 0;
    }

    bidiItemize( string.string, *this, QChar::DirON, &string.formats );
}


void ScriptItemArray::itemize( const QString &string )
{
    if ( !d ) {
	int size = 1;
	d = (ScriptItemArrayPrivate *)malloc( sizeof( ScriptItemArrayPrivate ) +
					      sizeof( ScriptItem ) * size );
	d->alloc = size;
	d->size = 0;
    }

    bidiItemize( string, *this, QChar::DirON, 0 );
}


void ScriptItemArray::resize( int s )
{
    int alloc = (s + 8) >> 3 << 3;
    d = (ScriptItemArrayPrivate *)realloc( d, sizeof( ScriptItemArrayPrivate ) +
		 sizeof( ScriptItem ) * alloc );
    d->alloc = alloc;
}
