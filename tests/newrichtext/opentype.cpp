#include "opentype.h"
#include "opentype/ftxopen.h"

#include <qglobal.h>
#include "qtextlayout.h"


inline void tag_to_string( char *string, FT_ULong tag )
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

#define DefaultLangSys 0xffff;

struct Features {
    FT_ULong tag;
    unsigned short bit;
};

// ### keep in sync with Shape enum in scriptenginearabic.cpp
Features arabicGSUBFeatures[] = {
    { FT_MAKE_TAG( 'c', 'c', 'm', 'p' ), 0x01 },
    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), 0x02 },
    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), 0x04 },
    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), 0x08 },
    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), 0x10 },
    { FT_MAKE_TAG( 'r', 'l', 'i', 'g' ), 0x20 },
    { FT_MAKE_TAG( 'c', 'a', 'l', 't' ), 0x40 },
    { FT_MAKE_TAG( 'l', 'i', 'g', 'a' ), 0x80 },
    // mset is used in old Win95 fonts that don't have a 'mark' positioning table.
    { FT_MAKE_TAG( 'm', 's', 'e', 't' ), 0x100 },
    { 0,  0 }
};
// required for minimal support are: fina, medi, init and rlig. Some old fonts actually only have liga instead of rlig,
// so we also accept liga without rlig

inline bool arabicFoundRequiredFeatures( int found_bits ) {
    return ((found_bits & 0x3c) == 0x3c || (found_bits & 0x9c) == 0x9c );
}

// these are the features that should get applied in all cases (if available in the font)
static const short arabicGSUBorMask = 0xf1e1;

Features arabicGPOSFeatures[] = {
    { FT_MAKE_TAG( 'c', 'u', 'r', 's' ), 0x1000 },
    { FT_MAKE_TAG( 'k', 'e', 'r', 'n' ), 0x2000 },
    { FT_MAKE_TAG( 'm', 'a', 'r', 'k' ), 0x4000 },
    { FT_MAKE_TAG( 'm', 'k', 'm', 'k' ), 0x8000 },
    { 0,  0 }
};


bool OpenTypeIface::loadArabicTables( FT_ULong script)
{
    FT_Error error = TT_GSUB_Select_Script( gsub, script, &script_index );
    if ( error ) {
	qDebug("could not select arabic script: %d", error );
	return FALSE;
    }

    qDebug("arabic is script %d", script_index );

    TTO_FeatureList featurelist = gsub->FeatureList;

    int numfeatures = featurelist.FeatureCount;

    qDebug("table has %d features", numfeatures );


    found_bits = 0;
    for( int i = 0; i < numfeatures; i++ ) {
	TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	FT_ULong feature = r->FeatureTag;
	Features *f = arabicGSUBFeatures;
	while ( f->tag ) {
	    if ( f->tag == feature ) {
		found_bits |= f->bit;
		break;
	    }
	    f++;
	}
	FT_UShort feature_index;
	TT_GSUB_Select_Feature( gsub, f->tag, script_index, 0xffff, &feature_index );
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
	    qDebug("could not select arabic script in gpos table: %d", error );
	    return TRUE;
	}

	TTO_FeatureList featurelist = gpos->FeatureList;

	int numfeatures = featurelist.FeatureCount;

	qDebug("table has %d features", numfeatures );

	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    FT_ULong feature = r->FeatureTag;
	    Features *f = arabicGPOSFeatures;
	    while ( f->tag ) {
		if ( f->tag == feature ) {
		    found_bits |= f->bit;
		    break;
		}
		f++;
	    }
	    FT_UShort feature_index;
	    TT_GPOS_Select_Feature( gpos, f->tag, script_index, 0xffff, &feature_index );
	    TT_GPOS_Add_Feature( gpos, feature_index, f->bit );

	    char featureString[5];
	    tag_to_string( featureString, r->FeatureTag );
	    qDebug("found feature '%s' in GPOS table", featureString );
	}


    }
    if ( !arabicFoundRequiredFeatures( found_bits ) ) {
	qDebug( "not all required features for arabic found!" );
	TT_GSUB_Clear_Features( gsub );
	return FALSE;
    }
    qDebug("found_bits = %x",  (uint)found_bits );

    return TRUE;
}


