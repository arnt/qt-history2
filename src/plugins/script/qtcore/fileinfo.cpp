#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include "../global.h"

Q_DECLARE_METATYPE(QFileInfo)
Q_DECLARE_METATYPE(QFileInfo*)
Q_DECLARE_METATYPE(QFileInfoList)
Q_DECLARE_METATYPE(QFile*)
Q_DECLARE_METATYPE(QDir)
Q_DECLARE_METATYPE(QDir*)

static inline QScriptValue newFileInfo(QScriptEngine *eng, const QFileInfo &info)
{
    return eng->newVariant(qVariantFromValue(info));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue arg = ctx->argument(0);
    if (arg.isUndefined())
        return newFileInfo(eng, QFileInfo());
    else if (QFile *file = qscriptvalue_cast<QFile*>(arg))
        return newFileInfo(eng, QFileInfo(*file));
    else if (QDir *dir = qscriptvalue_cast<QDir*>(arg))
        return newFileInfo(eng, QFileInfo(*dir, ctx->argument(1).toString()));
    else if (QFileInfo *other = qscriptvalue_cast<QFileInfo*>(arg))
        return newFileInfo(eng, QFileInfo(*other));
    return newFileInfo(eng, arg.toString());
}

/////////////////////////////////////////////////////////////

static QScriptValue absoluteDir(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, absoluteDir);
    return eng->toScriptValue(self->absoluteDir());
}

/////////////////////////////////////////////////////////////

static QScriptValue absoluteFilePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, absoluteFilePath);
    return QScriptValue(eng, self->absoluteFilePath());
}

/////////////////////////////////////////////////////////////

static QScriptValue absolutePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, absolutePath);
    return QScriptValue(eng, self->absolutePath());
}

/////////////////////////////////////////////////////////////

static QScriptValue baseName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, baseName);
    return QScriptValue(eng, self->baseName());
}

/////////////////////////////////////////////////////////////

static QScriptValue bundleName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, bundleName);
    return QScriptValue(eng, self->bundleName());
}

/////////////////////////////////////////////////////////////

static QScriptValue caching(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, caching);
    return QScriptValue(eng, self->caching());
}

/////////////////////////////////////////////////////////////

static QScriptValue canonicalFilePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, canonicalFilePath);
    return QScriptValue(eng, self->canonicalFilePath());
}

/////////////////////////////////////////////////////////////

static QScriptValue canonicalPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, canonicalPath);
    return QScriptValue(eng, self->canonicalPath());
}

/////////////////////////////////////////////////////////////

static QScriptValue completeBaseName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, completeBaseName);
    return QScriptValue(eng, self->completeBaseName());
}

/////////////////////////////////////////////////////////////

static QScriptValue completeSuffix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, completeSuffix);
    return QScriptValue(eng, self->completeSuffix());
}

/////////////////////////////////////////////////////////////

static QScriptValue created(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, created);
    return eng->newDate(self->created());
}

/////////////////////////////////////////////////////////////

static QScriptValue dir(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, dir);
    return eng->toScriptValue(self->dir());
}

/////////////////////////////////////////////////////////////

static QScriptValue exists(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, exists);
    return QScriptValue(eng, self->exists());
}

/////////////////////////////////////////////////////////////

static QScriptValue fileName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, fileName);
    return QScriptValue(eng, self->fileName());
}

/////////////////////////////////////////////////////////////

static QScriptValue filePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, filePath);
    return QScriptValue(eng, self->filePath());
}

/////////////////////////////////////////////////////////////

static QScriptValue group(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, group);
    return QScriptValue(eng, self->group());
}

/////////////////////////////////////////////////////////////

static QScriptValue groupId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, groupId);
    return QScriptValue(eng, self->groupId());
}

/////////////////////////////////////////////////////////////

static QScriptValue isAbsolute(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isAbsolute);
    return QScriptValue(eng, self->isAbsolute());
}

/////////////////////////////////////////////////////////////

static QScriptValue isBundle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isBundle);
    return QScriptValue(eng, self->isBundle());
}

/////////////////////////////////////////////////////////////

static QScriptValue isDir(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isDir);
    return QScriptValue(eng, self->isDir());
}

/////////////////////////////////////////////////////////////

static QScriptValue isExecutable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isExecutable);
    return QScriptValue(eng, self->isExecutable());
}

/////////////////////////////////////////////////////////////

static QScriptValue isFile(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isFile);
    return QScriptValue(eng, self->isFile());
}

/////////////////////////////////////////////////////////////

static QScriptValue isHidden(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isHidden);
    return QScriptValue(eng, self->isHidden());
}

/////////////////////////////////////////////////////////////

static QScriptValue isReadable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isReadable);
    return QScriptValue(eng, self->isReadable());
}

/////////////////////////////////////////////////////////////

static QScriptValue isRelative(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isRelative);
    return QScriptValue(eng, self->isRelative());
}

/////////////////////////////////////////////////////////////

