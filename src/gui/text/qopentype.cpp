#include "qopentype_p.h"
#include "qfontengine_p.h"
#include "qscriptengine_p.h"

//  --------------------------------------------------------------------------------------------------------------------
// Open type support
//  --------------------------------------------------------------------------------------------------------------------

// #define OT_DEBUG

#ifdef OT_DEBUG
static inline char *tag_to_string( FT_ULong tag )
{
    static char string[5];
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
    return string;
}
#endif

#define DefaultLangSys 0xffff
#define DefaultScript FT_MAKE_TAG( 'D', 'F', 'L', 'T' )

static const unsigned int supported_scripts [] = {
// 	// European Alphabetic Scripts
// 	Latin,
    FT_MAKE_TAG( 'l', 'a', 't', 'n' ),
// 	Greek,
    FT_MAKE_TAG( 'g', 'r', 'e', 'k' ),
// 	Cyrillic,
    FT_MAKE_TAG( 'c', 'y', 'r', 'l' ),
// 	Armenian,
        FT_MAKE_TAG( 'a', 'r', 'm', 'n' ),
// 	Georgian,
    FT_MAKE_TAG( 'g', 'e', 'o', 'r' ),
// 	Runic,
    FT_MAKE_TAG( 'r', 'u', 'n', 'r' ),
// 	Ogham,
    FT_MAKE_TAG( 'o', 'g', 'a', 'm' ),
// 	SpacingModifiers,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	CombiningMarks,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),

// 	// Middle Eastern Scripts
// 	Hebrew,
    FT_MAKE_TAG( 'h', 'e', 'b', 'r' ),
// 	Arabic,
    FT_MAKE_TAG( 'a', 'r', 'a', 'b' ),
// 	Syriac,
    FT_MAKE_TAG( 's', 'y', 'r', 'c' ),
// 	Thaana,
    FT_MAKE_TAG( 't', 'h', 'a', 'a' ),

// 	// South and Southeast Asian Scripts
// 	Devanagari,
    FT_MAKE_TAG( 'd', 'e', 'v', 'a' ),
// 	Bengali,
    FT_MAKE_TAG( 'b', 'e', 'n', 'g' ),
// 	Gurmukhi,
    FT_MAKE_TAG( 'g', 'u', 'r', 'u' ),
// 	Gujarati,
    FT_MAKE_TAG( 'g', 'u', 'j', 'r' ),
// 	Oriya,
    FT_MAKE_TAG( 'o', 'r', 'y', 'a' ),
// 	Tamil,
    FT_MAKE_TAG( 't', 'a', 'm', 'l' ),
// 	Telugu,
    FT_MAKE_TAG( 't', 'e', 'l', 'u' ),
// 	Kannada,
    FT_MAKE_TAG( 'k', 'n', 'd', 'a' ),
// 	Malayalam,
    FT_MAKE_TAG( 'm', 'l', 'y', 'm' ),
// 	Sinhala,
    // ### could not find any OT specs on this
    FT_MAKE_TAG( 's', 'i', 'n', 'h' ),
// 	Thai,
    FT_MAKE_TAG( 't', 'h', 'a', 'i' ),
// 	Lao,
    FT_MAKE_TAG( 'l', 'a', 'o', ' ' ),
// 	Tibetan,
    FT_MAKE_TAG( 't', 'i', 'b', 't' ),
// 	Myanmar,
    FT_MAKE_TAG( 'm', 'y', 'm', 'r' ),
// 	Khmer,
    FT_MAKE_TAG( 'k', 'h', 'm', 'r' ),

// 	// East Asian Scripts
// 	Han,
    FT_MAKE_TAG( 'h', 'a', 'n', 'i' ),
// 	Hiragana,
    FT_MAKE_TAG( 'k', 'a', 'n', 'a' ),
// 	Katakana,
    FT_MAKE_TAG( 'k', 'a', 'n', 'a' ),
// 	Hangul,
    FT_MAKE_TAG( 'h', 'a', 'n', 'g' ),
// 	Bopomofo,
    FT_MAKE_TAG( 'b', 'o', 'p', 'o' ),
// 	Yi,
    FT_MAKE_TAG( 'y', 'i', ' ', ' ' ),

// 	// Additional Scripts
// 	Ethiopic,
    FT_MAKE_TAG( 'e', 't', 'h', 'i' ),
// 	Cherokee,
    FT_MAKE_TAG( 'c', 'h', 'e', 'r' ),
// 	CanadianAboriginal,
    FT_MAKE_TAG( 'c', 'a', 'n', 's' ),
// 	Mongolian,
    FT_MAKE_TAG( 'm', 'o', 'n', 'g' ),

// 	// Symbols
// 	CurrencySymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	LetterlikeSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	NumberForms,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	MathematicalOperators,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	TechnicalSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	GeometricSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	MiscellaneousSymbols,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	EnclosedAndSquare,
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
// 	Braille,
    FT_MAKE_TAG( 'b', 'r', 'a', 'i' ),

//                Unicode, should be used
    FT_MAKE_TAG( 'D', 'F', 'L', 'T' )
    // ### where are these?
// 	FT_MAKE_TAG( 'b', 'y', 'z', 'm' ),
//     FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
    // ### Hangul Jamo
//     FT_MAKE_TAG( 'j', 'a', 'm', 'o' ),
};

