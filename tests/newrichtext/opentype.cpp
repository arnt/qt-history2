#include "opentype.h"
#include "opentype/ftxopen.h"

#include <qglobal.h>
#include "qtextlayout.h"


static inline void tag_to_string( char *string, FT_ULong tag )
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

#define DefaultLangSys 0xffff

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



struct SupportedScript {
    FT_ULong tag;
    const Features *features;
    unsigned short required_bits;
    unsigned short always_apply;
};

static const SupportedScript supported_scripts[] = {
    { OpenTypeIface::Arabic, arabicFeatures, 0x400e, 0xc000 },

    // Indic scripts, still unsupported:

// 	Bengali = FT_MAKE_TAG( 'b', 'e', 'n', 'g' ),
// 	Devanagari = FT_MAKE_TAG( 'd', 'e', 'v', 'a' ),
// 	Gujarati = FT_MAKE_TAG( 'g', 'u', 'j', 'r' ),
// 	Gurmukhi = FT_MAKE_TAG( 'g', 'u', 'r', 'u' ),
// 	Kannada = FT_MAKE_TAG( 'k', 'n', 'd', 'a' ),
// 	Tamil = FT_MAKE_TAG( 't', 'a', 'm', 'l' ),
// 	Telugu = FT_MAKE_TAG( 't', 'e', 'l', 'u' ),
// 	Oriya = FT_MAKE_TAG( 'o', 'r', 'y', 'a' ),

    // other unsupported:

// 	Khmer = FT_MAKE_TAG( 'k', 'h', 'm', 'r' ),
// 	Tibetan = FT_MAKE_TAG( 't', 'i', 'b', 't' ),

    // these have no open type engine according to MS. Usually the default language system will
    // do. We however need to make sure default substitutions in HAN happen according to
    // the correct language.

// 	Bopomofo = FT_MAKE_TAG( 'b', 'o', 'p', 'o' ),
// 	Braille = FT_MAKE_TAG( 'b', 'r', 'a', 'i' ),
// 	ByzantineMusic = FT_MAKE_TAG( 'b', 'y', 'z', 'm' ),
// 	CanadianSyllabics = FT_MAKE_TAG( 'c', 'a', 'n', 's' ),
// 	Cherokee = FT_MAKE_TAG( 'c', 'h', 'e', 'r' ),
// 	Han = FT_MAKE_TAG( 'h', 'a', 'n', 'i' ),
// 	Ethiopic = FT_MAKE_TAG( 'e', 't', 'h', 'i' ),
// 	HangulJamo = FT_MAKE_TAG( 'j', 'a', 'm', 'o' ),
// 	Hangul = FT_MAKE_TAG( 'h', 'a', 'n', 'g' ),

// 	Hiragana = FT_MAKE_TAG( 'k', 'a', 'n', 'a' ),
// 	Katakana = FT_MAKE_TAG( 'k', 'a', 'n', 'a' ),
// 	Malayalam = FT_MAKE_TAG( 'm', 'l', 'y', 'm' ),
// 	Mongolian = FT_MAKE_TAG( 'm', 'o', 'n', 'g' ),
// 	Myanmar = FT_MAKE_TAG( 'm', 'y', 'm', 'r' ),
// 	Sinhala = FT_MAKE_TAG( 's', 'i', 'n', 'h' ),
    { OpenTypeIface::Syriac, syriacFeatures, 0x400e, 0xc000 },
// 	Yi = FT_MAKE_TAG( 'y', 'i', ' ', ' ' ),

    // standard scripts:

// 	Armenian = FT_MAKE_TAG( 'a', 'r', 'm', 'n' ),
// 	Cyrillic = FT_MAKE_TAG( 'c', 'y', 'r', 'l' ),
// 	Default = FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	Georgian = FT_MAKE_TAG( 'g', 'e', 'o', 'r' ),
// 	Greek = FT_MAKE_TAG( 'g', 'r', 'e', 'k' ),
// 	Latin = FT_MAKE_TAG( 'l', 'a', 't', 'n' ),
// 	Ogham = FT_MAKE_TAG( 'o', 'g', 'a', 'm' ),
// 	Runic = FT_MAKE_TAG( 'r', 'u', 'n', 'r' ),

    // MS has a special engine for these, for now the standard one should be able to deal with them

// 	Hebrew = FT_MAKE_TAG( 'h', 'e', 'b', 'r' ),
// 	Lao = FT_MAKE_TAG( 'l', 'a', 'o', '' ),
// 	Thaana = FT_MAKE_TAG( 't', 'h', 'a', 'a' ),
// 	Thai = FT_MAKE_TAG( 't', 'h', 'a', 'i' ),
    { OpenTypeIface::Default, standardFeatures, 0x8000, 0x8000 }
};