static QScriptValue isRoot(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isRoot);
    return QScriptValue(eng, self->isRoot());
}

/////////////////////////////////////////////////////////////

static QScriptValue isSymLink(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isSymLink);
    return QScriptValue(eng, self->isSymLink());
}

/////////////////////////////////////////////////////////////

static QScriptValue isWritable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, isWritable);
    return QScriptValue(eng, self->isWritable());
}

/////////////////////////////////////////////////////////////

static QScriptValue lastModified(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, lastModified);
    return eng->newDate(self->lastModified());
}

/////////////////////////////////////////////////////////////

static QScriptValue lastRead(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, lastRead);
    return eng->newDate(self->lastRead());
}

/////////////////////////////////////////////////////////////

static QScriptValue makeAbsolute(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, makeAbsolute);
    return QScriptValue(eng, self->makeAbsolute());
}

/////////////////////////////////////////////////////////////

static QScriptValue owner(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, owner);
    return QScriptValue(eng, self->owner());
}

/////////////////////////////////////////////////////////////

static QScriptValue ownerId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, ownerId);
    return QScriptValue(eng, self->ownerId());
}

/////////////////////////////////////////////////////////////

static QScriptValue path(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, path);
    return QScriptValue(eng, self->path());
}

/////////////////////////////////////////////////////////////

static QScriptValue permission(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, permission);
    return QScriptValue(eng, self->permission(QFile::Permissions(ctx->argument(0).toInt32())));
}

/////////////////////////////////////////////////////////////

static QScriptValue permissions(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, permissions);
    return QScriptValue(eng, self->permissions());
}

/////////////////////////////////////////////////////////////

static QScriptValue refresh(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, refresh);
    self->refresh();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setCaching(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, setCaching);
    self->setCaching(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFile(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, setFile);
    QScriptValue arg = ctx->argument(0);
    if (QFile *file = qscriptvalue_cast<QFile*>(arg))
        self->setFile(*file);
    else if (QDir *dir = qscriptvalue_cast<QDir*>(arg))
        self->setFile(*dir, ctx->argument(1).toString());
    else
        self->setFile(arg.toString());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue size(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, size);
    return QScriptValue(eng, double(self->size()));
}

/////////////////////////////////////////////////////////////

static QScriptValue suffix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, suffix);
    return QScriptValue(eng, self->suffix());
}

/////////////////////////////////////////////////////////////

static QScriptValue symLinkTarget(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, symLinkTarget);
    return QScriptValue(eng, self->symLinkTarget());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFileInfo, toString);
    return QScriptValue(eng, QString::fromLatin1("QFileInfo(%0)").arg(self->filePath()));
}

/////////////////////////////////////////////////////////////

QScriptValue constructFileInfoClass(QScriptEngine *eng)
{
    QScriptValue proto = newFileInfo(eng, QFileInfo());
    ADD_METHOD(proto, absoluteDir);
    ADD_METHOD(proto, absoluteFilePath);
    ADD_METHOD(proto, absolutePath);
    ADD_METHOD(proto, baseName);
    ADD_METHOD(proto, bundleName);
    ADD_METHOD(proto, caching);
    ADD_METHOD(proto, canonicalFilePath);
    ADD_METHOD(proto, canonicalPath);
    ADD_METHOD(proto, completeBaseName);
    ADD_METHOD(proto, completeSuffix);
    ADD_METHOD(proto, created);
    ADD_METHOD(proto, dir);
    ADD_METHOD(proto, exists);
    ADD_METHOD(proto, fileName);
    ADD_METHOD(proto, filePath);
    ADD_METHOD(proto, group);
    ADD_METHOD(proto, groupId);
    ADD_METHOD(proto, isAbsolute);
    ADD_METHOD(proto, isBundle);
    ADD_METHOD(proto, isDir);
    ADD_METHOD(proto, isExecutable);
    ADD_METHOD(proto, isFile);
    ADD_METHOD(proto, isHidden);
    ADD_METHOD(proto, isReadable);
    ADD_METHOD(proto, isRelative);
    ADD_METHOD(proto, isRoot);
    ADD_METHOD(proto, isSymLink);
    ADD_METHOD(proto, isWritable);
    ADD_METHOD(proto, lastModified);
    ADD_METHOD(proto, lastRead);
    ADD_METHOD(proto, makeAbsolute);
    ADD_METHOD(proto, owner);
    ADD_METHOD(proto, ownerId);
    ADD_METHOD(proto, path);
    ADD_METHOD(proto, permission);
    ADD_METHOD(proto, permissions);
    ADD_METHOD(proto, refresh);
    ADD_METHOD(proto, setCaching);
    ADD_METHOD(proto, setFile);
    ADD_METHOD(proto, size);
    ADD_METHOD(proto, suffix);
    ADD_METHOD(proto, symLinkTarget);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QFileInfo>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QFileInfo*>(), proto);

    qScriptRegisterSequenceMetaType<QFileInfoList>(eng);

    return eng->newFunction(ctor, proto);
}
