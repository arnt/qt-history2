#include "qtextlayout.h"

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include "fontengine.h"

class ScriptEngine
{
public:
    virtual void charAttributes( const QString &text, int from, int len, CharAttributes *attributes ) = 0;
    virtual void shape( const FontEngineIface &f, const QString &text, int from, int len,
			const ScriptAnalysis &analysis, ShapedItem *result ) = 0;

    //virtual void position( xxxxx ) = 0;

    virtual int cursorToX( int cursorPos, const FontEngineIface &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped ) = 0;
    virtual int xToCursor( int x, const FontEngineIface &f, const QString &text, int from, int len,
			   const ScriptAnalysis &analysis, const ShapedItem &shaped ) = 0;


    //virtual const char ** supportedCMaps() const = 0;
};

#endif