QOpenType::QOpenType( FT_Face _face )
    : face( _face ), gdef( 0 ), gsub( 0 ), gpos( 0 ), current_script( 0 )
{
    hasGDef = hasGSub = hasGPos = TRUE;
    str = tmp = 0;
    positions = 0;
    tmpAttributes = 0;
    tmpLogClusters = 0;
}

QOpenType::~QOpenType()
{
    if ( gpos )
	TT_Done_GPOS_Table( gpos );
    if ( gsub )
	TT_Done_GSUB_Table( gsub );
    if ( gdef )
	TT_Done_GDEF_Table( gdef );
    if ( str )
	TT_GSUB_String_Done( str );
    if ( tmp )
	TT_GSUB_String_Done( tmp );
    if (positions)
	free(positions);
    if ( tmpAttributes )
	free(tmpAttributes);
    if (tmpLogClusters)
	free(tmpLogClusters);
}

bool QOpenType::supportsScript( unsigned int script )
{
    if ( current_script == supported_scripts[script] )
	return TRUE;

#ifdef OT_DEBUG
    qDebug("trying to load tables for script %d (%s))", script, tag_to_string( supported_scripts[script] ));
#endif

    FT_Error error;
    if ( !gdef ) {
	if ( (error = TT_Load_GDEF_Table( face, &gdef )) ) {
//   	    qDebug("error loading gdef table: %d", error );
	    hasGDef = FALSE;
	}
    }

    if ( !gsub ) {
	if ( (error = TT_Load_GSUB_Table( face, &gsub, gdef )) ) {
	    if ( error != FT_Err_Table_Missing ) {
//  		qDebug("error loading gsub table: %d", error );
		return FALSE;
	    } else {
//  		qDebug("face doesn't have a gsub table" );
		hasGSub = FALSE;
	    }
	}
    }

    if ( !gpos ) {
	if ( (error = TT_Load_GPOS_Table( face, &gpos, gdef )) ) {
//  		qDebug("error loading gpos table: %d", error );
	    hasGPos = FALSE;
	}
    }

    if ( loadTables( script ) ) {
	return TRUE;
    }
    return FALSE;
}

