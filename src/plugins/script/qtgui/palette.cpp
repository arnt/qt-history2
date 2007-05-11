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
    return ctx->throwError("Palette constructor");
}

/////////////////////////////////////////////////////////////

static QScriptValue alternateBase(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, alternateBase);
    return eng->toScriptValue(self->alternateBase());
}

/////////////////////////////////////////////////////////////

static QScriptValue base(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, base);
    return eng->toScriptValue(self->base());
}

/////////////////////////////////////////////////////////////

static QScriptValue brightText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, brightText);
    return eng->toScriptValue(self->brightText());
}

/////////////////////////////////////////////////////////////

static QScriptValue brush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, brush);
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
    DECLARE_SELF(Palette, button);
    return eng->toScriptValue(self->button());
}

/////////////////////////////////////////////////////////////

static QScriptValue buttonText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, buttonText);
    return eng->toScriptValue(self->buttonText());
}

/////////////////////////////////////////////////////////////

static QScriptValue cacheKey(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, cacheKey);
    return eng->toScriptValue(self->cacheKey());
}

/////////////////////////////////////////////////////////////

static QScriptValue color(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, color);
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
    DECLARE_SELF(Palette, currentColorGroup);
    return QScriptValue(eng, static_cast<int>(self->currentColorGroup()));
}

/////////////////////////////////////////////////////////////

static QScriptValue dark(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, dark);
    return eng->toScriptValue(self->dark());
}

/////////////////////////////////////////////////////////////

static QScriptValue highlight(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, highlight);
    return eng->toScriptValue(self->highlight());
}

/////////////////////////////////////////////////////////////

static QScriptValue highlightedText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, highlightedText);
    return eng->toScriptValue(self->highlightedText());
}

/////////////////////////////////////////////////////////////

static QScriptValue isBrushSet(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, isBrushSet);
    QPalette::ColorGroup group = static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32());
    QPalette::ColorRole role = static_cast<QPalette::ColorRole>(ctx->argument(1).toInt32());
    return QScriptValue(eng, self->isBrushSet(group, role));
}

/////////////////////////////////////////////////////////////

static QScriptValue isCopyOf(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, isCopyOf);
    return QScriptValue(eng, self->isCopyOf(qscriptvalue_cast<QPalette>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue isEqual(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, isEqual);
    QPalette::ColorGroup cg1 = static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32());
    QPalette::ColorGroup cg2 = static_cast<QPalette::ColorGroup>(ctx->argument(1).toInt32());
    return QScriptValue(eng, self->isEqual(cg1, cg2));
}

/////////////////////////////////////////////////////////////

static QScriptValue light(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, light);
    return eng->toScriptValue(self->light());
}

/////////////////////////////////////////////////////////////

static QScriptValue linkVisited(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, linkVisited);
    return eng->toScriptValue(self->linkVisited());
}

/////////////////////////////////////////////////////////////

static QScriptValue mid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, mid);
    return eng->toScriptValue(self->mid());
}

/////////////////////////////////////////////////////////////

static QScriptValue midlight(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, midlight);
    return eng->toScriptValue(self->midlight());
}

/////////////////////////////////////////////////////////////

static QScriptValue resolve(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, resolve);
    QPalette other = qscriptvalue_cast<QPalette>(ctx->argument(0));
    return newPalette(eng, self->resolve(other));
}

/////////////////////////////////////////////////////////////

static QScriptValue setBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, setBrush);
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
    DECLARE_SELF(Palette, setColor);
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
    DECLARE_SELF(Palette, setColorGroup);
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
    DECLARE_SELF(Palette, setCurrentColorGroup);
    QPalette::ColorGroup group = static_cast<QPalette::ColorGroup>(ctx->argument(0).toInt32());
    self->setCurrentColorGroup(group);
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue shadow(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, shadow);
    return eng->toScriptValue(self->shadow());
}

/////////////////////////////////////////////////////////////

static QScriptValue text(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, text);
    return eng->toScriptValue(self->text());
}

/////////////////////////////////////////////////////////////

static QScriptValue window(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, window);
    return eng->toScriptValue(self->window());
}

/////////////////////////////////////////////////////////////

static QScriptValue windowText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, windowText);
    return eng->toScriptValue(self->windowText());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Palette, toString);
    return QScriptValue(eng, "Palette");
}

/////////////////////////////////////////////////////////////

QScriptValue constructPaletteClass(QScriptEngine *eng)
{
    QScriptValue proto = newPalette(eng, QPalette());
    ADD_PROTO_FUNCTION(proto, alternateBase);
    ADD_PROTO_FUNCTION(proto, base);
    ADD_PROTO_FUNCTION(proto, brightText);
    ADD_PROTO_FUNCTION(proto, brush);
    ADD_PROTO_FUNCTION(proto, button);
    ADD_PROTO_FUNCTION(proto, buttonText);
    ADD_PROTO_FUNCTION(proto, cacheKey);
    ADD_PROTO_FUNCTION(proto, color);
    ADD_PROTO_FUNCTION(proto, currentColorGroup);
    ADD_PROTO_FUNCTION(proto, dark);
    ADD_PROTO_FUNCTION(proto, highlight);
    ADD_PROTO_FUNCTION(proto, highlightedText);
    ADD_PROTO_FUNCTION(proto, isBrushSet);
    ADD_PROTO_FUNCTION(proto, isCopyOf);
    ADD_PROTO_FUNCTION(proto, isEqual);
    ADD_PROTO_FUNCTION(proto, light);
    ADD_PROTO_FUNCTION(proto, linkVisited);
    ADD_PROTO_FUNCTION(proto, mid);
    ADD_PROTO_FUNCTION(proto, midlight);
    ADD_PROTO_FUNCTION(proto, resolve);
    ADD_PROTO_FUNCTION(proto, setBrush);
    ADD_PROTO_FUNCTION(proto, setColor);
    ADD_PROTO_FUNCTION(proto, setColorGroup);
    ADD_PROTO_FUNCTION(proto, setCurrentColorGroup);
    ADD_PROTO_FUNCTION(proto, shadow);
    ADD_PROTO_FUNCTION(proto, text);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, window);
    ADD_PROTO_FUNCTION(proto, windowText);

    eng->setDefaultPrototype(qMetaTypeId<QPalette>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QPalette*>(), proto);

    return eng->newFunction(ctor, proto);
}
