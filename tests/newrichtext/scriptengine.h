#include "qtextlayout.h"

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include "fontengine.h"

class ScriptEngine
{
public:
    virtual void charAttributes( const QString &text, int from, int len, CharAttributes *attributes ) = 0;
    // shaped is an in/out paramter. It already contains the correct font engine
    virtual void shape( ShapedItem *shaped ) = 0;
    virtual void position( ShapedItem *shaped ) = 0;

    //virtual const char ** supportedCMaps() const = 0;


    // some helper methods that might get used by all script engines. Implemented in scriptenginebasic.cpp

    // try to position diacritics around it's base char in absence of any better way to determine
    // positioning (like open type tables)
    // needs a correct logClusters and glyphAttributes array.
    void heuristicPositionMarks( ShapedItem *shaped );

    // set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars ang glyphs
    // and no reordering (except for reversing if (bidiLevel % 2 ) )
    // also computes logClusters heuristically
    void heuristicSetGlyphAttributes( ShapedItem *shaped );
};

#endif