bool QOpenType::loadTables( FT_ULong script)
{
    assert( script < QFont::Unicode );
    // find script in our list of supported scripts.
    unsigned int stag = supported_scripts[script];

    FT_Error error = TT_GSUB_Select_Script( gsub, stag, &script_index );
    if ( error ) {
#ifdef OT_DEBUG
 	qDebug("could not select script %d: %d", (int)script, error );
#endif
	if ( stag == DefaultScript ) {
	    // try to load default language system
	    error = TT_GSUB_Select_Script( gsub, DefaultScript, &script_index );
	    if ( error )
		return FALSE;
	} else {
	    return FALSE;
	}
    }
    script = stag;

#ifdef OT_DEBUG
    qDebug("script %s has script index %d", tag_to_string(script), script_index );
#endif

#ifdef OT_DEBUG
    {
	TTO_FeatureList featurelist = gsub->FeatureList;
	int numfeatures = featurelist.FeatureCount;
	qDebug("gsub table has %d features", numfeatures );
	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    qDebug("   feature '%s'", tag_to_string(r->FeatureTag));
	}
    }
#endif
    if ( hasGPos ) {
	FT_UShort script_index;
	error = TT_GPOS_Select_Script( gpos, script, &script_index );
	if ( error ) {
// 	    qDebug("could not select script in gpos table: %d", error );
	    return TRUE;
	}

	TTO_FeatureList featurelist = gpos->FeatureList;

	int numfeatures = featurelist.FeatureCount;

#ifdef OT_DEBUG
 	qDebug("gpos table has %d features", numfeatures );
#endif

	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    FT_UShort feature_index;
	    TT_GPOS_Select_Feature( gpos, r->FeatureTag, script_index, 0xffff, &feature_index );

#ifdef OT_DEBUG
 	    qDebug("   feature '%s'", tag_to_string(r->FeatureTag));
#endif
	}


    }

    current_script = script;

    return TRUE;
}

#ifdef OT_DEBUG
static void dump_string(TTO_GSUB_String *string)
{
    for (int i = 0; i < string->length; ++i) {
	qDebug("    %x: component=%d", string->string[i], string->glyph_properties[i].component);
    }
}
#endif

void QOpenType::init(QShaperItem *item)
{
    length = item->num_glyphs;

    if ( !str )
	TT_GSUB_String_New(&str);
    if ( str->allocated < (uint)length )
	TT_GSUB_String_Allocate( str, length );
    if ( !tmp )
	TT_GSUB_String_New(&tmp);
    if ( tmp->allocated < (uint)length )
	TT_GSUB_String_Allocate( tmp, length );
    tmp->length = 0;


    tmpAttributes = (QGlyphLayout::Attributes *) realloc( tmpAttributes, length*sizeof(QGlyphLayout::Attributes) );
    for (int i = 0; i < length; ++i) {
	str->string[i] = item->glyphs[i].glyph;
	str->glyph_properties[i].component = 0;
	tmpAttributes[i] = item->glyphs[i].attributes;
    }

    for (int i = 0; i < length; ++i)
	str->character_index[i] = i;

    str->length = length;
    orig_nglyphs = length;
#ifdef OT_DEBUG
    qDebug("-----------------------------------------");
    qDebug("log clusters before shaping:");
    for (int j = 0; j < length; j++)
	qDebug("    log[%d] = %d", j, item->log_clusters[j] );
    qDebug("original glyphs:");
    for (int i = 0; i < length; ++i)
	qDebug("   glyph=%4x char_index=%d mark: %d cmb: %d", str->string[i], str->character_index[i],
	       item->glyphs[i].attributes.mark, item->glyphs[i].attributes.combiningClass);
    dump_string(str);
#endif

    tmpLogClusters = (unsigned short *) realloc( tmpLogClusters, length*sizeof(unsigned short) );
    memcpy( tmpLogClusters, item->log_clusters, length*sizeof(unsigned short) );
}

