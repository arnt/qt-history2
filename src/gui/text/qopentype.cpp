#include "qopentype_p.h"
#include "qfontengine_p.h"

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
//  	    qDebug("error loading gdef table: %d", error );
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

void QOpenType::init(QGlyphLayout *glyphs, int num_glyphs,
		     unsigned short *logClusters, int len, int /*char_offset*/)
{
    if ( !str )
	TT_GSUB_String_New(&str);
    if ( str->allocated < (uint)num_glyphs )
	TT_GSUB_String_Allocate( str, num_glyphs );
    if ( !tmp )
	TT_GSUB_String_New(&tmp);
    if ( tmp->allocated < (uint)num_glyphs )
	TT_GSUB_String_Allocate( tmp, num_glyphs );
    tmp->length = 0;

    length = len;

    Q_ASSERT(len == num_glyphs);
    tmpAttributes = (QGlyphLayout::Attributes *) realloc( tmpAttributes, num_glyphs*sizeof(QGlyphLayout::Attributes) );
    for (int i = 0; i < num_glyphs; ++i) {
	str->string[i] = glyphs[i].glyph;
	tmpAttributes[i] = glyphs[i].attributes;
    }

    for (int i = 0; i < num_glyphs; ++i)
	str->character_index[i] = i;

#ifdef OT_DEBUG
    qDebug("-----------------------------------------");
    qDebug("log clusters before shaping:");
    for (int j = 0; j < length; j++)
	qDebug("    log[%d] = %d", j, logClusters[j] );
    qDebug("original glyphs:");
    for (int i = 0; i < num_glyphs; ++i)
	qDebug("   glyph=%4x char_index=%d mark: %d cmb: %d", str->string[i], str->character_index[i], glyphAttributes[i].mark, glyphAttributes[i].combiningClass);
#endif
    str->length = num_glyphs;
    orig_nglyphs = num_glyphs;

    tmpLogClusters = (unsigned short *) realloc( tmpLogClusters, length*sizeof(unsigned short) );
    memcpy( tmpLogClusters, logClusters, length*sizeof(unsigned short) );
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
#endif

    unsigned char w[256];
    unsigned char *where_to_apply = w;
    if (str->length > 255)
	where_to_apply = (unsigned char *)malloc(str->length*sizeof(unsigned char));

    memset(where_to_apply, 1, str->length);
    if (where) {
	int j = str->length-1;
	for (int i = orig_nglyphs-1; i >= 0; --i) {
	    if (str->character_index[j] > i)
		--j;
	    if (!where[i])
		where_to_apply[j] = 0;
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
    for ( int i = 0; i < (int)str->length; i++) {
      qDebug("   %4x", str->string[i]);
    }
#endif
    positioned = FALSE;
}


extern void q_heuristicPosition( QTextEngine *engine, QScriptItem *item );

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

void QOpenType::appendTo(QTextEngine *engine, QScriptItem *si, bool doLogClusters)
{
#ifdef OT_DEBUG
    qDebug("QOpenType::finalize:");
#endif
    // make sure we have enough space to write everything back
    engine->ensureSpace( si->num_glyphs + str->length );

    QGlyphLayout *glyphs = engine->glyphs( si ) + si->num_glyphs;

    for (unsigned int i = 0; i < str->length; ++i)
	glyphs[i].glyph = str->string[i];

    if (doLogClusters) {
	// we can't do this for indic, as we pass the stuf in syllables and it's easier to do it in the shaper.
	unsigned short *logClusters = engine->logClusters( si );
	int clusterStart = 0;
	int oldCi = 0;
	for ( int i = 0; i < (int)str->length; i++ ) {
	    int ci = str->character_index[i];
	    glyphs[i].attributes = tmpAttributes[ci];
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
    QFontEngine *font = engine->fontEngine(*si);
    font->recalcAdvances( str->length, glyphs );
    for ( int i = 0; i < (int)str->length; i++ ) {
	if ( glyphs[i].attributes.mark ) {
	    glyphs[i].advance.x = 0;
	    glyphs[i].advance.y = 0;
	}
// 	    qDebug("   adv=%d", glyphs[i].advance);
    }
    si->num_glyphs += str->length;

    // positioning code:
    if ( hasGPos && positioned) {
	float scale = font->scale();
// 	qDebug("positioned glyphs:" );
	for ( int i = 0; i < (int)str->length; i++) {
// 	    qDebug("    %d:\t orig advance: %d\tadv=(%d/%d)\tpos=(%d/%d)\tback=%d\tnew_advance=%d", i,
// 		   glyphs[i].advance, (int)(positions[i].x_advance >> 6), (int)(positions[i].y_advance >> 6 ),
// 		   (int)(positions[i].x_pos >> 6 ), (int)(positions[i].y_pos >> 6),
// 		   positions[i].back, positions[i].new_advance );
	    // ###### fix the case where we have y advances. How do we handle this in Uniscribe?????
	    if ( positions[i].new_advance ) {
		glyphs[i].advance.x = qRound((positions[i].x_advance >> 6)*scale);
		glyphs[i].advance.y = qRound((-positions[i].y_advance >> 6)*scale);
	    } else {
		glyphs[i].advance.x += qRound((positions[i].x_advance >> 6)*scale);
		glyphs[i].advance.y -= qRound((positions[i].y_advance >> 6)*scale);
	    }
	    glyphs[i].offset.x = qRound((positions[i].x_pos >> 6)*scale);
	    glyphs[i].offset.y = -qRound((positions[i].y_pos >> 6)*scale);
	    int back = positions[i].back;
	    if ( si->analysis.bidiLevel % 2 ) {
		while ( back-- ) {
		    glyphs[i].offset.x -= glyphs[i-back].advance.x;
		    glyphs[i].offset.y -= -glyphs[i-back].advance.y;
		}
	    } else {
		while ( back ) {
		    glyphs[i].offset.x -= glyphs[i-(back--)].advance.x;
		    glyphs[i].offset.y -= -glyphs[i-(back--)].advance.y;
		}
	    }
// 	    qDebug("   ->\tadv=%d\tpos=(%d/%d)",
// 		   glyphs[i].advance, glyphs[i].offset.x, glyphs[i].offset.y );
	}
	si->hasPositioning = TRUE;
    } else {
	q_heuristicPosition( engine, si );
    }

#ifdef OT_DEBUG
    qDebug("log clusters after shaping:");
    if (doLogClusters) {
	for (int j = 0; j < length; j++)
	    qDebug("    log[%d] = %d", j, engine->logClusters(si)[j] );
    }
    qDebug("final glyphs:");
    for (int i = 0; i < (int)str->length; ++i)
	qDebug("   glyph=%4x char_index=%d mark: %d cmp: %d, clusterStart: %d width=%d",
	       glyphs[i], str->character_index[i], glyphAttributes[i].mark, glyphAttributes[i].combiningClass, glyphAttributes[i].clusterStart,
	       glyphs[i].advance);
    qDebug("-----------------------------------------");
#endif
}
