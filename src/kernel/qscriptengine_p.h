#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include "qtextengine_p.h"
class QString;
class QOpenType;
class QTextEngine;

typedef void (*ShapeFunction)( int script, const QString &, int, int, QTextEngine *, QScriptItem * );
typedef void (*AttributeFunction)( int script, const QString &, int, int, QCharAttributes * );

struct q_scriptEngine {
    ShapeFunction shape;
    AttributeFunction charAttributes;
};

extern const q_scriptEngine scriptEngines[];

#endif
