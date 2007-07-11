#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include "../global.h"

Q_DECLARE_METATYPE(QSettings*)

static QScriptValue newSettings(QScriptEngine *eng, QSettings *settings)
{
    return eng->newQObject(settings, QScriptEngine::AutoOwnership);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QString organization = ctx->argument(0).toString();
    QSettings *settings;
    if (organization.contains(QChar('/')) || organization.contains(QChar('\\'))) {
        // This looks like it is a real filename.
        QSettings::Format format;
        if (ctx->argumentCount() >= 2)
            format = (QSettings::Format)(ctx->argument(1).toInt32());
        else
            format = QSettings::NativeFormat;
        settings = new QSettings(organization, format);
    } else {
        // Assume that this is an organization-based settings request.
        QString application = ctx->argument(1).toString();
        QSettings::Scope scope = QSettings::UserScope;
        if (ctx->argumentCount() >= 3)
            scope = (QSettings::Scope)(ctx->argument(2).toInt32());
        settings = new QSettings(scope, organization, application);
    }
    return newSettings(eng, settings);
}

static QScriptValue clear(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, clear);
    self->clear();
    return eng->undefinedValue();
}

static QScriptValue sync(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, sync);
    self->sync();
    return eng->undefinedValue();
}

static QScriptValue status(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, status);
    return QScriptValue(eng, (int)(self->status()));
}

static QScriptValue beginGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, beginGroup);
    self->beginGroup(ctx->argument(0).toString());
    return eng->undefinedValue();
}

static QScriptValue endGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, endGroup);
    self->endGroup();
    return eng->undefinedValue();
}

static QScriptValue group(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, group);
    return QScriptValue(eng, self->group());
}

static QScriptValue beginReadArray(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, beginReadArray);
    return QScriptValue(eng, self->beginReadArray(ctx->argument(0).toString()));
}

static QScriptValue beginWriteArray(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, beginWriteArray);
    if (ctx->argumentCount() < 2)
        self->beginWriteArray(ctx->argument(0).toString());
    else
        self->beginWriteArray(ctx->argument(0).toString(), ctx->argument(1).toInt32());
    return eng->undefinedValue();
}

static QScriptValue endArray(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, endArray);
    self->endArray();
    return eng->undefinedValue();
}

static QScriptValue setArrayIndex(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, setArrayIndex);
    self->setArrayIndex(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

static QScriptValue allKeys(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, allKeys);
    return qScriptValueFromValue(eng, self->allKeys());
}

static QScriptValue childKeys(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, childKeys);
    return qScriptValueFromValue(eng, self->childKeys());
}

static QScriptValue childGroups(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, childGroups);
    return qScriptValueFromValue(eng, self->childGroups());
}

static QScriptValue isWritable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, isWritable);
    return QScriptValue(eng, self->isWritable());
}

static QScriptValue setValue(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, setValue);
    if (ctx->argument(1).isUndefined()) {
        // Write undefined values as the empty string, not "undefined".
        self->setValue(ctx->argument(0).toString(), QString());
    } else {
        self->setValue(ctx->argument(0).toString(), ctx->argument(1).toString());
    }
    return eng->undefinedValue();
}

static QScriptValue value(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, value);
    QString key = ctx->argument(0).toString();
    if (ctx->argumentCount() < 2)
        return QScriptValue(eng, self->value(key).toString());
    else
        return QScriptValue(eng, self->value(key, ctx->argument(1).toString()).toString());
}

static QScriptValue remove(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, remove);
    self->remove(ctx->argument(0).toString());
    return eng->undefinedValue();
}

static QScriptValue contains(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, contains);
    return QScriptValue(eng, self->contains(ctx->argument(0).toString()));
}

static QScriptValue setFallbacksEnabled(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, setFallbacksEnabled);
    self->setFallbacksEnabled(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

static QScriptValue fallbacksEnabled(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, fallbacksEnabled);
    return QScriptValue(eng, self->fallbacksEnabled());
}

static QScriptValue fileName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSettings, fileName);
    return QScriptValue(eng, self->fileName());
}

QScriptValue constructSettingsClass(QScriptEngine *eng)
{
    QScriptValue proto = eng->newObject();
    ADD_METHOD(proto, clear); 
    ADD_METHOD(proto, sync); 
    ADD_METHOD(proto, status); 
    ADD_METHOD(proto, beginGroup); 
    ADD_METHOD(proto, endGroup); 
    ADD_METHOD(proto, group); 
    ADD_METHOD(proto, beginReadArray); 
    ADD_METHOD(proto, beginWriteArray); 
    ADD_METHOD(proto, endArray); 
    ADD_METHOD(proto, setArrayIndex); 
    ADD_METHOD(proto, allKeys); 
    ADD_METHOD(proto, childKeys); 
    ADD_METHOD(proto, childGroups); 
    ADD_METHOD(proto, isWritable); 
    ADD_METHOD(proto, setValue); 
    ADD_METHOD(proto, value); 
    ADD_METHOD(proto, remove); 
    ADD_METHOD(proto, contains); 
    ADD_METHOD(proto, setFallbacksEnabled); 
    ADD_METHOD(proto, fallbacksEnabled); 
    ADD_METHOD(proto, fileName); 
    eng->setDefaultPrototype(qMetaTypeId<QSettings*>(), proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    ADD_ENUM_VALUE(ctorFun, QSettings, NoError);
    ADD_ENUM_VALUE(ctorFun, QSettings, AccessError);
    ADD_ENUM_VALUE(ctorFun, QSettings, FormatError);

    ADD_ENUM_VALUE(ctorFun, QSettings, NativeFormat);
    ADD_ENUM_VALUE(ctorFun, QSettings, IniFormat);
    ADD_ENUM_VALUE(ctorFun, QSettings, InvalidFormat);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat1);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat2);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat3);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat4);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat5);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat6);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat7);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat8);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat9);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat10);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat11);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat12);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat13);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat14);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat15);
    ADD_ENUM_VALUE(ctorFun, QSettings, CustomFormat16);

    ADD_ENUM_VALUE(ctorFun, QSettings, UserScope);
    ADD_ENUM_VALUE(ctorFun, QSettings, SystemScope);

    return ctorFun;
}
