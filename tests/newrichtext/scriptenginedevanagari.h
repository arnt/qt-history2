#ifndef SCRIPTENGINE_DEVANAGARI_H
#define SCRIPTENGINE_DEVANAGARI_H

#include "scriptenginebasic.h"


class QScriptEngineDevanagari : public QScriptEngineBasic
{
public:
    void shape( QShapedItem *result );
    void position( QShapedItem *result );
};

#endif
