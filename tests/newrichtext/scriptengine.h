#include "qtextlayout.h"

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include "fontengine.h"

class ScriptEngine
{
public:
    virtual void charAttributes( const QString &text, int from, int len, CharAttributes *attributes ) = 0;
    // shaped is an in/out paramter. It already contains the correct font engine
    virtual void shape( const QString &text, int from, int len, ShapedItem *shaped ) = 0;

    //virtual void position( xxxxx ) = 0;

    virtual int cursorToX( int cPos, const QString &text, int from, int len, const ShapedItem &shaped ) = 0;
    virtual int xToCursor( int x, const QString &text, int from, int len, const ShapedItem &shaped ) = 0;


    //virtual const char ** supportedCMaps() const = 0;
};

#endif
