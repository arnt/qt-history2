#ifndef OPENTYPEIFACE_H
#define OPENTYPEIFACE_H

#include <freetype/freetype.h>
#include "opentype/ftxopen.h"

class ShapedItem;

class OpenTypeIface
{
public:
    OpenTypeIface( FT_Face face );

    bool supportsScript( unsigned int script );

    void apply( unsigned int script, ShapedItem *shaped, unsigned short *featuresToApply );

private:
    bool loadTables( FT_ULong script);
    TTO_GSUB_String *substitute( ShapedItem *shaped, unsigned short *featuresToApply );
    void position( ShapedItem *shaped, TTO_GSUB_String *in );


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
