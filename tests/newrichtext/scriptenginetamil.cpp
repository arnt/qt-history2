#include "scriptenginetamil.h"
#include "opentype.h"
#include "qfont.h"
#include "qtextdata.h"

#define BASE 0xb80

enum Features {
    AkhantFeature = 0x0004,
    RephFeature = 0x0008,
    BelowFormFeature = 0x0010,
    HalfFormFeature = 0x0020,
    PostFormFeature = 0x0040,
    VattuFeature = 0x0080,
    PreSubstFeature = 0x0100,
    BelowSubstFeature = 0x0200,
    AboveSubstFeature = 0x0400,
    PostSubstFeature = 0x0800,
    HalantFeature = 0x1000
};

enum Form {
    Invalid = 0x0,
    Unknown = Invalid,
    Consonant,
    Halant,
    Matra,
    VowelMark,
    StressMark,
    IndependentVowel,
    LengthMark,
    Other,
};

static int tamilForms[0x80] = {
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Invalid, Invalid,
    Invalid, Consonant, Consonant, Invalid,
    Consonant, Invalid, Consonant, Consonant,

    Invalid, Invalid, Invalid, Consonant,
    Consonant, Invalid, Invalid, Invalid,
    Consonant, Consonant, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Invalid, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Invalid,
    Invalid, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other
};

static Form form( const QChar &ch ) {
    ushort uc = ch.unicode();
    if ( uc < BASE || uc > BASE+0x7f ) {
	if ( uc == 0x25cc )
	    return Consonant;
	return Other;
    }
    return (Form)tamilForms[uc-BASE];
}

enum Position {
    None,
    Pre,
    Above,
    Below,
    Post,
    Split
};

static int tamilPosition[0x80] = {
    None, None, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Post,

    Above, Below, Below, None,
    None, None, Pre, Pre,
    Pre, None, Split, Split,
    Split, Halant, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
};

static inline Position position( const QChar &ch ) {
    unsigned short uc = ch.unicode();
    if ( uc < BASE && uc > BASE+0x7f )
	return None;
    return (Position) tamilPosition[uc-BASE];
}

/* syllables are of the form:

   (Consonant  Halant)* Consonant Matra? VowelMark? StressMark?
   (Consonant Halant)* Consonant Halant
   IndependentVowel VowelMark? StressMark?

   We return syllable boundaries on invalid combinations aswell
*/
static int nextSyllableBoundary( const QString &s, int start, int end, bool *invalid )
{
    *invalid = FALSE;
//     qDebug("nextSyllableBoundary: start=%d, end=%d", start, end );
    const QChar *uc = s.unicode()+start;

    int pos = 0;
    Form state = form( uc[pos] );
//     qDebug("state[%d]=%d (uc=%4x)", pos, state, uc[pos].unicode() );
    pos++;

    if ( state != Consonant && state != IndependentVowel ) {
	if ( state != Other )
	    *invalid = TRUE;
	goto finish;
    }

    while ( pos < end - start ) {
	Form newState = form( uc[pos] );
// 	qDebug("state[%d]=%d (uc=%4x)", pos, newState, uc[pos].unicode() );
	switch( newState ) {
	case Consonant:
	    if ( state == Halant )
		break;
	    goto finish;
	case Halant:
	    if ( state == Consonant )
		break;
	    goto finish;
	case StressMark:
	    if ( state == VowelMark )
		break;
	    // fall through
	case VowelMark:
	    if ( state == Matra || state == IndependentVowel )
		break;
	    // fall through
	case Matra:
	    if ( state == Consonant )
		break;
#if 0
	    if ( state == IndependentVowel ) {
		// ####
		;
	    }
#endif
	    if ( state == Matra ) {
		if ( uc[pos-1].unicode() == 0x0bc6 &&
		     ( uc[pos].unicode() == 0xbbe || uc[pos].unicode() == 0xbd7 ) )
			break;
		if ( uc[pos-1].unicode() == 0x0bc7 && uc[pos].unicode() == 0xbbe )
			break;
	    }
	    goto finish;

	case LengthMark:
	    if ( state == Consonant || state == Matra ||
		 state == VowelMark || state == StressMark )
		break;
	case IndependentVowel:
	case Invalid:
	case Other:
	    goto finish;
	}
	state = newState;
	pos++;
    }
 finish:
    return pos+start;
}

