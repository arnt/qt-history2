#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

class QString;
class QOpenType;
struct QCharAttributes;
struct QScriptItem;

typedef void (*ShapeFunction)( int script, const QString &, int, int, QScriptItem * );
typedef void (*AttributeFunction)( const QString &, int, int, QCharAttributes * );

struct q_scriptEngine {
    ShapeFunction shape;
    AttributeFunction charAttributes;
};

extern const q_scriptEngine scriptEngines[];

#endif
