#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsView>
#include "../global.h"

Q_DECLARE_METATYPE(QGraphicsScene*)
Q_DECLARE_METATYPE(QGraphicsView*)

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QGraphicsView *view = 0;
    if (QGraphicsScene *scene = qscriptvalue_cast<QGraphicsScene*>(ctx->argument(0))) {
        view = new QGraphicsView(scene, qscriptvalue_cast<QWidget*>(ctx->argument(1)));
    } else {
        view = new QGraphicsView(qscriptvalue_cast<QWidget*>(ctx->argument(0)));
    }
    return eng->newQObject(view, QScriptEngine::AutoOwnership);
}

/////////////////////////////////////////////////////////////

static QScriptValue setScene(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsView, setScene);
    self->setScene(qscriptvalue_cast<QGraphicsScene*>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsView, toString);
    return QScriptValue(eng, "QGraphicsView");
}

/////////////////////////////////////////////////////////////

QScriptValue constructGraphicsViewClass(QScriptEngine *eng)
{
    QScriptValue proto = eng->newQObject(new QGraphicsView(), QScriptEngine::AutoOwnership);
    ADD_PROTO_FUNCTION(proto, setScene);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QGraphicsView*>(), proto);

    return eng->newFunction(ctor, proto);
}
