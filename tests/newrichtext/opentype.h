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
	Arabic = FT_MAKE_TAG( 'a', 'r', 'a', 'b' )
    };

    bool supportsScript( unsigned int script );

    void applyGlyphSubstitutions( unsigned int script, ShapedItem *shaped, unsigned short *featuresToApply );
    void applyGlyphPositioning( unsigned int script, ShapedItem *shaped );

private:
    bool loadArabicTables( FT_ULong script);


    FT_Face face;
    TTO_GDEF gdef;
    TTO_GSUB gsub;
    TTO_GPOS gpos;
    FT_UShort script_index;
    FT_ULong current_script;
    FT_ULong found_bits;
    bool hasGDef : 1;
    bool hasGSub : 1;
    bool hasGPos : 1;
};

#endif