void QOpenType::applyGSUBFeature(unsigned int featureTag, bool *where)
{
    FT_UShort feature_index;
    FT_Error err = TT_GSUB_Select_Feature( gsub, featureTag, script_index, 0xffff, &feature_index);
    if (err) {
#ifdef OT_DEBUG
// 	qDebug("feature %s not covered by table or language system", tag_to_string( featureTag ));
#endif
	return;
    }

#ifdef OT_DEBUG
    qDebug("applying GSUB feature %s with index %d", tag_to_string( featureTag ), feature_index);
    if (where)
	for (int i = 0; i < orig_nglyphs; ++i)
	    qDebug("   apply_char=%s", where[i] ? "true" : "false");
#endif

    unsigned char w[256];
    unsigned char *where_to_apply = w;
    if (str->length > 255)
	where_to_apply = (unsigned char *)malloc(str->length*sizeof(unsigned char));

    memset(where_to_apply, 1, str->length);
    if (where) {
	for (uint i = 0; i < str->length; ++i) {
	    where_to_apply[i] = where[str->character_index[i]];
	}
#ifdef OT_DEBUG
	for (int i = 0; i < (int)str->length; ++i)
	    qDebug("   apply=%s", where_to_apply[i] ? "true" : "false");
#endif
    }

    TT_GSUB_Apply_Feature(gsub, feature_index, where_to_apply, &str, &tmp);

    if (w != where_to_apply)
	free(where_to_apply);

#ifdef OT_DEBUG
    qDebug("after applying:");
    dump_string(str);
#endif
    positioned = FALSE;
}


extern void q_heuristicPosition(QShaperItem *item);

void QOpenType::applyGPOSFeatures()
{
#ifdef OT_DEBUG
    qDebug("applying GPOS features");
#endif
    // currently just apply all features

    if ( hasGPos ) {
	positions = (TTO_GPOS_Data *) realloc( positions, str->length*sizeof(TTO_GPOS_Data) );
	memset(positions, 0, str->length*sizeof(TTO_GPOS_Data));

	TTO_FeatureList featurelist = gpos->FeatureList;
	int numfeatures = featurelist.FeatureCount;

	for( int i = 0; i < numfeatures; i++ ) {
	    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
	    FT_UShort feature_index;
	    FT_Error error = TT_GPOS_Select_Feature( gpos, r->FeatureTag, script_index, 0xffff, &feature_index );
	    if (error != FT_Err_Ok)
		continue;

#ifdef OT_DEBUG
	    qDebug("applying POS feature %s with index %d", tag_to_string( r->FeatureTag ), feature_index);
#endif

	    str->pos = 0;
	    // ### is FT_LOAD_DEFAULT the right thing to do?
	    TT_GPOS_Apply_Feature( face, gpos, feature_index, FT_LOAD_DEFAULT, str, &positions, FALSE, false );
	}
    }
    positioned = TRUE;
}

const int *QOpenType::mapping(int &len)
{
    len = str->length;
    return str->character_index;
}