// vowel matras that have to be split into two parts.
static const unsigned short split_o[2]  = { 0xbc6, 0xbbe };
static const unsigned short split_oo[2] = { 0xbc7, 0xbbe };
static const unsigned short split_au[2] = { 0xbc6, 0xbd7 };

static QString reorderSyllable( const QString &string, int start, int end, unsigned short *featuresToApply,
				GlyphAttributes *attributes, bool invalid )
{
    int len = end - start;

    QString reordered = string.mid( start, len );
    if ( invalid ) {
	reordered = QChar( 0x25cc ) + reordered;
	len++;
    }

    // in case mid() returns the whole string!
    reordered.setLength( reordered.length() );

    QChar *uc = (QChar *)reordered.unicode();

    // We can do this rule at the beginning, as it doesn't interact with later operations.
    // Rule 4: split two part matras into parts
    // This could be done better, but works for now.
    for ( int i = 0; i < len; i++ ) {
	const QChar *split = 0;
	if ( uc[i].unicode() == 0xbca )
	    split = (const QChar *)split_o;
	else if ( uc[i].unicode() == 0xbcb )
	    split = (const QChar *)split_oo;
	else if ( uc[i].unicode() == 0xbcc )
	    split = (const QChar *)split_au;
	if ( split ) {
	    reordered.replace( i, 1, split, 2 );
	    uc = (QChar *)reordered.unicode();
	    len++;
	    break;
	}
    }

    for ( int i = 0; i < len; i++ )
	featuresToApply[i] = 0;

    // nothing to do in this case!
    if ( len == 1 ) {
	attributes[0].mark = (category( reordered.unicode()[0] ) == QChar::Mark_NonSpacing);
	attributes[0].clusterStart = FALSE;
	return reordered;
    }

    int base = 0;
    if ( form( *uc ) == Consonant ) {

	// Rule 1: find base consonant
	for ( int i = len-1; i > 0; i-- ) {
	    if ( form( uc[i] ) == Consonant ) {
		base = i;
		break;
	    }
	}

// 	qDebug("base consonant at %d skipped=%s", base, lastConsonant != base ? "true" :"false" );

    }

    // Rule 5: identify matra position. there are no post/below base consonats
    // in devanagari except for [Ra Halant]_Vattu, but these are already at the
    // right position

    // all reordering happens now to the chars after (base+(reph halant)_vattu?)
    // so we move base to there
    int fixed = base+1;


    // we continuosly position the matras and vowel marks and increase the fixed
    // until we reached the end.
    static struct {
	Form form;
	Position position;
    } finalOrder [] = {
	{ Matra, Pre },
	{ Matra, Below },
	{ VowelMark, Below },
	{ StressMark, Below },
	{ Matra, Above },
	{ Matra, Post },
	{ Consonant, None },
	{ Halant, None },
	{ VowelMark, Above },
	{ StressMark, Above },
	{ VowelMark, Post },
	{ (Form)0, None }
    };

//     qDebug("base=%d fixed=%d", base, fixed );
    int toMove = 0;
    while ( fixed < len ) {
// 	qDebug("fixed = %d", fixed );
	for ( int i = fixed; i < len; i++ ) {
	    if ( form( uc[i] ) == finalOrder[toMove].form &&
		 position( uc[i] ) == finalOrder[toMove].position ) {
		// need to move this glyph
		int to = fixed;
		if ( finalOrder[toMove].position == Pre )
		    to = base;
// 		qDebug("moving from %d to %d", i,  to );
		QChar ch = uc[i];
		unsigned short feature = featuresToApply[i];
		for ( int j = i; j > to; j-- ) {
		    uc[j] = uc[j-1];
		    featuresToApply[j] = featuresToApply[j-1];
		}
		uc[to] = ch;
		switch( finalOrder[toMove].position ) {
		case Pre:
// 		    feature |= PreSubstFeature;
		    break;
		case Above:
// 		    feature |= AboveSubstFeature;
		    break;
		case Below:
		    feature |= BelowFormFeature;//|BelowSubstFeature;
		    break;
		case Post:
		    feature |= PostSubstFeature;//|PostFormFeature;
		    break;
		case None:
		    break;
		case Split:
		    break;
		}
		featuresToApply[to] = feature;
		fixed++;
	    }
	}
	toMove++;
	if ( finalOrder[toMove].form == 0 )
	    break;
    }

    bool halantForm = base < len-1 && (form( uc[base+1] ) == Halant);
    if ( halantForm ) {
	// #### we have to take care this doesn't get applied to Akhant ligatures,
	// but that's currently rather hard (without a bigger rewrite of the open type
	// API (ftx*.c)
	featuresToApply[base] |= HalantFeature;
	featuresToApply[base+1] |= HalantFeature;
    }

    // set the features we need to apply in OT
    int state = form( uc[0] );
    bool lastWasBase = (base == 0);
    if ( state == Consonant )
	featuresToApply[0] |= AkhantFeature;

    for ( int i = 1; i < len; i++ ) {
	int newState = form( uc[i] );
	switch( newState ) {
	case Consonant:
	    lastWasBase = (i == base);
	    featuresToApply[i] |= AkhantFeature;
	    break;
	case Halant:
	    if ( state == Consonant ) {
		if ( !lastWasBase  ) {
		    featuresToApply[i-1] |= HalfFormFeature;
		    featuresToApply[i] |= HalfFormFeature;
		}
	    }
	    break;
	case StressMark:
	case VowelMark:
	case Matra:
	case LengthMark:
	case IndependentVowel:
	case Invalid:
	case Other:
	    break;
	}
	state = newState;
    }

    for ( int i = 0; i < (int)reordered.length(); i++ ) {
	attributes[i].mark = (category( reordered.unicode()[0] ) == QChar::Mark_NonSpacing);
	attributes[i].clusterStart = FALSE;
    }
    attributes[0].clusterStart = TRUE;

//     qDebug("reordered:");
//     for ( int i = 0; i < (int)reordered.length(); i++ )
// 	qDebug("    %d: %4x apply=%4x clusterStart=%d", i, reordered[i].unicode(), featuresToApply[i], attributes[i].clusterStart );

    return reordered;
}

