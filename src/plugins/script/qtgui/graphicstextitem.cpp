#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QFont>
#include <QtGui/QTextCursor>
#include <qdebug.h>
#include "../global.h"

Q_DECLARE_METATYPE(QGraphicsTextItem*)
Q_DECLARE_METATYPE(QTextDocument*)
Q_DECLARE_METATYPE(QTextCursor)

static inline QScriptValue newGraphicsTextItem(QScriptEngine *eng, QGraphicsTextItem *item)
{
    return eng->newQObject(item, QScriptEngine::AutoOwnership);
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if ((ctx->argumentCount() > 1) || ctx->argument(0).isString()) {
        return newGraphicsTextItem(eng,
            new QGraphicsTextItem(ctx->argument(0).toString(),
                                  qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        return newGraphicsTextItem(eng,
            new QGraphicsTextItem(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue adjustSize(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, adjustSize);
    self->adjustSize();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue defaultTextColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, defaultTextColor);
    return eng->toScriptValue(self->defaultTextColor());
}

/////////////////////////////////////////////////////////////

static QScriptValue document(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, document);
    return eng->toScriptValue(self->document());
}

/////////////////////////////////////////////////////////////

static QScriptValue font(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, font);
    return eng->toScriptValue(self->font());
}

/////////////////////////////////////////////////////////////

static QScriptValue openExternalLinks(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, openExternalLinks);
    return QScriptValue(eng, self->openExternalLinks());
}

/////////////////////////////////////////////////////////////

static QScriptValue setDefaultTextColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, setDefaultTextColor);
    self->setDefaultTextColor(qscriptvalue_cast<QColor>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setDocument(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, setDocument);
    self->setDocument(qscriptvalue_cast<QTextDocument*>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFont(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, setFont);
    self->setFont(qscriptvalue_cast<QFont>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setHtml(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, setHtml);
    self->setHtml(ctx->argument(0).toString());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setOpenExternalLinks(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, setOpenExternalLinks);
    self->setOpenExternalLinks(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setPlainText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, setPlainText);
    self->setPlainText(ctx->argument(0).toString());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTextCursor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, setTextCursor);
    self->setTextCursor(qscriptvalue_cast<QTextCursor>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTextInteractionFlags(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, setTextInteractionFlags);
    self->setTextInteractionFlags(static_cast<Qt::TextInteractionFlags>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTextWidth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, setTextWidth);
    self->setTextWidth(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue textCursor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, textCursor);
    return eng->toScriptValue(self->textCursor());
}

/////////////////////////////////////////////////////////////

static QScriptValue textInteractionFlags(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, textInteractionFlags);
    return QScriptValue(eng, static_cast<int>(self->textInteractionFlags()));
}

/////////////////////////////////////////////////////////////

static QScriptValue textWidth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, textWidth);
    return QScriptValue(eng, self->textWidth());
}

/////////////////////////////////////////////////////////////

static QScriptValue toHtml(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, toHtml);
    return QScriptValue(eng, self->toHtml());
}

/////////////////////////////////////////////////////////////

static QScriptValue toPlainText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, toPlainText);
    return QScriptValue(eng, self->toPlainText());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsTextItem, toString);
    return QScriptValue(eng, "QGraphicsTextItem");
}

/////////////////////////////////////////////////////////////

QScriptValue constructGraphicsTextItemClass(QScriptEngine *eng)
{
    QScriptValue proto = newGraphicsTextItem(eng, new QGraphicsTextItem());
    // We can't set the standard QGraphicsItem prototype as prototype of
    // our prototype, because QGraphicsTextItem is a QObject. So, copy the
    // functions from the QGraphicsItem prototype.
    QScriptValueIterator it(eng->defaultPrototype(qMetaTypeId<QGraphicsItem*>()));
    while (it.hasNext()) {
        it.next();
        proto.setProperty(it.name(), it.value());
    }

    ADD_PROTO_FUNCTION(proto, adjustSize);
    ADD_PROTO_FUNCTION(proto, defaultTextColor);
    ADD_PROTO_FUNCTION(proto, document);
    ADD_PROTO_FUNCTION(proto, font);
    ADD_PROTO_FUNCTION(proto, openExternalLinks);
    ADD_PROTO_FUNCTION(proto, setDefaultTextColor);
    ADD_PROTO_FUNCTION(proto, setDocument);
    ADD_PROTO_FUNCTION(proto, setFont);
    ADD_PROTO_FUNCTION(proto, setHtml);
    ADD_PROTO_FUNCTION(proto, setOpenExternalLinks);
    ADD_PROTO_FUNCTION(proto, setPlainText);
    ADD_PROTO_FUNCTION(proto, setTextCursor);
    ADD_PROTO_FUNCTION(proto, setTextInteractionFlags);
    ADD_PROTO_FUNCTION(proto, setTextWidth);
    ADD_PROTO_FUNCTION(proto, textCursor);
    ADD_PROTO_FUNCTION(proto, textInteractionFlags);
    ADD_PROTO_FUNCTION(proto, textWidth);
    ADD_PROTO_FUNCTION(proto, toHtml);
    ADD_PROTO_FUNCTION(proto, toPlainText);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QGraphicsTextItem*>(), proto);

    return eng->newFunction(ctor, proto);
}
