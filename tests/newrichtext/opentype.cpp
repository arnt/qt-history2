#include "opentype.h"
#include "opentype/ftxopen.h"

#include <qglobal.h>
#include "qtextlayout.h"
#include "qfont.h"

static inline void tag_to_string( char *string, FT_ULong tag )
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

#define DefaultLangSys 0xffff
#define DefaultScript FT_MAKE_TAG( 'D', 'F', 'L', 'T' )

struct Features {
    FT_ULong tag;
    unsigned short bit;
};

// GPOS features are always applied. We only have to note the GSUB features that should not get
// applied in all cases here.

const Features standardFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'l', 'i', 'g', 'a' ), 0x8000 },
    { FT_MAKE_TAG( 'c', 'l', 'i', 'g' ), 0x8000 },
    { 0,  0 }
};

// always keep in sync with Shape enum in scriptenginearabic.cpp
const Features arabicFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), 0x01 },
    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), 0x02 },
    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), 0x04 },
    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), 0x08 },
    { FT_MAKE_TAG( 'r', 'l', 'i', 'g' ), 0x4000 },
    { FT_MAKE_TAG( 'c', 'a', 'l', 't' ), 0x8000 },
    { FT_MAKE_TAG( 'l', 'i', 'g', 'a' ), 0x4000 },
    { FT_MAKE_TAG( 'd', 'l', 'i', 'g' ), 0x8000 },
    // mset is used in old Win95 fonts that don't have a 'mark' positioning table.
    { FT_MAKE_TAG( 'm', 's', 'e', 't' ), 0x8000 },
    { 0,  0 }
};

const Features syriacFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x8000 },
    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), 0x01 },
    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), 0x02 },
    { FT_MAKE_TAG( 'f', 'i', 'n', '2' ), 0x02 },
    { FT_MAKE_TAG( 'f', 'i', 'n', '3' ), 0x02 },
    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), 0x04 },
    { FT_MAKE_TAG( 'm', 'e', 'd', '2' ), 0x04 },
    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), 0x08 },
    { FT_MAKE_TAG( 'r', 'l', 'i', 'g' ), 0x4000 },
    { FT_MAKE_TAG( 'c', 'a', 'l', 't' ), 0x8000 },
    { FT_MAKE_TAG( 'l', 'i', 'g', 'a' ), 0x8000 },
    { FT_MAKE_TAG( 'd', 'l', 'i', 'g' ), 0x8000 },
    { 0,  0 }
};

const Features devanagariFeatures[] = {
    // Language based forms
    { FT_MAKE_TAG( 'n', 'u', 'k', 't' ), 0x0001 },
    { FT_MAKE_TAG( 'a', 'k', 'h', 'n' ), 0x0002 },
    { FT_MAKE_TAG( 'r', 'p', 'h', 'f' ), 0x0004 },
    { FT_MAKE_TAG( 'b', 'l', 'w', 'f' ), 0x0008 },
    { FT_MAKE_TAG( 'h', 'a', 'l', 'f' ), 0x0010 },
    { FT_MAKE_TAG( 'p', 's', 'b', 'f' ), 0x0020 },
    { FT_MAKE_TAG( 'v', 'a', 't', 'u' ), 0x0040 },
    // Conjunkts and typographical forms
    { FT_MAKE_TAG( 'p', 'r', 'e', 's' ), 0x0080 },
    { FT_MAKE_TAG( 'b', 'l', 'w', 's' ), 0x0100 },
    { FT_MAKE_TAG( 'a', 'b', 'v', 's' ), 0x0200 },
    { FT_MAKE_TAG( 'p', 's', 't', 's' ), 0x0400 },
    // halant forms
    { FT_MAKE_TAG( 'h', 'a', 'l', 'n' ), 0x0800 },
    { 0,  0 }
};



struct SupportedScript {
    FT_ULong tag;
    const Features *features;
    unsigned short required_bits;
    unsigned short always_apply;
};