static QString analyzeSyllables( const QShapedItem *shaped, unsigned short *featuresToApply,
				 GlyphAttributes *attributes ) {
    QString reordered;
    QShapedItemPrivate *d = shaped->d;
    int sstart = d->from;
    int end = sstart + d->length;
    int fpos = 0;
    while ( sstart < end ) {
	bool invalid;
	int send = nextSyllableBoundary( d->string, sstart, end, &invalid );
// 	qDebug("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
// 	       invalid ? "true" : "false" );
	QString str = reorderSyllable( d->string, sstart, send, featuresToApply+fpos, attributes+fpos, invalid );
	reordered += str;
	fpos += str.length();

	sstart = send;
    }
    return reordered;
}


void QScriptEngineTamil::shape( QShapedItem *result )
{
//     qDebug("QScriptEngineDevanagari::shape()");

    QShapedItemPrivate *d = result->d;

    unsigned short fa[256];
    unsigned short *featuresToApply = fa;
    if ( d->length > 127 )
	featuresToApply = new unsigned short[ 2*d->length ];


    d->glyphAttributes = (GlyphAttributes *)realloc( d->glyphAttributes, d->length * 2 * sizeof( GlyphAttributes ) );

    QString reordered = analyzeSyllables( result, featuresToApply, d->glyphAttributes );
    d->num_glyphs = reordered.length();

    d->logClusters = (unsigned short *) realloc( d->logClusters, d->num_glyphs * sizeof( unsigned short ) );
    int pos = 0;
    for ( int i = 0; i < d->num_glyphs; i++ ) {
	if ( d->glyphAttributes[i].clusterStart )
	    pos = i;
	d->logClusters[i] = pos;
    }

    d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
    int error = d->fontEngine->stringToCMap( reordered.unicode(), d->num_glyphs, d->glyphs, &d->num_glyphs );
    if ( error == QFontEngineIface::OutOfMemory ) {
	d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
	d->fontEngine->stringToCMap( reordered.unicode(), d->num_glyphs, d->glyphs, &d->num_glyphs );
    }

    QOpenType *openType = result->d->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( QFont::Tamil ) ) {
	((QOpenType *) openType)->apply( QFont::Tamil, result, featuresToApply );
	d->isPositioned = TRUE;
    } else {
	heuristicSetGlyphAttributes( result );
    }

    if ( result->d->length > 127 )
	delete featuresToApply;
}


void QScriptEngineTamil::position( QShapedItem *result )
{
    if ( result->d->isPositioned )
	return;

    calculateAdvances( result );
    result->d->isPositioned = TRUE;
}
