#ifndef SCRIPTENGINESYRIAC_H
#define SCRIPTENGINESYRIAC_H

#include "scriptenginearabic.h"


class QScriptEngineSyriac : public QScriptEngineArabic
{
public:
    void shape( QShapedItem *result );
};

#endif
