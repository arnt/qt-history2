#ifndef SCRIPTENGINEBASIC_H
#define SCRIPTENGINEBASIC_H

#include "scriptengine.h"


class ScriptEngineBasic : public ScriptEngine
{
public:
    void charAttributes( const QString &text, int from, int len, CharAttributes *attributes );

    void shape( ShapedItem *result );
    void position( ShapedItem *shaped );

    // internal
    static void calculateAdvances( ShapedItem *shaped );
};


#endif
