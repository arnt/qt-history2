#ifndef SCRIPTENGINE_DEVANAGARI_H
#define SCRIPTENGINE_DEVANAGARI_H

#include "scriptenginebasic.h"


class ScriptEngineDevanagari : public ScriptEngineBasic
{
public:
    void shape( ShapedItem *result );
    void position( ShapedItem *result );
};

#endif
