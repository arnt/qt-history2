#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QtCore/QTimeLine>
#include "../global.h"

Q_DECLARE_METATYPE(QTimeLine*)

static QScriptValue newTimeLine(QScriptEngine *eng, QTimeLine *timeLine)
{
    return eng->newQObject(timeLine, QScriptEngine::AutoOwnership);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0) {
        return newTimeLine(eng, new QTimeLine());
    } else {
        return newTimeLine(eng, new QTimeLine(ctx->argument(0).toInt32(),
                                              qscriptvalue_cast<QObject*>(ctx->argument(1))));
    }
}

static QScriptValue currentFrame(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, currentFrame);
    return QScriptValue(eng, self->currentFrame());
}

static QScriptValue currentValue(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, currentValue);
    return QScriptValue(eng, self->currentValue());
}

static QScriptValue endFrame(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, endFrame);
    return QScriptValue(eng, self->endFrame());
}

static QScriptValue frameForTime(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, frameForTime);
    return QScriptValue(eng, self->frameForTime(ctx->argument(0).toInt32()));
}

static QScriptValue setCurveShape(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, setCurveShape);
    self->setCurveShape(QTimeLine::CurveShape(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

static QScriptValue setEndFrame(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, setEndFrame);
    self->setEndFrame(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

static QScriptValue setFrameRange(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, setFrameRange);
    self->setFrameRange(ctx->argument(0).toInt32(), ctx->argument(1).toInt32());
    return eng->undefinedValue();
}

static QScriptValue setStartFrame(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, setStartFrame);
    self->setStartFrame(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

static QScriptValue startFrame(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, startFrame);
    return QScriptValue(eng, self->startFrame());
}

static QScriptValue state(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, state);
    return QScriptValue(eng, self->state());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(TimeLine, toString);
    return QScriptValue(eng, QString::fromLatin1("TimeLine(duration=%0)")
                        .arg(self->duration()));
}

QScriptValue constructTimeLineClass(QScriptEngine *eng)
{
    QScriptValue proto = newTimeLine(eng, new QTimeLine());
    ADD_PROTO_FUNCTION(proto, currentFrame); 
    ADD_PROTO_FUNCTION(proto, currentValue); 
    ADD_PROTO_FUNCTION(proto, endFrame); 
    ADD_PROTO_FUNCTION(proto, frameForTime);
    ADD_PROTO_FUNCTION(proto, setCurveShape);
    ADD_PROTO_FUNCTION(proto, setEndFrame);
    ADD_PROTO_FUNCTION(proto, setFrameRange);
    ADD_PROTO_FUNCTION(proto, setStartFrame);
    ADD_PROTO_FUNCTION(proto, startFrame);
    ADD_PROTO_FUNCTION(proto, state);
    ADD_PROTO_FUNCTION(proto, toString);
    eng->setDefaultPrototype(qMetaTypeId<QTimeLine*>(), proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    ADD_ENUM_VALUE(ctorFun, QTimeLine, EaseInCurve);
    ADD_ENUM_VALUE(ctorFun, QTimeLine, EaseOutCurve);
    ADD_ENUM_VALUE(ctorFun, QTimeLine, EaseInOutCurve);
    ADD_ENUM_VALUE(ctorFun, QTimeLine, LinearCurve);
    ADD_ENUM_VALUE(ctorFun, QTimeLine, SineCurve);

    ADD_ENUM_VALUE(ctorFun, QTimeLine, Forward);
    ADD_ENUM_VALUE(ctorFun, QTimeLine, Backward);

    ADD_ENUM_VALUE(ctorFun, QTimeLine, NotRunning);
    ADD_ENUM_VALUE(ctorFun, QTimeLine, Paused);
    ADD_ENUM_VALUE(ctorFun, QTimeLine, Running);

    return ctorFun;
}
