#include "scriptenginedevanagari.h"
#include "opentype.h"
#include "qfont.h"
#include "qtextdata.h"

enum Features {
    NuktaFeature = 0x0001,
    AkhantFeature = 0x0002,
    RephFeature = 0x0004,
    BelowFormFeature = 0x0008,
    HalfFormFeature = 0x0010,
    PostFormFeature = 0x0020,
    VattuFeature = 0x0040,
    PreSubstFeature = 0x0080,
    BelowSubstFeature = 0x0100,
    AboveSubstFeature = 0x0200,
    PostSubstFeature = 0x0400,
    HalantFeature = 0x0800
};

enum Form {
    Invalid = 0x0,
    Unknown = Invalid,
    Consonant,
    Nukta,
    Halant,
    Matra,
    VowelMark,
    StressMark,
    IndependentVowel,
    LengthMark,
    Other,
};

static int devanagariForms[0x80] = {
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,

    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, StressMark, StressMark, StressMark,
    StressMark, Unknown, Unknown, Unknown,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Other, Other, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other
};

Form form( const QChar &ch ) {
    ushort uc = ch.unicode();
    if ( uc < 0x900 || uc > 0x97f ) {
	if ( uc == 0x25cc )
	    return Consonant;
	return Other;
    }
    return (Form)devanagariForms[uc-0x900];
}

static bool isRa( const QChar &ch ) {
    return (ch.unicode() == 0x930);
}

enum Position {
    None,
    Pre,
    Above,
    Below,
    Post
};

static int devanagariPosition[0x80] = {
    None, Above, Above, Post,
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
    None, None, Post, Pre,

    Post, Below, Below, Below,
    Below, Above, Above, Above,
    Above, Post, Post, Post,
    Post, None, None, None,

    None, Above, Below, Above,
    Above, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
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
    if ( uc < 0x900 && uc > 0x97f )
	return None;
    return (Position) devanagariPosition[uc-0x900];
}

/* syllables are of the form:

   (Consonant Nukta? Halant)* Consonant Matra? VowelMark? StressMark?
   (Consonant Nukta? Halant)* Consonant Halant
   IndependentVowel VowelMark? StressMark?

   // ### check the above is correct

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
	    if ( state == Nukta || state == Consonant )
		break;
	    goto finish;
	case Nukta:
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
	    if ( state == Consonant || state == Nukta )
		break;
	    goto finish;

	case LengthMark:
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

    for ( int i = 0; i < len; i++ )
	featuresToApply[i] = 0;

    // nothing to do in this case!
    if ( len == 1 ) {
	attributes[0].mark = isMark( reordered.unicode()[0] );
	attributes[0].clusterStart = FALSE;
	return reordered;
    }

    int base = 0;
    if ( form( *uc ) == Consonant ) {
	bool reph = FALSE;
	if ( len > 2 && isRa( uc[0] ) && form( uc[1] ) == Halant ) {
	    reph = TRUE;
// 	    qDebug("Reph");
	}

	// Rule 1: find base consonant
	int lastConsonant = 0;
	for ( int i = len-1; i > 0; i-- ) {
	    if ( form( uc[i] ) == Consonant ) {
		if ( !lastConsonant )
		    lastConsonant = i;
		// ### The MS specs says, that this should be done only if the syllable starts with a reph,
		// but they seem to act differently.
		if ( /*!reph ||*/ !isRa( uc[i] ) ) {
		    base = i;
		    break;
		}
	    }
	}
	if ( reph && base == 0 )
	    base = lastConsonant;

