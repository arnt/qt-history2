#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QtCore/QFile>
#include "../global.h"

Q_DECLARE_METATYPE(QIODevice*)
Q_DECLARE_METATYPE(QFile*)

class FileConstructor : public QObject,
                        public QScriptable
{
    Q_OBJECT

    Q_PROPERTY(int NoError READ NoError)
    Q_PROPERTY(int ReadError READ ReadError)
    Q_PROPERTY(int WriteError READ WriteError)
    Q_PROPERTY(int FatalError READ FatalError)
    Q_PROPERTY(int ResourceError READ ResourceError)
    Q_PROPERTY(int OpenError READ OpenError)
    Q_PROPERTY(int AbortError READ AbortError)
    Q_PROPERTY(int TimeOutError READ TimeOutError)
    Q_PROPERTY(int UnspecifiedError READ UnspecifiedError)
    Q_PROPERTY(int RemoveError READ RemoveError)
    Q_PROPERTY(int RenameError READ RenameError)
    Q_PROPERTY(int PositionError READ PositionError)
    Q_PROPERTY(int ResizeError READ ResizeError)
    Q_PROPERTY(int PermissionsError READ PermissionsError)
    Q_PROPERTY(int CopyError READ CopyError)

    Q_PROPERTY(int ReadOwner READ ReadOwner)
    Q_PROPERTY(int WriteOwner READ WriteOwner)
    Q_PROPERTY(int ExeOwner READ ExeOwner)
    Q_PROPERTY(int ReadUser READ ReadUser)
    Q_PROPERTY(int WriteUser READ WriteUser)
    Q_PROPERTY(int ExeUser READ ExeUser)
    Q_PROPERTY(int ReadGroup READ ReadGroup)
    Q_PROPERTY(int WriteGroup READ WriteGroup)
    Q_PROPERTY(int ExeGroup READ ExeGroup)
    Q_PROPERTY(int ReadOther READ ReadOther)
    Q_PROPERTY(int WriteOther READ WriteOther)
    Q_PROPERTY(int ExeOther READ ExeOther)

public:
    FileConstructor() : QObject() { }

    int NoError() const { return QFile::NoError; }
    int ReadError() const { return QFile::ReadError; }
    int WriteError() const { return QFile::WriteError; }
    int FatalError() const { return QFile::FatalError; }
    int ResourceError() const { return QFile::ResourceError; }
    int OpenError() const { return QFile::OpenError; }
    int AbortError() const { return QFile::AbortError; }
    int TimeOutError() const { return QFile::TimeOutError; }
    int UnspecifiedError() const { return QFile::UnspecifiedError; }
    int RemoveError() const { return QFile::RemoveError; }
    int RenameError() const { return QFile::RenameError; }
    int PositionError() const { return QFile::PositionError; }
    int ResizeError() const { return QFile::ResizeError; }
    int PermissionsError() const { return QFile::PermissionsError; }
    int CopyError() const { return QFile::CopyError; }

    int ReadOwner() const { return QFile::ReadOwner; }
    int WriteOwner() const { return QFile::WriteOwner; }
    int ExeOwner() const { return QFile::ExeOwner; }
    int ReadUser() const { return QFile::ReadUser; }
    int WriteUser() const { return QFile::WriteUser; }
    int ExeUser() const { return QFile::ExeUser; }
    int ReadGroup() const { return QFile::ReadGroup; }
    int WriteGroup() const { return QFile::WriteGroup; }
    int ExeGroup() const { return QFile::ExeGroup; }
    int ReadOther() const { return QFile::ReadOther; }
    int WriteOther() const { return QFile::WriteOther; }
    int ExeOther() const { return QFile::ExeOther; }

public Q_SLOTS:
    QScriptValue qscript_call(const QString &name)
    { return engine()->newQObject(new QFile(name)); }
    QScriptValue qscript_call(QObject *parent)
    { return engine()->newQObject(new QFile(parent)); }
    QScriptValue qscript_call(const QString &name, QObject *parent)
    { return engine()->newQObject(new QFile(name, parent)); }
    bool copy(const QString &fileName, const QString &newName)
    { return QFile::copy(fileName, newName); }
    bool exists(const QString &fileName)
    { return QFile::exists(fileName); }
    bool link(const QString &fileName, const QString &linkName)
    { return QFile::link(fileName, linkName); }
    int permissions(const QString &fileName)
    { return QFile::permissions(fileName); }
    bool remove(const QString &fileName)
    { return QFile::remove(fileName); }
    bool rename(const QString &oldName, const QString &newName)
    { return QFile::rename(oldName, newName); }
    bool resize(const QString fileName, qint64 sz)
    { return QFile::resize(fileName, sz); }
    bool setPermissions(const QString &fileName, int permissions)
    { return QFile::setPermissions(fileName, QFile::Permissions(permissions)); }
    QString symLinkTarget(const QString &fileName)
    { return QFile::symLinkTarget(fileName); }
    QString toString() const
    {
        return QLatin1String("QFile() { [native code] }");
    }
};

