#ifndef SCRIPTENGINE_BENGALI_H
#define SCRIPTENGINE_BENGALI_H

#include "scriptenginebasic.h"


class QScriptEngineBengali : public QScriptEngineBasic
{
public:
    void shape( QShapedItem *result );
    void position( QShapedItem *result );
};

#endif
