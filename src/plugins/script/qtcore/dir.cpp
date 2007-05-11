#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtCore/QDir>
#include "../global.h"

Q_DECLARE_METATYPE(QDir)
Q_DECLARE_METATYPE(QDir*)

Q_DECLARE_METATYPE(QFileInfoList)

static inline QScriptValue newDir(QScriptEngine *eng, const QDir &dir)
{
    return eng->newVariant(qVariantFromValue(dir));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newDir(eng, QDir());
    if (QDir *other = qscriptvalue_cast<QDir*>(ctx->argument(0)))
        return newDir(eng, QDir(*other));
    return newDir(eng, QDir(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue absoluteFilePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, absoluteFilePath);
    return QScriptValue(eng, self->absoluteFilePath(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue absolutePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, absolutePath);
    return QScriptValue(eng, self->absolutePath());
}

/////////////////////////////////////////////////////////////

static QScriptValue canonicalPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, canonicalPath);
    return QScriptValue(eng, self->canonicalPath());
}

/////////////////////////////////////////////////////////////

static QScriptValue cd(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, cd);
    return QScriptValue(eng, self->cd(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue cdUp(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, cdUp);
    return QScriptValue(eng, self->cdUp());
}

/////////////////////////////////////////////////////////////

static QScriptValue count(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, count);
    return QScriptValue(eng, self->count());
}

/////////////////////////////////////////////////////////////

static QScriptValue dirName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, dirName);
    return QScriptValue(eng, self->dirName());
}

/////////////////////////////////////////////////////////////

static QScriptValue entryInfoList(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, entryInfoList);
    QDir::Filters filters = QDir::NoFilter;
    QDir::SortFlags sort = QDir::NoSort;
    if (ctx->argument(0).isArray()) {
        QStringList nameFilters = qscriptvalue_cast<QStringList>(ctx->argument(0));
        if (ctx->argumentCount() > 1)
            filters = QDir::Filters(ctx->argument(1).toInt32());
        if (ctx->argumentCount() > 2)
            sort = QDir::SortFlags(ctx->argument(2).toInt32());
        return eng->toScriptValue(self->entryInfoList(nameFilters, filters, sort));
    } else {
        if (ctx->argumentCount() > 0)
            filters = QDir::Filters(ctx->argument(0).toInt32());
        if (ctx->argumentCount() > 1)
            sort = QDir::SortFlags(ctx->argument(1).toInt32());
        return eng->toScriptValue(self->entryInfoList(filters, sort));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue entryList(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, entryList);
    QDir::Filters filters = QDir::NoFilter;
    QDir::SortFlags sort = QDir::NoSort;
    if (ctx->argument(0).isArray()) {
        QStringList nameFilters = qscriptvalue_cast<QStringList>(ctx->argument(0));
        if (ctx->argumentCount() > 1)
            filters = QDir::Filters(ctx->argument(1).toInt32());
        if (ctx->argumentCount() > 2)
            sort = QDir::SortFlags(ctx->argument(2).toInt32());
        return eng->toScriptValue(self->entryList(nameFilters, filters, sort));
    } else {
        if (ctx->argumentCount() > 0)
            filters = QDir::Filters(ctx->argument(0).toInt32());
        if (ctx->argumentCount() > 1)
            sort = QDir::SortFlags(ctx->argument(1).toInt32());
        return eng->toScriptValue(self->entryList(filters, sort));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue exists(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, exists);
    if (ctx->argumentCount() == 0)
        return QScriptValue(eng, self->exists());
    return QScriptValue(eng, self->exists(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue filePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, filePath);
    return QScriptValue(eng, self->filePath(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue filter(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, filter);
    return QScriptValue(eng, self->filter());
}

/////////////////////////////////////////////////////////////

static QScriptValue isAbsolute(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, isAbsolute);
    return QScriptValue(eng, self->isAbsolute());
}

/////////////////////////////////////////////////////////////

static QScriptValue isReadable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, isReadable);
    return QScriptValue(eng, self->isReadable());
}

/////////////////////////////////////////////////////////////

static QScriptValue isRelative(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, isRelative);
    return QScriptValue(eng, self->isRelative());
}

/////////////////////////////////////////////////////////////

static QScriptValue isRoot(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, isRoot);
    return QScriptValue(eng, self->isRoot());
}

/////////////////////////////////////////////////////////////

static QScriptValue makeAbsolute(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, makeAbsolute);
    return QScriptValue(eng, self->makeAbsolute());
}

/////////////////////////////////////////////////////////////

