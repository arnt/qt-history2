#include "qtextlayout.h"

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

// ### the FontEngine should be a more internal structure.
typedef QFont FontEngine;

class ScriptEngine
{
public:
    virtual void charAttributes( const QString &text, int from, int len, CharAttributes *attributes ) = 0;
    virtual void shape( const FontEngine &f, const QString &text, int from, int len,
			const ScriptAnalysis &analysis, ShapedItem *result ) = 0;

    virtual int cursorToX( int cursorPos, const FontEngine &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped ) = 0;
    virtual int xToCursor( int x, const FontEngine &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped ) = 0;
};

#endif