bool OpenTypeIface::loadTables( FT_ULong script)
{
    always_apply = 0;

    // find script in our list of supported scripts.
    const SupportedScript *s = supported_scripts;
    while ( s->tag != OpenTypeIface::Default ) {
	if ( s->tag == script )
	    break;
	++s;
    }

    FT_Error error = TT_GSUB_Select_Script( gsub, script, &script_index );
    if ( error ) {
	qDebug("could not select script %d: %d", (int)script, error );
	if ( s->tag == OpenTypeIface::Default ) {
	    // try to load default language system
	    error = TT_GSUB_Select_Script( gsub, Default, &script_index );
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

// 	char featureString[5];
// 	tag_to_string( featureString, r->FeatureTag );
// 	qDebug("found feature '%s' in GSUB table", featureString );
// 	qDebug("setting bit %x for feature, feature_index = %d", f->bit, feature_index );
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
// 	    qDebug("found feature '%s' in GPOS table", featureString );
	}


    }
    if ( found_bits & s->required_bits == s->required_bits ) {
	qDebug( "not all required features for arabic found! found_bits=%x", found_bits );
	TT_GSUB_Clear_Features( gsub );
	return FALSE;
    }
//     qDebug("found_bits = %x",  (uint)found_bits );

    always_apply = s->always_apply;

    return TRUE;
}


OpenTypeIface::OpenTypeIface( FT_Face _face )
    : face( _face ), gdef( 0 ), gsub( 0 ), gpos( 0 ), current_script( 0 )
{
    hasGDef = hasGSub = hasGPos = TRUE;
}

bool OpenTypeIface::supportsScript( unsigned int script )
{
    if ( current_script == script )
	return TRUE;

    FT_Error error;
    if ( !gdef ) {
	if ( (error = TT_Load_GDEF_Table( face, &gdef )) ) {
// 	    qDebug("error loading gdef table: %d", error );
	    hasGDef = FALSE;
	    return FALSE;
	}
    }

    if ( !gsub ) {
	if ( (error = TT_Load_GSUB_Table( face, &gsub, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
// 		qDebug("error loading gsub table: %d", error );
		return FALSE;
	    } else {
// 		qDebug("face doesn't have a gsub table" );
		hasGSub = FALSE;
	    }
	}
    }

    if ( !gpos ) {
	if ( (error = TT_Load_GPOS_Table( face, &gpos, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
// 		qDebug("error loading gpos table: %d", error );
		return FALSE;
	    } else {
// 		qDebug("face doesn't have a gpos table" );
		hasGPos = FALSE;
	    }
	}
    }

    if ( loadTables( script ) ) {
	current_script = script;
	return TRUE;
    }
    return FALSE;
}

bool OpenTypeIface::applyGlyphSubstitutions( unsigned int script, ShapedItem *shaped, unsigned short *featuresToApply )
{
    if ( script != current_script ) {
	TT_GSUB_Clear_Features( gsub );

	if ( script == Arabic && loadTables( script ) )
	    current_script = script;
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
//       qDebug("    glyph[%d] = %x apply=%x, logcluster=%d", i, shaped->d->glyphs[i], featuresToApply[i], i );
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
//  	qDebug("    glyph[%d] logcluster=%d mark=%d", i, out->logClusters[i], shaped->d->glyphAttributes[i].mark );
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

	if ( script != current_script ) {
	    TT_GSUB_Clear_Features( gsub );

	    if ( script == Arabic && loadTables( script ) )
		current_script = script;
	    else
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