const SupportedScript supported_scripts [] = {
// 	// European Alphabetic Scripts
// 	Latin,
    { FT_MAKE_TAG( 'l', 'a', 't', 'n' ), standardFeatures, 0x0000, 0x8000 },
// 	Greek,
    { FT_MAKE_TAG( 'g', 'r', 'e', 'k' ), standardFeatures, 0x0000, 0x8000 },
// 	Cyrillic,
    { FT_MAKE_TAG( 'c', 'y', 'r', 'l' ), standardFeatures, 0x0000, 0x8000 },
// 	Armenian,
        { FT_MAKE_TAG( 'a', 'r', 'm', 'n' ), standardFeatures, 0x0000, 0x8000 },
// 	Georgian,
    { FT_MAKE_TAG( 'g', 'e', 'o', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Runic,
    { FT_MAKE_TAG( 'r', 'u', 'n', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Ogham,
    { FT_MAKE_TAG( 'o', 'g', 'a', 'm' ), standardFeatures, 0x0000, 0x8000 },
// 	SpacingModifiers,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	CombiningMarks,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },

// 	// Middle Eastern Scripts
// 	Hebrew,
    { FT_MAKE_TAG( 'h', 'e', 'b', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Arabic,
    { FT_MAKE_TAG( 'a', 'r', 'a', 'b' ), arabicFeatures, 0x400e, 0xc000 },
// 	Syriac,
    { FT_MAKE_TAG( 's', 'y', 'r', 'c' ), syriacFeatures, 0x400e, 0xc000 },
// 	Thaana,
    { FT_MAKE_TAG( 't', 'h', 'a', 'a' ), standardFeatures, 0x0000, 0x8000 },

// 	// South and Southeast Asian Scripts
// 	Devanagari,
    // #### MS Mangal doens't have pstf. Required for Devanagari?
    { FT_MAKE_TAG( 'd', 'e', 'v', 'a' ), devanagariFeatures, 0x0fdf, 0x8000 },
// 	Bengali,
    { FT_MAKE_TAG( 'b', 'e', 'n', 'g' ), standardFeatures, 0x0000, 0x8000 },
// 	Gurmukhi,
    { FT_MAKE_TAG( 'g', 'u', 'r', 'u' ), standardFeatures, 0x0000, 0x8000 },
// 	Gujarati,
    { FT_MAKE_TAG( 'g', 'u', 'j', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Oriya,
    { FT_MAKE_TAG( 'o', 'r', 'y', 'a' ), standardFeatures, 0x0000, 0x8000 },
// 	Tamil,
    { FT_MAKE_TAG( 't', 'a', 'm', 'l' ), standardFeatures, 0x0000, 0x8000 },
// 	Telugu,
    { FT_MAKE_TAG( 't', 'e', 'l', 'u' ), standardFeatures, 0x0000, 0x8000 },
// 	Kannada,
    { FT_MAKE_TAG( 'k', 'n', 'd', 'a' ), standardFeatures, 0x0000, 0x8000 },
// 	Malayalam,
    { FT_MAKE_TAG( 'm', 'l', 'y', 'm' ), standardFeatures, 0x0000, 0x8000 },
// 	Sinhala,
    { FT_MAKE_TAG( 's', 'i', 'n', 'h' ), standardFeatures, 0x0000, 0x8000 },
// 	Thai,
    { FT_MAKE_TAG( 't', 'h', 'a', 'i' ), standardFeatures, 0x0000, 0x8000 },
// 	Lao,
    { FT_MAKE_TAG( 'l', 'a', 'o', ' ' ), standardFeatures, 0x0000, 0x8000 },
// 	Tibetan,
    { FT_MAKE_TAG( 't', 'i', 'b', 't' ), standardFeatures, 0x0000, 0x8000 },
// 	Myanmar,
    { FT_MAKE_TAG( 'm', 'y', 'm', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	Khmer,
    { FT_MAKE_TAG( 'k', 'h', 'm', 'r' ), standardFeatures, 0x0000, 0x8000 },

// 	// East Asian Scripts
// 	Han,
    { FT_MAKE_TAG( 'h', 'a', 'n', 'i' ), standardFeatures, 0x0000, 0x8000 },
// 	Hiragana,
    { FT_MAKE_TAG( 'k', 'a', 'n', 'a' ), standardFeatures, 0x0000, 0x8000 },
// 	Katakana,
    { FT_MAKE_TAG( 'k', 'a', 'n', 'a' ), standardFeatures, 0x0000, 0x8000 },
// 	Hangul,
    { FT_MAKE_TAG( 'h', 'a', 'n', 'g' ), standardFeatures, 0x0000, 0x8000 },
// 	Bopomofo,
    { FT_MAKE_TAG( 'b', 'o', 'p', 'o' ), standardFeatures, 0x0000, 0x8000 },
// 	Yi,
    { FT_MAKE_TAG( 'y', 'i', ' ', ' ' ), standardFeatures, 0x0000, 0x8000 },

// 	// Additional Scripts
// 	Ethiopic,
    { FT_MAKE_TAG( 'e', 't', 'h', 'i' ), standardFeatures, 0x0000, 0x8000 },
// 	Cherokee,
    { FT_MAKE_TAG( 'c', 'h', 'e', 'r' ), standardFeatures, 0x0000, 0x8000 },
// 	CanadianAboriginal,
    { FT_MAKE_TAG( 'c', 'a', 'n', 's' ), standardFeatures, 0x0000, 0x8000 },
// 	Mongolian,
    { FT_MAKE_TAG( 'm', 'o', 'n', 'g' ), standardFeatures, 0x0000, 0x8000 },

// 	// Symbols
// 	CurrencySymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	LetterlikeSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	NumberForms,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	MathematicalOperators,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	TechnicalSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	GeometricSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	MiscellaneousSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	EnclosedAndSquare,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 },
// 	Braille,
    { FT_MAKE_TAG( 'b', 'r', 'a', 'i' ), standardFeatures, 0x0000, 0x8000 },

//                Unicode, should be used
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), standardFeatures, 0x0000, 0x8000 }
    // ### where are these?
// 	FT_MAKE_TAG( 'b', 'y', 'z', 'm' ),
//     FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
    // ### Hangul Jamo
//     FT_MAKE_TAG( 'j', 'a', 'm', 'o' ),
};


bool OpenTypeIface::loadTables( FT_ULong script)
{
    always_apply = 0;

    assert( script < QFont::Unicode );
    // find script in our list of supported scripts.
    const SupportedScript *s = supported_scripts + script;
    script = s->tag;

    FT_Error error = TT_GSUB_Select_Script( gsub, script, &script_index );
    if ( error ) {
	qDebug("could not select script %d: %d", (int)script, error );
	if ( s->tag == DefaultScript ) {
	    // try to load default language system
	    error = TT_GSUB_Select_Script( gsub, DefaultScript, &script_index );
	    if ( error )
		return FALSE;
	} else {
	    return FALSE;
	}
    }

//     qDebug("arabic is script %d", script_index );

    TTO_FeatureList featurelist = gsub->FeatureList;

    int numfeatures = featurelist.FeatureCount;

//     qDebug("table has %d features", numfeatures );


    found_bits = 0;
    for( int i = 0; i < numfeatures; i++ ) {
	TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	FT_ULong feature = r->FeatureTag;
	const Features *f = s->features;
	while ( f->tag ) {
	    if ( f->tag == feature ) {
		found_bits |= f->bit;
		break;
	    }
	    f++;
	}
	FT_UShort feature_index;
	TT_GSUB_Select_Feature( gsub, f->tag, script_index, DefaultLangSys,
				&feature_index );
	TT_GSUB_Add_Feature( gsub, feature_index, f->bit );

	char featureString[5];
	tag_to_string( featureString, r->FeatureTag );
	qDebug("found feature '%s' in GSUB table", featureString );
	qDebug("setting bit %x for feature, feature_index = %d", f->bit, feature_index );
    }
    if ( hasGPos ) {
	FT_UShort script_index;
	error = TT_GPOS_Select_Script( gpos, script, &script_index );
	if ( error ) {
// 	    qDebug("could not select arabic script in gpos table: %d", error );
	    return TRUE;
	}

	TTO_FeatureList featurelist = gpos->FeatureList;

	int numfeatures = featurelist.FeatureCount;

// 	qDebug("table has %d features", numfeatures );

	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    FT_UShort feature_index;
	    TT_GPOS_Select_Feature( gpos, r->FeatureTag, script_index, 0xffff, &feature_index );
	    TT_GPOS_Add_Feature( gpos, feature_index, s->always_apply );

	    char featureString[5];
	    tag_to_string( featureString, r->FeatureTag );
	    qDebug("found feature '%s' in GPOS table", featureString );
	}


    }
    if ( found_bits & s->required_bits != s->required_bits ) {
	qDebug( "not all required features for script found! found_bits=%x", found_bits );
	TT_GSUB_Clear_Features( gsub );
	return FALSE;
    }
//     qDebug("found_bits = %x",  (uint)found_bits );

    always_apply = s->always_apply;
    current_script = script;

    return TRUE;
}


OpenTypeIface::OpenTypeIface( FT_Face _face )
    : face( _face ), gdef( 0 ), gsub( 0 ), gpos( 0 ), current_script( 0 )
{
    hasGDef = hasGSub = hasGPos = TRUE;
}

bool OpenTypeIface::supportsScript( unsigned int script )
{
    if ( current_script == supported_scripts[script].tag )
	return TRUE;

    char featureString[5];
    tag_to_string( featureString, supported_scripts[script].tag );
    qDebug("trying to load tables for script %d (%s))", script, featureString);

    FT_Error error;
    if ( !gdef ) {
	if ( (error = TT_Load_GDEF_Table( face, &gdef )) ) {
	    qDebug("error loading gdef table: %d", error );
	    hasGDef = FALSE;
	    return FALSE;
	}
    }

    if ( !gsub ) {
	if ( (error = TT_Load_GSUB_Table( face, &gsub, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
		qDebug("error loading gsub table: %d", error );
		return FALSE;
	    } else {
		qDebug("face doesn't have a gsub table" );
		hasGSub = FALSE;
	    }
	}
    }

    if ( !gpos ) {
	if ( (error = TT_Load_GPOS_Table( face, &gpos, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
		qDebug("error loading gpos table: %d", error );
		return FALSE;
	    } else {
		qDebug("face doesn't have a gpos table" );
		hasGPos = FALSE;
	    }
	}
    }

    if ( loadTables( script ) ) {
	return TRUE;
    }
    return FALSE;
}

bool OpenTypeIface::applyGlyphSubstitutions( unsigned int script, ShapedItem *shaped, unsigned short *featuresToApply )
{
    if ( current_script != supported_scripts[script].tag ) {
	TT_GSUB_Clear_Features( gsub );

	if ( !loadTables( script ) )
	    return FALSE;
    }

    for ( int i = 0; i < shaped->d->num_glyphs; i++ ) {
	featuresToApply[i] |= always_apply;
    }


    TTO_GSUB_String *in = 0;
    TTO_GSUB_String *out = 0;

    TT_GSUB_String_New( face->memory, &in );
    TT_GSUB_String_Set_Length( in, shaped->d->num_glyphs );
    TT_GSUB_String_New( face->memory, &out);
    TT_GSUB_String_Set_Length( out, shaped->d->num_glyphs );
    out->length = 0;

    for ( int i = 0; i < shaped->d->num_glyphs; i++) {
      in->string[i] = shaped->d->glyphs[i];
      in->logClusters[i] = i;
      in->properties[i] = ~featuresToApply[i];
//        qDebug("    glyph[%d] = %x apply=%x, logcluster=%d", i, shaped->d->glyphs[i], featuresToApply[i], i );
    }
    in->max_ligID = 0;

    TT_GSUB_Apply_String (gsub, in, out);

    if ( shaped->d->num_glyphs < (int)out->length ) {
	shaped->d->glyphs = ( GlyphIndex *) realloc( shaped->d->glyphs, out->length*sizeof(GlyphIndex) );
    }
    shaped->d->num_glyphs = out->length;

//     qDebug("out: num_glyphs = %d", shaped->d->num_glyphs );
    GlyphAttributes *oldAttrs = shaped->d->glyphAttributes;
    shaped->d->glyphAttributes = ( GlyphAttributes *) malloc( out->length*sizeof(GlyphAttributes) );

    int clusterStart = 0;
    int oldlc = 0;
    for ( int i = 0; i < shaped->d->num_glyphs; i++ ) {
	shaped->d->glyphs[i] = out->string[i];
	int lc = out->logClusters[i];
	shaped->d->glyphAttributes[i] = oldAttrs[lc];
	if ( !shaped->d->glyphAttributes[i].mark && lc != oldlc ) {
	    for ( int j = oldlc; j < lc; j++ )
		shaped->d->logClusters[j] = clusterStart;
	    clusterStart = i;
	    oldlc = lc;
	}
//   	qDebug("    glyph[%d]=%4x logcluster=%d mark=%d", i, out->string[i], out->logClusters[i], shaped->d->glyphAttributes[i].mark );
	// ### need to fix logclusters aswell!!!!
    }
    for ( int j = oldlc; j < shaped->d->length; j++ )
	shaped->d->logClusters[j] = shaped->d->num_glyphs-1;

    free( oldAttrs );

    TT_GSUB_String_Done( in );

    // we need to keep this one around for shaping
    if ( hasGPos )
	shaped->d->enginePrivate = (void *)out;
    else
	TT_GSUB_String_Done( out );
    return TRUE;
}


bool OpenTypeIface::applyGlyphPositioning( unsigned int script, ShapedItem *shaped )
{
    TTO_GSUB_String *in = (TTO_GSUB_String *)shaped->d->enginePrivate;
    TTO_GPOS_Data *out = 0;

    bool retval = FALSE;
    if ( hasGPos ) {
	retval = TRUE;

	if ( current_script != supported_scripts[script].tag ) {
	    TT_GSUB_Clear_Features( gsub );

	    if ( !loadTables( script ) )
		return FALSE;
	}


	bool reverse = (shaped->d->analysis.bidiLevel % 2);
	// ### is FT_LOAD_DEFAULT the right thing to do?
	TT_GPOS_Apply_String( face, gpos, FT_LOAD_DEFAULT, in, &out, FALSE, reverse );

	Offset *advances = shaped->d->advances;
	Offset *offsets = shaped->d->offsets;

	//     qDebug("positioned glyphs:" );
	for ( int i = 0; i < shaped->d->num_glyphs; i++) {
	    // 	qDebug("    %d:\tadv=(%d/%d)\tpos=(%d/%d)\tback=%d\tnew_advance=%d", i,
	    // 	       (int)(out[i].x_advance >> 6), (int)(out[i].y_advance >> 6 ),
	    // 	       (int)(out[i].x_pos >> 6 ), (int)(out[i].y_pos >> 6),
	    // 	       out[i].back, out[i].new_advance );
	    if ( out[i].new_advance ) {
		advances[i].x = out[i].x_advance >> 6;
		advances[i].y = -out[i].y_advance >> 6;
	    } else {
		advances[i].x += out[i].x_advance >> 6;
		advances[i].y -= out[i].y_advance >> 6;
	    }
	    offsets[i].x = out[i].x_pos >> 6;
	    offsets[i].y = -(out[i].y_pos >> 6);
	    int back = out[i].back;
	    while ( back ) {
		offsets[i].x -= advances[i-back].x;
		offsets[i].y -= advances[i-back].y;
		back--;
	    }
	    // 	qDebug("   ->\tadv=(%d/%d)\tpos=(%d/%d)",
	    // 	       advances[i].x, advances[i].y, offsets[i].x, offsets[i].y );
	}
    }

    if ( in )
	TT_GSUB_String_Done( in );
    shaped->d->enginePrivate = 0;
    free( out );

    return retval;
}
