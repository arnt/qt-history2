#ifndef SCRIPTENGINE_TAMIL_H
#define SCRIPTENGINE_TAMIL_H

#include "scriptenginebasic.h"


class QScriptEngineTamil : public QScriptEngineBasic
{
public:
    void shape( QShapedItem *result );
    void position( QShapedItem *result );
};

#endif