bool QOpenType::appendTo(QShaperItem *item, bool doLogClusters)
{
#ifdef OT_DEBUG
    qDebug("QOpenType::finalize:");
#endif
    // make sure we have enough space to write everything back
    if (item->num_glyphs < (int)str->length) {
	item->num_glyphs = str->length;
	return false;
    }

    QGlyphLayout *glyphs = item->glyphs;

    for (unsigned int i = 0; i < str->length; ++i) {
	glyphs[i].glyph = str->string[i];
	glyphs[i].attributes = tmpAttributes[str->character_index[i]];
    }

    if (doLogClusters) {
	// we can't do this for indic, as we pass the stuf in syllables and it's easier to do it in the shaper.
	unsigned short *logClusters = item->log_clusters;
	int clusterStart = 0;
	int oldCi = 0;
	for ( int i = 0; i < (int)str->length; i++ ) {
	    int ci = str->character_index[i];
	    // 	qDebug("   ci[%d] = %d mark=%d, cmb=%d, cs=%d tmplc=%d",
	    // 	       i, ci, glyphAttributes[i].mark, glyphAttributes[i].combiningClass, glyphAttributes[i].clusterStart,  tmpLogClusters[ci]);
	    if ( !glyphs[i].attributes.mark && glyphs[i].attributes.clusterStart && ci != oldCi ) {
		for ( int j = oldCi; j < ci; j++ )
		    logClusters[j] = clusterStart;
		clusterStart = i;
		oldCi = ci;
	    }
	}
	for ( int j = oldCi; j < length; j++ )
	    logClusters[j] = clusterStart;
    }

    // calulate the advances for the shaped glyphs
//     qDebug("unpositioned: ");
    item->font->recalcAdvances( str->length, glyphs, 0 ); // WYSIWYG: Fixme
    item->num_glyphs = str->length;

    // positioning code:
    if ( hasGPos && positioned) {
// 	qDebug("positioned glyphs:" );
	// ### use Q26Dot6
	float scale = item->font->scale();
	for ( int i = 0; i < (int)str->length; i++) {
// 	    qDebug("    %d:\t orig advance: (%d/%d)\tadv=(%d/%d)\tpos=(%d/%d)\tback=%d\tnew_advance=%d", i,
// 		   glyphs[i].advance.x, glyphs[i].advance.y,
// 		   (int)(positions[i].x_advance >> 6), (int)(positions[i].y_advance >> 6 ),
// 		   (int)(positions[i].x_pos >> 6 ), (int)(positions[i].y_pos >> 6),
// 		   positions[i].back, positions[i].new_advance );
	    // ###### fix the case where we have y advances. How do we handle this in Uniscribe?????
	    if ( positions[i].new_advance ) {
		glyphs[i].advance.x = Q26Dot6((item->flags & QTextEngine::RightToLeft
					       ? -positions[i].x_advance : positions[i].x_advance), F26Dot6)*scale;
		glyphs[i].advance.y = Q26Dot6(-positions[i].y_advance, F26Dot6)*scale;
	    } else {
		glyphs[i].advance.x += Q26Dot6(item->flags & QTextEngine::RightToLeft
					       ? -positions[i].x_advance : positions[i].x_advance, F26Dot6)*scale;
		glyphs[i].advance.y -= Q26Dot6(positions[i].y_advance, F26Dot6)*scale;
	    }
	    glyphs[i].offset.x = Q26Dot6(positions[i].x_pos, F26Dot6);
	    glyphs[i].offset.y = Q26Dot6(-positions[i].y_pos, F26Dot6);
	    int back = positions[i].back;
	    if (item->flags & QTextEngine::RightToLeft) {
		while ( back-- ) {
		    glyphs[i].offset.x -= glyphs[i-back].advance.x;
		    glyphs[i].offset.y -= -glyphs[i-back].advance.y;
		}
	    } else {
		while ( back ) {
		    glyphs[i].offset.x -= glyphs[i-back].advance.x;
		    glyphs[i].offset.y -= -glyphs[i-back].advance.y;
		    --back;
		}
	    }
// 	    qDebug("   ->\tadv=%d\tpos=(%d/%d)",
// 		   glyphs[i].advance, glyphs[i].offset.x, glyphs[i].offset.y );
	}
    } else {
	q_heuristicPosition(item);
    }

#ifdef OT_DEBUG
    if (doLogClusters) {
	qDebug("log clusters after shaping:");
	for (int j = 0; j < length; j++)
	    qDebug("    log[%d] = %d", j, item->log_clusters[j] );
    }
    qDebug("final glyphs:");
    for (int i = 0; i < (int)str->length; ++i)
	qDebug("   glyph=%4x char_index=%d mark: %d cmp: %d, clusterStart: %d advance=%d/%d offset=%d/%d",
	       glyphs[i].glyph, str->character_index[i], glyphs[i].attributes.mark,
	       glyphs[i].attributes.combiningClass, glyphs[i].attributes.clusterStart,
	       glyphs[i].advance.x.value(), glyphs[i].advance.y.value(),
	       glyphs[i].offset.x.value(), glyphs[i].offset.y.value());
    qDebug("-----------------------------------------");
#endif
    return true;
}
