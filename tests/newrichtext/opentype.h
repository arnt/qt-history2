#ifndef OPENTYPEIFACE_H
#define OPENTYPEIFACE_H

#include <freetype/freetype.h>
#include "opentype/ftxopen.h"

class ShapedItem;

class OpenTypeIface
{
public:
    OpenTypeIface( FT_Face face );


    enum Scripts {
	Arabic = FT_MAKE_TAG( 'a', 'r', 'a', 'b' ),
	Armenian = FT_MAKE_TAG( 'a', 'r', 'm', 'n' ),
	Bengali = FT_MAKE_TAG( 'b', 'e', 'n', 'g' ),
	Bopomofo = FT_MAKE_TAG( 'b', 'o', 'p', 'o' ),
	Braille = FT_MAKE_TAG( 'b', 'r', 'a', 'i' ),
	ByzantineMusic = FT_MAKE_TAG( 'b', 'y', 'z', 'm' ),
	CanadianSyllabics = FT_MAKE_TAG( 'c', 'a', 'n', 's' ),
	Cherokee = FT_MAKE_TAG( 'c', 'h', 'e', 'r' ),
	Han = FT_MAKE_TAG( 'h', 'a', 'n', 'i' ),
	Cyrillic = FT_MAKE_TAG( 'c', 'y', 'r', 'l' ),
	Default = FT_MAKE_TAG( 'D', 'F', 'L', 'T' ),
	Devanagari = FT_MAKE_TAG( 'd', 'e', 'v', 'a' ),
	Ethiopic = FT_MAKE_TAG( 'e', 't', 'h', 'i' ),
	Georgian = FT_MAKE_TAG( 'g', 'e', 'o', 'r' ),
	Greek = FT_MAKE_TAG( 'g', 'r', 'e', 'k' ),
	Gujarati = FT_MAKE_TAG( 'g', 'u', 'j', 'r' ),
	Gurmukhi = FT_MAKE_TAG( 'g', 'u', 'r', 'u' ),
	HangulJamo = FT_MAKE_TAG( 'j', 'a', 'm', 'o' ),
	Hangul = FT_MAKE_TAG( 'h', 'a', 'n', 'g' ),
	Hebrew = FT_MAKE_TAG( 'h', 'e', 'b', 'r' ),
	Hiragana = FT_MAKE_TAG( 'k', 'a', 'n', 'a' ),
	Kannada = FT_MAKE_TAG( 'k', 'n', 'd', 'a' ),
	Katakana = FT_MAKE_TAG( 'k', 'a', 'n', 'a' ),
	Khmer = FT_MAKE_TAG( 'k', 'h', 'm', 'r' ),
	Lao = FT_MAKE_TAG( 'l', 'a', 'o', ' ' ),
	Latin = FT_MAKE_TAG( 'l', 'a', 't', 'n' ),
	Malayalam = FT_MAKE_TAG( 'm', 'l', 'y', 'm' ),
	Mongolian = FT_MAKE_TAG( 'm', 'o', 'n', 'g' ),
	Myanmar = FT_MAKE_TAG( 'm', 'y', 'm', 'r' ),
	Ogham = FT_MAKE_TAG( 'o', 'g', 'a', 'm' ),
	Oriya = FT_MAKE_TAG( 'o', 'r', 'y', 'a' ),
	Runic = FT_MAKE_TAG( 'r', 'u', 'n', 'r' ),
	Sinhala = FT_MAKE_TAG( 's', 'i', 'n', 'h' ),
	Syriac = FT_MAKE_TAG( 's', 'y', 'r', 'c' ),
	Tamil = FT_MAKE_TAG( 't', 'a', 'm', 'l' ),
	Telugu = FT_MAKE_TAG( 't', 'e', 'l', 'u' ),
	Thaana = FT_MAKE_TAG( 't', 'h', 'a', 'a' ),
	Thai = FT_MAKE_TAG( 't', 'h', 'a', 'i' ),
	Tibetan = FT_MAKE_TAG( 't', 'i', 'b', 't' ),
	Yi = FT_MAKE_TAG( 'y', 'i', ' ', ' ' ),
    };

    bool supportsScript( unsigned int script );

    bool applyGlyphSubstitutions( unsigned int script, ShapedItem *shaped, unsigned short *featuresToApply );
    bool applyGlyphPositioning( unsigned int script, ShapedItem *shaped );

private:
    bool loadTables( FT_ULong script);


    FT_Face face;
    TTO_GDEF gdef;
    TTO_GSUB gsub;
    TTO_GPOS gpos;
    FT_UShort script_index;
    FT_ULong current_script;
    unsigned short found_bits;
    unsigned short always_apply;
    bool hasGDef : 1;
    bool hasGSub : 1;
    bool hasGPos : 1;
};

#endif
