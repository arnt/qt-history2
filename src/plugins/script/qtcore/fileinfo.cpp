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
    DECLARE_SELF(FileInfo, absoluteDir);
    return eng->toScriptValue(self->absoluteDir());
}

/////////////////////////////////////////////////////////////

static QScriptValue absoluteFilePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, absoluteFilePath);
    return QScriptValue(eng, self->absoluteFilePath());
}

/////////////////////////////////////////////////////////////

static QScriptValue absolutePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, absolutePath);
    return QScriptValue(eng, self->absolutePath());
}

/////////////////////////////////////////////////////////////

static QScriptValue baseName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, baseName);
    return QScriptValue(eng, self->baseName());
}

/////////////////////////////////////////////////////////////

static QScriptValue bundleName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, bundleName);
    return QScriptValue(eng, self->bundleName());
}

/////////////////////////////////////////////////////////////

static QScriptValue caching(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, caching);
    return QScriptValue(eng, self->caching());
}

/////////////////////////////////////////////////////////////

static QScriptValue canonicalFilePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, canonicalFilePath);
    return QScriptValue(eng, self->canonicalFilePath());
}

/////////////////////////////////////////////////////////////

static QScriptValue canonicalPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, canonicalPath);
    return QScriptValue(eng, self->canonicalPath());
}

/////////////////////////////////////////////////////////////

static QScriptValue completeBaseName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, completeBaseName);
    return QScriptValue(eng, self->completeBaseName());
}

/////////////////////////////////////////////////////////////

static QScriptValue completeSuffix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, completeSuffix);
    return QScriptValue(eng, self->completeSuffix());
}

/////////////////////////////////////////////////////////////

static QScriptValue created(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, created);
    return eng->newDate(self->created());
}

/////////////////////////////////////////////////////////////

static QScriptValue dir(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, dir);
    return eng->toScriptValue(self->dir());
}

/////////////////////////////////////////////////////////////

static QScriptValue exists(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, exists);
    return QScriptValue(eng, self->exists());
}

/////////////////////////////////////////////////////////////

static QScriptValue fileName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, fileName);
    return QScriptValue(eng, self->fileName());
}

/////////////////////////////////////////////////////////////

static QScriptValue filePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, filePath);
    return QScriptValue(eng, self->filePath());
}

/////////////////////////////////////////////////////////////

static QScriptValue group(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, group);
    return QScriptValue(eng, self->group());
}

/////////////////////////////////////////////////////////////

static QScriptValue groupId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, groupId);
    return QScriptValue(eng, self->groupId());
}

/////////////////////////////////////////////////////////////

static QScriptValue isAbsolute(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isAbsolute);
    return QScriptValue(eng, self->isAbsolute());
}

/////////////////////////////////////////////////////////////

static QScriptValue isBundle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isBundle);
    return QScriptValue(eng, self->isBundle());
}

/////////////////////////////////////////////////////////////

static QScriptValue isDir(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isDir);
    return QScriptValue(eng, self->isDir());
}

/////////////////////////////////////////////////////////////

static QScriptValue isExecutable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isExecutable);
    return QScriptValue(eng, self->isExecutable());
}

/////////////////////////////////////////////////////////////

static QScriptValue isFile(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isFile);
    return QScriptValue(eng, self->isFile());
}

/////////////////////////////////////////////////////////////

static QScriptValue isHidden(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isHidden);
    return QScriptValue(eng, self->isHidden());
}

/////////////////////////////////////////////////////////////

static QScriptValue isReadable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isReadable);
    return QScriptValue(eng, self->isReadable());
}

/////////////////////////////////////////////////////////////

static QScriptValue isRelative(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isRelative);
    return QScriptValue(eng, self->isRelative());
}

/////////////////////////////////////////////////////////////

static QScriptValue isRoot(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isRoot);
    return QScriptValue(eng, self->isRoot());
}

/////////////////////////////////////////////////////////////

static QScriptValue isSymLink(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isSymLink);
    return QScriptValue(eng, self->isSymLink());
}

/////////////////////////////////////////////////////////////

static QScriptValue isWritable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, isWritable);
    return QScriptValue(eng, self->isWritable());
}

/////////////////////////////////////////////////////////////

static QScriptValue lastModified(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, lastModified);
    return eng->newDate(self->lastModified());
}

/////////////////////////////////////////////////////////////

static QScriptValue lastRead(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, lastRead);
    return eng->newDate(self->lastRead());
}

/////////////////////////////////////////////////////////////

static QScriptValue makeAbsolute(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, makeAbsolute);
    return QScriptValue(eng, self->makeAbsolute());
}

