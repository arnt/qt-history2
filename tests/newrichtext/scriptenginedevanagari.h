#ifndef SCRIPTENGINE_DEVANAGARI_H
#define SCRIPTENGINE_DEVANAGARI_H

#include "scriptenginebasic.h"


class ScriptEngineDevanagari : public ScriptEngineBasic
{
public:
    void shape( ShapedItem *result );
    void position( ShapedItem *result );
protected:
    void openTypeShape( int script, const OpenTypeIface*, ShapedItem *result, const QString &reordered, unsigned short *featuresToApply );
    void openTypePosition( int script, const OpenTypeIface *, ShapedItem *result );
};

#endif
