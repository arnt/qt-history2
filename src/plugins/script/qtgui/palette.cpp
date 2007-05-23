#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QPalette>
#include "../global.h"

Q_DECLARE_METATYPE(QPalette)
Q_DECLARE_METATYPE(QPalette*)

static inline QScriptValue newPalette(QScriptEngine *eng, const QPalette &palette)
{
    return eng->newVariant(qVariantFromValue(palette));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newPalette(eng, QPalette());
    if (QPalette *other = qscriptvalue_cast<QPalette*>(ctx->argument(0)))
        return newPalette(eng, QPalette(*other));
    return ctx->throwError("QPalette constructor");
}

/////////////////////////////////////////////////////////////

static QScriptValue alternateBase(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, alternateBase);
    return eng->toScriptValue(self->alternateBase());
}

/////////////////////////////////////////////////////////////

static QScriptValue base(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, base);
    return eng->toScriptValue(self->base());
}

/////////////////////////////////////////////////////////////

static QScriptValue brightText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, brightText);
    return eng->toScriptValue(self->brightText());
}

/////////////////////////////////////////////////////////////

static QScriptValue brush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, brush);
    if (ctx->argumentCount() == 2) {
        QPalette::ColorGroup group = static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32());
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(ctx->argument(1).toInt32());
        return eng->toScriptValue(self->brush(group, role));
    } else {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(ctx->argument(0).toInt32());
        return eng->toScriptValue(self->brush(role));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue button(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, button);
    return eng->toScriptValue(self->button());
}

/////////////////////////////////////////////////////////////

static QScriptValue buttonText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, buttonText);
    return eng->toScriptValue(self->buttonText());
}

/////////////////////////////////////////////////////////////

static QScriptValue cacheKey(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, cacheKey);
    return eng->toScriptValue(self->cacheKey());
}

/////////////////////////////////////////////////////////////

static QScriptValue color(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, color);
    if (ctx->argumentCount() == 2) {
        QPalette::ColorGroup group = static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32());
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(ctx->argument(1).toInt32());
        return eng->toScriptValue(self->color(group, role));
    } else {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(ctx->argument(0).toInt32());
        return eng->toScriptValue(self->color(role));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue currentColorGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, currentColorGroup);
    return QScriptValue(eng, static_cast<int>(self->currentColorGroup()));
}

/////////////////////////////////////////////////////////////

static QScriptValue dark(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, dark);
    return eng->toScriptValue(self->dark());
}

/////////////////////////////////////////////////////////////

static QScriptValue highlight(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, highlight);
    return eng->toScriptValue(self->highlight());
}

/////////////////////////////////////////////////////////////

static QScriptValue highlightedText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, highlightedText);
    return eng->toScriptValue(self->highlightedText());
}

/////////////////////////////////////////////////////////////

static QScriptValue isBrushSet(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, isBrushSet);
    QPalette::ColorGroup group = static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32());
    QPalette::ColorRole role = static_cast<QPalette::ColorRole>(ctx->argument(1).toInt32());
    return QScriptValue(eng, self->isBrushSet(group, role));
}

/////////////////////////////////////////////////////////////

static QScriptValue isCopyOf(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, isCopyOf);
    return QScriptValue(eng, self->isCopyOf(qscriptvalue_cast<QPalette>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue isEqual(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, isEqual);
    QPalette::ColorGroup cg1 = static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32());
    QPalette::ColorGroup cg2 = static_cast<QPalette::ColorGroup>(ctx->argument(1).toInt32());
    return QScriptValue(eng, self->isEqual(cg1, cg2));
}

/////////////////////////////////////////////////////////////

static QScriptValue light(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, light);
    return eng->toScriptValue(self->light());
}

/////////////////////////////////////////////////////////////

static QScriptValue linkVisited(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, linkVisited);
    return eng->toScriptValue(self->linkVisited());
}

/////////////////////////////////////////////////////////////

static QScriptValue mid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, mid);
    return eng->toScriptValue(self->mid());
}

/////////////////////////////////////////////////////////////

static QScriptValue midlight(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, midlight);
    return eng->toScriptValue(self->midlight());
}

/////////////////////////////////////////////////////////////

