#ifndef SCRIPTENGINEBASIC_H
#define SCRIPTENGINEBASIC_H

#include "scriptengine.h"


class QScriptEngineBasic : public QScriptEngine
{
public:
    void charAttributes( const QString &text, int from, int len, CharAttributes *attributes );

    void shape( QShapedItem *result );
    void position( QShapedItem *shaped );

    // internal
    static void calculateAdvances( QShapedItem *shaped );
};


#endif
