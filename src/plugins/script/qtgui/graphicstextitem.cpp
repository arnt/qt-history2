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
    DECLARE_SELF(QGraphicsTextItem, adjustSize);
    self->adjustSize();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue defaultTextColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, defaultTextColor);
    return eng->toScriptValue(self->defaultTextColor());
}

/////////////////////////////////////////////////////////////

static QScriptValue document(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, document);
    return eng->toScriptValue(self->document());
}

/////////////////////////////////////////////////////////////

static QScriptValue font(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, font);
    return eng->toScriptValue(self->font());
}

/////////////////////////////////////////////////////////////

static QScriptValue openExternalLinks(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, openExternalLinks);
    return QScriptValue(eng, self->openExternalLinks());
}

/////////////////////////////////////////////////////////////

static QScriptValue setDefaultTextColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, setDefaultTextColor);
    self->setDefaultTextColor(qscriptvalue_cast<QColor>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setDocument(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, setDocument);
    self->setDocument(qscriptvalue_cast<QTextDocument*>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFont(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, setFont);
    self->setFont(qscriptvalue_cast<QFont>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setHtml(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, setHtml);
    self->setHtml(ctx->argument(0).toString());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setOpenExternalLinks(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, setOpenExternalLinks);
    self->setOpenExternalLinks(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setPlainText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, setPlainText);
    self->setPlainText(ctx->argument(0).toString());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTextCursor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, setTextCursor);
    self->setTextCursor(qscriptvalue_cast<QTextCursor>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTextInteractionFlags(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, setTextInteractionFlags);
    self->setTextInteractionFlags(static_cast<Qt::TextInteractionFlags>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTextWidth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, setTextWidth);
    self->setTextWidth(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue textCursor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, textCursor);
    return eng->toScriptValue(self->textCursor());
}

/////////////////////////////////////////////////////////////

static QScriptValue textInteractionFlags(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, textInteractionFlags);
    return QScriptValue(eng, static_cast<int>(self->textInteractionFlags()));
}

/////////////////////////////////////////////////////////////

static QScriptValue textWidth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, textWidth);
    return QScriptValue(eng, self->textWidth());
}

/////////////////////////////////////////////////////////////

static QScriptValue toHtml(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, toHtml);
    return QScriptValue(eng, self->toHtml());
}

/////////////////////////////////////////////////////////////

static QScriptValue toPlainText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, toPlainText);
    return QScriptValue(eng, self->toPlainText());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsTextItem, toString);
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

    ADD_METHOD(proto, adjustSize);
    ADD_METHOD(proto, defaultTextColor);
    ADD_METHOD(proto, document);
    ADD_METHOD(proto, font);
    ADD_METHOD(proto, openExternalLinks);
    ADD_METHOD(proto, setDefaultTextColor);
    ADD_METHOD(proto, setDocument);
    ADD_METHOD(proto, setFont);
    ADD_METHOD(proto, setHtml);
    ADD_METHOD(proto, setOpenExternalLinks);
    ADD_METHOD(proto, setPlainText);
    ADD_METHOD(proto, setTextCursor);
    ADD_METHOD(proto, setTextInteractionFlags);
    ADD_METHOD(proto, setTextWidth);
    ADD_METHOD(proto, textCursor);
    ADD_METHOD(proto, textInteractionFlags);
    ADD_METHOD(proto, textWidth);
    ADD_METHOD(proto, toHtml);
    ADD_METHOD(proto, toPlainText);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QGraphicsTextItem*>(), proto);

    return eng->newFunction(ctor, proto);
}