// 	qDebug("base consonant at %d skipped=%s", base, lastConsonant != base ? "true" :"false" );

	// Rule 2: move halant of base consonant to last one. Only
	// possible if we skipped consonants while finding the base
	if ( lastConsonant != base && form( uc[base+1] ) == Halant ) {
// 	    qDebug("moving halant from %d to %d!", base+1, lastConsonant);
	    QChar halant = uc[base+1];
	    for ( int i = base+1; i < lastConsonant; i++ )
		uc[i] = uc[i+1];
	    uc[lastConsonant] = halant;

	}

	// Rule 3: Move reph to follow post base matra
	if ( reph ) {
	    int toPos = base+1;
	    if ( toPos < len && form( uc[toPos] ) == Matra )
		toPos++;
// 	    qDebug("moving reph from %d to %d", 0, toPos );
	    QChar ra = uc[0];
	    QChar halant = uc[1];
	    for ( int i = 2; i < toPos; i++ )
		uc[i-2] = uc[i];
	    uc[toPos-2] = ra;
	    uc[toPos-1] = halant;
	    featuresToApply[toPos-2] |= RephFeature;
	    featuresToApply[toPos-1] |= RephFeature;
	    base -= 2;
	}
    }

    // Rule 4: split two part matras into parts
    // doesn't apply for devanagari

    // Rule 5: identify matra position. there are no post/below base consonats
    // in devanagari except for [Ra Halant]_Vattu, but these are already at the
    // right position

    // all reordering happens now to the chars after (base+(reph halant)_vattu?)
    // so we move base to there
    int fixed = base;
    if ( fixed < len - 2 && isRa( uc[fixed+1] ) && form( uc[fixed+2] == Halant ) )
	fixed += 2;


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

    fixed++;
    int toMove = 0;
    while ( fixed < len ) {
	//        qDebug("fixed = %d", fixed );
	for ( int i = fixed; i < len; i++ ) {
	    if ( form( uc[i] ) == finalOrder[toMove].form &&
		 position( uc[i] ) == finalOrder[toMove].position ) {
		// need to move this glyph
		int to = fixed;
		if ( finalOrder[toMove].position == Pre )
		    to = 0;
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
	featuresToApply[0] |= AkhantFeature|NuktaFeature;

    for ( int i = 1; i < len; i++ ) {
	int newState = form( uc[i] );
	switch( newState ) {
	case Consonant:
	    lastWasBase = (i == base);
	    featuresToApply[i] |= AkhantFeature|NuktaFeature;
	    break;
	case Halant:
	    if ( state == Nukta || state == Consonant ) {
		// vattu or halant feature
		if ( isRa( uc[i-1] ) && len > 2 ) {
		    if ( !(featuresToApply[i] & RephFeature) ) {
			featuresToApply[i-1] |= BelowFormFeature|VattuFeature;
			featuresToApply[i] |= BelowFormFeature|VattuFeature;
			int j = i-2;
			while ( j >= 0 ) {
			    int f = form( uc[j] );
			    featuresToApply[j] |= VattuFeature;
			    if ( f == Consonant )
				break;
			    j--;
			}
		    }
		}
		else if ( !lastWasBase  ) {
		    if ( state == Nukta )
			featuresToApply[i-2] |= HalfFormFeature;
		    featuresToApply[i-1] |= HalfFormFeature;
		    featuresToApply[i] |= HalfFormFeature;
		}
	    }
	    break;
	case Nukta:
	    if ( state == Consonant ) {
		featuresToApply[i-1] |= NuktaFeature;
		featuresToApply[i] |= NuktaFeature;
	    }
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
	attributes[i].mark = isMark( reordered.unicode()[i] );
	attributes[i].clusterStart = FALSE;
    }
    attributes[0].clusterStart = TRUE;

//     qDebug("reordered:");
//     for ( int i = 0; i < (int)reordered.length(); i++ )
// 	qDebug("    %d: %4x apply=%4x clusterStart=%d", i, reordered[i].unicode(), featuresToApply[i], attributes[i].clusterStart );

    return reordered;
}

static QString analyzeSyllables( const ShapedItem *shaped, unsigned short *featuresToApply,
				 GlyphAttributes *attributes ) {
    QString reordered;
    ShapedItemPrivate *d = shaped->d;
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


void ScriptEngineDevanagari::shape( ShapedItem *result )
{
//     qDebug("ScriptEngineDevanagari::shape()");

    ShapedItemPrivate *d = result->d;

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

    OpenTypeIface *openType = result->d->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( QFont::Devanagari ) ) {
	openTypeShape( QFont::Devanagari, openType, result, reordered, featuresToApply );
    } else {
	ScriptEngineBasic::shape( result );
    }

    if ( result->d->length > 127 )
	delete featuresToApply;
}


void ScriptEngineDevanagari::position( ShapedItem *result )
{
    OpenTypeIface *openType = result->d->fontEngine->openTypeIface();

    if ( openType && openType->supportsScript( QFont::Devanagari ) ) {
	openTypePosition( QFont::Devanagari, openType, result );
	return;
    }

    ScriptEngineBasic::position( result );
}

void ScriptEngineDevanagari::openTypeShape( int script, const OpenTypeIface *openType, ShapedItem *result, const QString &reordered, unsigned short *featuresToApply )
{
//     qDebug("ScriptEngineDevanagari::openTypeShape()");
    ShapedItemPrivate *d = result->d;
    int len = d->num_glyphs;

    d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
    int error = d->fontEngine->stringToCMap( reordered.unicode(), len, d->glyphs, &d->num_glyphs );
    if ( error == FontEngineIface::OutOfMemory ) {
	d->glyphs = (GlyphIndex *)realloc( d->glyphs, d->num_glyphs*sizeof( GlyphIndex ) );
	d->fontEngine->stringToCMap( reordered.unicode(), len, d->glyphs, &d->num_glyphs );
    }

#if 0
    qDebug("before shaping: glyph attributes:" );
    for ( int i = 0; i < result->d->num_glyphs; i++) {
	qDebug("   ->\tmark=%d",
	       result->d->glyphAttributes[i].mark );
    }
#endif

    ((OpenTypeIface *) openType)->applyGlyphSubstitutions( script, result, featuresToApply );

}


void ScriptEngineDevanagari::openTypePosition( int script, const OpenTypeIface *openType, ShapedItem *result )
{
    ShapedItemPrivate *d = result->d;
    d->offsets = (Offset *) realloc( d->offsets, d->num_glyphs * sizeof( Offset ) );
    memset( d->offsets, 0, d->num_glyphs * sizeof( Offset ) );
    d->advances = (Offset *) realloc( d->advances, d->num_glyphs * sizeof( Offset ) );
    d->ascent = d->fontEngine->ascent();
    d->descent = d->fontEngine->descent();
    for ( int i = 0; i < d->num_glyphs; i++ ) {
	QGlyphInfo gi = d->fontEngine->boundingBox( d->glyphs[i] );
	d->advances[i].x = gi.xoff;
	d->advances[i].y = gi.yoff;
	// #### not quite correct! should be done after glyph positioning!
	int y = d->offsets[i].y + gi.y;
	d->ascent = QMAX( d->ascent, -y );
	d->descent = QMAX( d->descent, y + gi.height );
    }

    bool positioned = ((OpenTypeIface *) openType)->applyGlyphPositioning( script, result );
#if 0
    qDebug("after positoning: glyph attributes:" );
    for ( int i = 0; i < result->d->num_glyphs; i++) {
	qDebug("   ->\tmark=%d",
	       result->d->glyphAttributes[i].mark );
    }
#endif
    if ( !positioned ) {
// 	qDebug("no open type positioning, using heuristics");
	heuristicPositionMarks( result );
    }


//     qDebug("logClusters:");
//     for ( int i = 0; i < result->d->length; i++ )
// 	qDebug("    %d -> %d", i, result->d->logClusters[i] );
}