/////////////////////////////////////////////////////////////

static QScriptValue owner(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, owner);
    return QScriptValue(eng, self->owner());
}

/////////////////////////////////////////////////////////////

static QScriptValue ownerId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, ownerId);
    return QScriptValue(eng, self->ownerId());
}

/////////////////////////////////////////////////////////////

static QScriptValue path(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, path);
    return QScriptValue(eng, self->path());
}

/////////////////////////////////////////////////////////////

static QScriptValue permission(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, permission);
    return QScriptValue(eng, self->permission(QFile::Permissions(ctx->argument(0).toInt32())));
}

/////////////////////////////////////////////////////////////

static QScriptValue permissions(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, permissions);
    return QScriptValue(eng, self->permissions());
}

/////////////////////////////////////////////////////////////

static QScriptValue refresh(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, refresh);
    self->refresh();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setCaching(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, setCaching);
    self->setCaching(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFile(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, setFile);
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
    DECLARE_SELF(FileInfo, size);
    return QScriptValue(eng, double(self->size()));
}

/////////////////////////////////////////////////////////////

static QScriptValue suffix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, suffix);
    return QScriptValue(eng, self->suffix());
}

/////////////////////////////////////////////////////////////

static QScriptValue symLinkTarget(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, symLinkTarget);
    return QScriptValue(eng, self->symLinkTarget());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(FileInfo, toString);
    return QScriptValue(eng, QString::fromLatin1("FileInfo(%0)").arg(self->filePath()));
}

/////////////////////////////////////////////////////////////

QScriptValue constructFileInfoClass(QScriptEngine *eng)
{
    QScriptValue proto = newFileInfo(eng, QFileInfo());
    ADD_PROTO_FUNCTION(proto, absoluteDir);
    ADD_PROTO_FUNCTION(proto, absoluteFilePath);
    ADD_PROTO_FUNCTION(proto, absolutePath);
    ADD_PROTO_FUNCTION(proto, baseName);
    ADD_PROTO_FUNCTION(proto, bundleName);
    ADD_PROTO_FUNCTION(proto, caching);
    ADD_PROTO_FUNCTION(proto, canonicalFilePath);
    ADD_PROTO_FUNCTION(proto, canonicalPath);
    ADD_PROTO_FUNCTION(proto, completeBaseName);
    ADD_PROTO_FUNCTION(proto, completeSuffix);
    ADD_PROTO_FUNCTION(proto, created);
    ADD_PROTO_FUNCTION(proto, dir);
    ADD_PROTO_FUNCTION(proto, exists);
    ADD_PROTO_FUNCTION(proto, fileName);
    ADD_PROTO_FUNCTION(proto, filePath);
    ADD_PROTO_FUNCTION(proto, group);
    ADD_PROTO_FUNCTION(proto, groupId);
    ADD_PROTO_FUNCTION(proto, isAbsolute);
    ADD_PROTO_FUNCTION(proto, isBundle);
    ADD_PROTO_FUNCTION(proto, isDir);
    ADD_PROTO_FUNCTION(proto, isExecutable);
    ADD_PROTO_FUNCTION(proto, isFile);
    ADD_PROTO_FUNCTION(proto, isHidden);
    ADD_PROTO_FUNCTION(proto, isReadable);
    ADD_PROTO_FUNCTION(proto, isRelative);
    ADD_PROTO_FUNCTION(proto, isRoot);
    ADD_PROTO_FUNCTION(proto, isSymLink);
    ADD_PROTO_FUNCTION(proto, isWritable);
    ADD_PROTO_FUNCTION(proto, lastModified);
    ADD_PROTO_FUNCTION(proto, lastRead);
    ADD_PROTO_FUNCTION(proto, makeAbsolute);
    ADD_PROTO_FUNCTION(proto, owner);
    ADD_PROTO_FUNCTION(proto, ownerId);
    ADD_PROTO_FUNCTION(proto, path);
    ADD_PROTO_FUNCTION(proto, permission);
    ADD_PROTO_FUNCTION(proto, permissions);
    ADD_PROTO_FUNCTION(proto, refresh);
    ADD_PROTO_FUNCTION(proto, setCaching);
    ADD_PROTO_FUNCTION(proto, setFile);
    ADD_PROTO_FUNCTION(proto, size);
    ADD_PROTO_FUNCTION(proto, suffix);
    ADD_PROTO_FUNCTION(proto, symLinkTarget);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QFileInfo>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QFileInfo*>(), proto);

    qScriptRegisterSequenceMetaType<QFileInfoList>(eng);

    return eng->newFunction(ctor, proto);
}