static QScriptValue mkdir(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, mkdir);
    return QScriptValue(eng, self->mkdir(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue mkpath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, mkpath);
    return QScriptValue(eng, self->mkpath(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue nameFilters(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, nameFilters);
    return eng->toScriptValue(self->nameFilters());
}

/////////////////////////////////////////////////////////////

static QScriptValue path(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, path);
    return QScriptValue(eng, self->path());
}

/////////////////////////////////////////////////////////////

static QScriptValue refresh(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, refresh);
    self->refresh();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue relativeFilePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, relativeFilePath);
    return QScriptValue(eng, self->relativeFilePath(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue remove(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, remove);
    return QScriptValue(eng, self->remove(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue rename(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, rename);
    return QScriptValue(eng, self->rename(ctx->argument(0).toString(),
                                          ctx->argument(1).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue rmdir(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, rmdir);
    return QScriptValue(eng, self->rmdir(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue rmpath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, rmpath);
    return QScriptValue(eng, self->rmpath(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue setFilter(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, setFilter);
    self->setFilter(QDir::Filters(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setNameFilters(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, setNameFilters);
    self->setNameFilters(qscriptvalue_cast<QStringList>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, setPath);
    self->setPath(ctx->argument(0).toString());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setSorting(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, setSorting);
    self->setSorting(QDir::SortFlags(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue sorting(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, sorting);
    return QScriptValue(eng, static_cast<int>(self->sorting()));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Dir, toString);
    return QScriptValue(eng, QString::fromLatin1("Dir(%0)").arg(self->path()));
}

/////////////////////////////////////////////////////////////

static QScriptValue addSearchPath(QScriptContext *ctx, QScriptEngine *eng)
{
    QDir::addSearchPath(ctx->argument(0).toString(), ctx->argument(1).toString());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue cleanPath(QScriptContext *ctx, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::cleanPath(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue current(QScriptContext *, QScriptEngine *eng)
{
    return newDir(eng, QDir::current());
}

/////////////////////////////////////////////////////////////

static QScriptValue currentPath(QScriptContext *, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::currentPath());
}

/////////////////////////////////////////////////////////////

static QScriptValue drives(QScriptContext *, QScriptEngine *eng)
{
    return eng->toScriptValue(QDir::drives());
}

/////////////////////////////////////////////////////////////

static QScriptValue fromNativeSeparators(QScriptContext *ctx, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::fromNativeSeparators(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue home(QScriptContext *, QScriptEngine *eng)
{
    return newDir(eng, QDir::home());
}

/////////////////////////////////////////////////////////////

static QScriptValue homePath(QScriptContext *, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::homePath());
}

/////////////////////////////////////////////////////////////

static QScriptValue isAbsolutePath(QScriptContext *ctx, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::isAbsolutePath(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue isRelativePath(QScriptContext *ctx, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::isRelativePath(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue match(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argument(0).isArray()) {
        return QScriptValue(eng, QDir::match(qscriptvalue_cast<QStringList>(ctx->argument(0)),
                                             ctx->argument(1).toString()));
    } else {
        return QScriptValue(eng, QDir::match(ctx->argument(0).toString(),
                                             ctx->argument(1).toString()));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue root(QScriptContext *, QScriptEngine *eng)
{
    return newDir(eng, QDir::root());
}

/////////////////////////////////////////////////////////////

static QScriptValue rootPath(QScriptContext *, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::rootPath());
}

/////////////////////////////////////////////////////////////

static QScriptValue searchPaths(QScriptContext *ctx, QScriptEngine *eng)
{
    return eng->toScriptValue(ctx->argument(0).toString());
}

/////////////////////////////////////////////////////////////

static QScriptValue separator(QScriptContext *, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::separator());
}

/////////////////////////////////////////////////////////////

static QScriptValue setCurrent(QScriptContext *ctx, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::setCurrent(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue setSearchPaths(QScriptContext *ctx, QScriptEngine *eng)
{
    QDir::setSearchPaths(ctx->argument(0).toString(),
                         qscriptvalue_cast<QStringList>(ctx->argument(1)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue temp(QScriptContext *, QScriptEngine *eng)
{
    return newDir(eng, QDir::temp());
}

/////////////////////////////////////////////////////////////

static QScriptValue tempPath(QScriptContext *, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::tempPath());
}

/////////////////////////////////////////////////////////////

static QScriptValue toNativeSeparators(QScriptContext *ctx, QScriptEngine *eng)
{
    return QScriptValue(eng, QDir::toNativeSeparators(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

QScriptValue constructDirClass(QScriptEngine *eng)
{
    QScriptValue proto = newDir(eng, QDir());
    ADD_PROTO_FUNCTION(proto, absoluteFilePath);
    ADD_PROTO_FUNCTION(proto, absolutePath);
    ADD_PROTO_FUNCTION(proto, canonicalPath);
    ADD_PROTO_FUNCTION(proto, cd);
    ADD_PROTO_FUNCTION(proto, cdUp);
    ADD_PROTO_FUNCTION(proto, count);
    ADD_PROTO_FUNCTION(proto, dirName);
    ADD_PROTO_FUNCTION(proto, entryInfoList);
    ADD_PROTO_FUNCTION(proto, entryList);
    ADD_PROTO_FUNCTION(proto, exists);
    ADD_PROTO_FUNCTION(proto, filePath);
    ADD_PROTO_FUNCTION(proto, filter);
    ADD_PROTO_FUNCTION(proto, isAbsolute);
    ADD_PROTO_FUNCTION(proto, isReadable);
    ADD_PROTO_FUNCTION(proto, isRelative);
    ADD_PROTO_FUNCTION(proto, isRoot);
    ADD_PROTO_FUNCTION(proto, makeAbsolute);
    ADD_PROTO_FUNCTION(proto, mkdir);
    ADD_PROTO_FUNCTION(proto, mkpath);
    ADD_PROTO_FUNCTION(proto, nameFilters);
    ADD_PROTO_FUNCTION(proto, path);
    ADD_PROTO_FUNCTION(proto, refresh);
    ADD_PROTO_FUNCTION(proto, relativeFilePath);
    ADD_PROTO_FUNCTION(proto, remove);
    ADD_PROTO_FUNCTION(proto, rename);
    ADD_PROTO_FUNCTION(proto, rmdir);
    ADD_PROTO_FUNCTION(proto, rmpath);
    ADD_PROTO_FUNCTION(proto, setFilter);
    ADD_PROTO_FUNCTION(proto, setNameFilters);
    ADD_PROTO_FUNCTION(proto, setPath);
    ADD_PROTO_FUNCTION(proto, setSorting);
    ADD_PROTO_FUNCTION(proto, sorting);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QDir>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QDir*>(), proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    ADD_CTOR_FUNCTION(ctorFun, addSearchPath);
    ADD_CTOR_FUNCTION(ctorFun, cleanPath);
    ADD_CTOR_FUNCTION(ctorFun, current);
    ADD_CTOR_FUNCTION(ctorFun, currentPath);
    ADD_CTOR_FUNCTION(ctorFun, drives);
    ADD_CTOR_FUNCTION(ctorFun, fromNativeSeparators);
    ADD_CTOR_FUNCTION(ctorFun, home);
    ADD_CTOR_FUNCTION(ctorFun, homePath);
    ADD_CTOR_FUNCTION(ctorFun, isAbsolutePath);
    ADD_CTOR_FUNCTION(ctorFun, isRelativePath);
    ADD_CTOR_FUNCTION(ctorFun, match);
    ADD_CTOR_FUNCTION(ctorFun, root);
    ADD_CTOR_FUNCTION(ctorFun, rootPath);
    ADD_CTOR_FUNCTION(ctorFun, searchPaths);
    ADD_CTOR_FUNCTION(ctorFun, separator);
    ADD_CTOR_FUNCTION(ctorFun, setCurrent);
    ADD_CTOR_FUNCTION(ctorFun, setSearchPaths);
    ADD_CTOR_FUNCTION(ctorFun, temp);
    ADD_CTOR_FUNCTION(ctorFun, tempPath);
    ADD_CTOR_FUNCTION(ctorFun, toNativeSeparators);

    ADD_ENUM_VALUE(ctorFun, QDir, Dirs);
    ADD_ENUM_VALUE(ctorFun, QDir, AllDirs);
    ADD_ENUM_VALUE(ctorFun, QDir, Files);
    ADD_ENUM_VALUE(ctorFun, QDir, Drives);
    ADD_ENUM_VALUE(ctorFun, QDir, NoSymLinks);
    ADD_ENUM_VALUE(ctorFun, QDir, NoDotAndDotDot);
    ADD_ENUM_VALUE(ctorFun, QDir, AllEntries);
    ADD_ENUM_VALUE(ctorFun, QDir, Readable);
    ADD_ENUM_VALUE(ctorFun, QDir, Writable);
    ADD_ENUM_VALUE(ctorFun, QDir, Executable);
    ADD_ENUM_VALUE(ctorFun, QDir, Modified);
    ADD_ENUM_VALUE(ctorFun, QDir, Hidden);
    ADD_ENUM_VALUE(ctorFun, QDir, System);
    ADD_ENUM_VALUE(ctorFun, QDir, CaseSensitive);

    ADD_ENUM_VALUE(ctorFun, QDir, Name);
    ADD_ENUM_VALUE(ctorFun, QDir, Time);
    ADD_ENUM_VALUE(ctorFun, QDir, Size);
    ADD_ENUM_VALUE(ctorFun, QDir, Type);
    ADD_ENUM_VALUE(ctorFun, QDir, Unsorted);
    ADD_ENUM_VALUE(ctorFun, QDir, DirsFirst);
    ADD_ENUM_VALUE(ctorFun, QDir, DirsLast);
    ADD_ENUM_VALUE(ctorFun, QDir, Reversed);
    ADD_ENUM_VALUE(ctorFun, QDir, IgnoreCase);
    ADD_ENUM_VALUE(ctorFun, QDir, LocaleAware);

    return ctorFun;
}