class FilePrototype : public QFile,
                      public QScriptable
{
    Q_OBJECT
    Q_ENUMS(Permission)
    Q_FLAGS(Permissions)
public:
    enum Permission { };
    Q_DECLARE_FLAGS(Permissions, Permission)

    FilePrototype() : QFile() { }

public Q_SLOTS:
    bool copy(const QString &newName)
    {
        DECLARE_SELF2(File, copy, false);
        return self->copy(newName);
    }
    QFile::FileError error() const
    {
        DECLARE_SELF2(File, error, QFile::UnspecifiedError);
        return self->error();
    }
    bool exists() const
    {
        DECLARE_SELF2(File, exists, false);
        return self->exists();
    }
    QString fileName() const
    {
        DECLARE_SELF2(File, fileName, QString());
        return self->fileName();
    }
    bool flush()
    {
        DECLARE_SELF2(File, flush, false);
        return self->flush();
    }
    int handle() const
    {
        DECLARE_SELF2(File, handle, -1);
        return self->handle();
    }
    bool link(const QString &linkName)
    {
        DECLARE_SELF2(File, link, false);
        return self->link(linkName);
    }
    bool open(FILE *fh, OpenMode mode)
    {
        DECLARE_SELF2(File, open, false);
        return self->open(fh, mode);
    }
    bool open(int fd, OpenMode mode)
    {
        DECLARE_SELF2(File, open, false);
        return self->open(fd, mode);
    }
    bool open(int mode)
    {
        DECLARE_SELF2(File, open, false);
        return self->open(QIODevice::OpenMode(mode));
    }
    QFile::Permissions permissions() const
    {
        DECLARE_SELF2(File, permissions, QFile::Permissions(0));
        return self->permissions();
    }
    bool remove()
    {
        DECLARE_SELF2(File, remove, false);
        return self->remove();
    }
    bool rename(const QString &newName)
    {
        DECLARE_SELF2(File, rename, false);
        return self->rename(newName);
    }
    bool resize(qint64 sz)
    {
        DECLARE_SELF2(File, resize, false);
        return self->resize(sz);
    }
    void setFileName(const QString &name)
    {
        DECLARE_SELF2(File, setFileName, );
        self->setFileName(name);
    }
    bool setPermissions(QFile::Permissions permissions)
    {
        DECLARE_SELF2(File, setPermissions, false);
        return self->setPermissions(permissions);
    }
    QString symLinkTarget() const
    {
        DECLARE_SELF2(File, symLinkTarget, QString());
        return self->symLinkTarget();
    }
    void unsetError()
    {
        DECLARE_SELF2(File, unsetError, );
        self->unsetError();
    }
    QString toString() const
    {
        DECLARE_SELF2(File, toString, QString());
        return QString::fromLatin1("QFile(%0)")
            .arg(self->fileName());
    }
};

/////////////////////////////////////////////////////////////

QScriptValue constructFileClass(QScriptEngine *eng)
{
    FilePrototype *fileProtoObject = new FilePrototype();
    QScriptValue proto = eng->newQObject(fileProtoObject, QScriptEngine::ScriptOwnership);
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QIODevice*>()));
    eng->setDefaultPrototype(qMetaTypeId<QFile*>(), proto);

    FileConstructor *fileCtorObject = new FileConstructor();
    QScriptValue ctor = eng->newQObject(fileCtorObject, QScriptEngine::ScriptOwnership);
    ctor.setPrototype(eng->globalObject().property("Function").property("prototype"));
    ctor.setProperty("prototype", proto);
    proto.setProperty("constructor", ctor);
    return ctor;
}

#include "file.moc"