static QScriptValue resolve(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, resolve);
    QPalette other = qscriptvalue_cast<QPalette>(ctx->argument(0));
    return newPalette(eng, self->resolve(other));
}

/////////////////////////////////////////////////////////////

static QScriptValue setBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, setBrush);
    if (ctx->argumentCount() == 3) {
        QPalette::ColorGroup group = static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32());
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(ctx->argument(1).toInt32());
        QBrush brush = qscriptvalue_cast<QBrush>(ctx->argument(2));
        self->setBrush(group, role, brush);
    } else {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(ctx->argument(0).toInt32());
        QBrush brush = qscriptvalue_cast<QBrush>(ctx->argument(1));
        self->setBrush(role, brush);
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, setColor);
    if (ctx->argumentCount() == 3) {
        QPalette::ColorGroup group = static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32());
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(ctx->argument(1).toInt32());
        QColor color = qscriptvalue_cast<QColor>(ctx->argument(2));
        self->setColor(group, role, color);
    } else {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(ctx->argument(0).toInt32());
        QColor color = qscriptvalue_cast<QColor>(ctx->argument(1));
        self->setColor(role, color);
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setColorGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, setColorGroup);
    self->setColorGroup(static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32()),
                        qscriptvalue_cast<QBrush>(ctx->argument(1)),
                        qscriptvalue_cast<QBrush>(ctx->argument(2)),
                        qscriptvalue_cast<QBrush>(ctx->argument(3)),
                        qscriptvalue_cast<QBrush>(ctx->argument(4)),
                        qscriptvalue_cast<QBrush>(ctx->argument(5)),
                        qscriptvalue_cast<QBrush>(ctx->argument(6)),
                        qscriptvalue_cast<QBrush>(ctx->argument(7)),
                        qscriptvalue_cast<QBrush>(ctx->argument(8)),
                        qscriptvalue_cast<QBrush>(ctx->argument(9)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setCurrentColorGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, setCurrentColorGroup);
    QPalette::ColorGroup group = static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32());
    self->setCurrentColorGroup(group);
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue shadow(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, shadow);
    return eng->toScriptValue(self->shadow());
}

/////////////////////////////////////////////////////////////

static QScriptValue text(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, text);
    return eng->toScriptValue(self->text());
}

/////////////////////////////////////////////////////////////

static QScriptValue window(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, window);
    return eng->toScriptValue(self->window());
}

/////////////////////////////////////////////////////////////

static QScriptValue windowText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, windowText);
    return eng->toScriptValue(self->windowText());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPalette, toString);
    return QScriptValue(eng, "QPalette");
}

/////////////////////////////////////////////////////////////

QScriptValue constructPaletteClass(QScriptEngine *eng)
{
    QScriptValue proto = newPalette(eng, QPalette());
    ADD_METHOD(proto, alternateBase);
    ADD_METHOD(proto, base);
    ADD_METHOD(proto, brightText);
    ADD_METHOD(proto, brush);
    ADD_METHOD(proto, button);
    ADD_METHOD(proto, buttonText);
    ADD_METHOD(proto, cacheKey);
    ADD_METHOD(proto, color);
    ADD_METHOD(proto, currentColorGroup);
    ADD_METHOD(proto, dark);
    ADD_METHOD(proto, highlight);
    ADD_METHOD(proto, highlightedText);
    ADD_METHOD(proto, isBrushSet);
    ADD_METHOD(proto, isCopyOf);
    ADD_METHOD(proto, isEqual);
    ADD_METHOD(proto, light);
    ADD_METHOD(proto, linkVisited);
    ADD_METHOD(proto, mid);
    ADD_METHOD(proto, midlight);
    ADD_METHOD(proto, resolve);
    ADD_METHOD(proto, setBrush);
    ADD_METHOD(proto, setColor);
    ADD_METHOD(proto, setColorGroup);
    ADD_METHOD(proto, setCurrentColorGroup);
    ADD_METHOD(proto, shadow);
    ADD_METHOD(proto, text);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, window);
    ADD_METHOD(proto, windowText);

    eng->setDefaultPrototype(qMetaTypeId<QPalette>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QPalette*>(), proto);

    return eng->newFunction(ctor, proto);
}