OpenTypeIface::OpenTypeIface( FT_Face _face )
    : face( _face ), gdef( 0 ), gsub( 0 ), gpos( 0 ), current_script( 0 )
{
    hasGDef = hasGSub = hasGPos = TRUE;
    (void) supportsScript( Arabic );
}

bool OpenTypeIface::supportsScript( unsigned int script )
{
    if ( current_script == script )
	return TRUE;

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
    // nothing else implemented currently
    if ( script != Arabic )
	return FALSE;

    if ( loadArabicTables( script ) ) {
	current_script = script;
	return TRUE;
    }
    return FALSE;
}

void OpenTypeIface::applyGlyphSubstitutions( unsigned int script, ShapedItem *shaped, unsigned short *featuresToApply )
{
    if ( script != current_script ) {
	TT_GSUB_Clear_Features( gsub );

	if ( script == Arabic && loadArabicTables( script ) )
	    current_script = script;
    }

    for ( int i = 0; i < shaped->d->num_glyphs; i++ ) {
	featuresToApply[i] |= arabicGSUBorMask;
	featuresToApply[i] &= found_bits;
    }


    TTO_GSUB_String *in = 0;
    TTO_GSUB_String *out = 0;

    TT_GSUB_String_New( face->memory, &in );
    TT_GSUB_String_Set_Length( in, shaped->d->num_glyphs );
    TT_GSUB_String_New( face->memory, &out);

    qDebug("in: num_glyphs = %d", shaped->d->num_glyphs );
    for ( int i = 0; i < shaped->d->num_glyphs; i++) {
      in->string[i] = shaped->d->glyphs[i];
      qDebug("    glyph[%d] = %x apply=%x, logcluster=%d", i, shaped->d->glyphs[i], featuresToApply[i], shaped->d->logClusters[i] );
      in->logClusters[i] = shaped->d->logClusters[i];
      in->properties[i] = ~featuresToApply[i];
    }
    in->max_ligID = 0;

    TT_GSUB_Apply_String (gsub, in, out);

    if ( shaped->d->num_glyphs < (int)out->length )
	shaped->d->glyphs = ( GlyphIndex *) realloc( shaped->d->glyphs, out->length );
    shaped->d->num_glyphs = out->length;

    qDebug("out: num_glyphs = %d", shaped->d->num_glyphs );

    int lastCluster = -1;
    for ( int i = 0; i < shaped->d->num_glyphs; i++) {
	shaped->d->glyphs[i] = out->string[i];
	qDebug("    glyph[%d] = %x", i, shaped->d->glyphs[i] );
	shaped->d->logClusters[i] = out->logClusters[i];
	shaped->d->glyphAttributes[i].clusterStart = (shaped->d->logClusters[i] != lastCluster);
	// ### need to fix marks aswell!!!!
    }

    TT_GSUB_String_Done( in );

    // we need to keep this one around for shaping
    if ( hasGPos )
	shaped->d->enginePrivate = (void *)out;
    else
	TT_GSUB_String_Done( out );
}


void OpenTypeIface::applyGlyphPositioning( unsigned int script, ShapedItem *shaped )
{
    if ( !hasGPos )
	return;

    if ( script != current_script ) {
	TT_GSUB_Clear_Features( gsub );

	if ( script == Arabic && loadArabicTables( script ) )
	    current_script = script;
    }

    TTO_GSUB_String *in = (TTO_GSUB_String *)shaped->d->enginePrivate;
    TTO_GPOS_Data *out = 0;

    // ### is FT_LOAD_DEFAULT the right thing to do?
    TT_GPOS_Apply_String( face, gpos, FT_LOAD_DEFAULT, in, &out, FALSE, (shaped->d->analysis.bidiLevel % 2) );

    qDebug("positioned glyphs:" );
    for ( int i = 0; i < shaped->d->num_glyphs; i++) {
	qDebug("    %d:\tadv=(%d/%d)\tpos=(%d/%d)\tback=%d\tnew_advance=%d", i,
	       (int)(out[i].x_advance >> 6), (int)(out[i].y_advance >> 6 ),
	       (int)(out[i].x_pos >> 6 ), (int)(out[i].y_pos >> 6),
	       out[i].back, out[i].new_advance );
    }

    TT_GSUB_String_Done( in );
    shaped->d->enginePrivate = 0;
    free( out );
}
