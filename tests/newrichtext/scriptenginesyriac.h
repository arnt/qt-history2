#ifndef SCRIPTENGINESYRIAC_H
#define SCRIPTENGINESYRIAC_H

#include "scriptenginearabic.h"


class ScriptEngineSyriac : public ScriptEngineArabic
{
public:
    void shape( ShapedItem *result );
};

#endif
