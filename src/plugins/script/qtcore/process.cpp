#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QtCore/QProcess>
#include "../global.h"

Q_DECLARE_METATYPE(QIODevice*)
Q_DECLARE_METATYPE(QProcess*)

class ProcessConstructor : public QObject,
                           public QScriptable
{
    Q_OBJECT

    Q_PROPERTY(int NormalExit READ NormalExit)
    Q_PROPERTY(int CrashExit READ CrashExit)

    Q_PROPERTY(int StandardOutput READ StandardOutput)
    Q_PROPERTY(int StandardError READ StandardError)

    Q_PROPERTY(int SeparateChannels READ SeparateChannels)
    Q_PROPERTY(int MergedChannels READ MergedChannels)
    Q_PROPERTY(int ForwardedChannels READ ForwardedChannels)

    Q_PROPERTY(int FailedToStart READ FailedToStart)
    Q_PROPERTY(int Crashed READ Crashed)
    Q_PROPERTY(int Timedout READ Timedout)
    Q_PROPERTY(int WriteError READ WriteError)
    Q_PROPERTY(int ReadError READ ReadError)
    Q_PROPERTY(int UnknownError READ UnknownError)

    Q_PROPERTY(int NotRunning READ NotRunning)
    Q_PROPERTY(int Starting READ Starting)
    Q_PROPERTY(int Running READ Running)

public:
    ProcessConstructor() : QObject() { }

    int NormalExit() const { return QProcess::NormalExit; }
    int CrashExit() const { return QProcess::CrashExit; }

    int StandardOutput() const { return QProcess::StandardOutput; }
    int StandardError() const { return QProcess::StandardError; }

    int SeparateChannels() const { return QProcess::SeparateChannels; }
    int MergedChannels() const { return QProcess::MergedChannels; }
    int ForwardedChannels() const { return QProcess::ForwardedChannels; }

    int FailedToStart() const { return QProcess::FailedToStart; }
    int Crashed() const { return QProcess::Crashed; }
    int Timedout() const { return QProcess::Timedout; }
    int WriteError() const { return QProcess::WriteError; }
    int ReadError() const { return QProcess::ReadError; }
    int UnknownError() const { return QProcess::UnknownError; }

    int NotRunning() const { return QProcess::NotRunning; }
    int Starting() const { return QProcess::Starting; }
    int Running() const { return QProcess::Running; }

public Q_SLOTS:
    QScriptValue qscript_call(QObject *parent = 0)
    { return engine()->newQObject(new QProcess(parent)); }
    int execute(const QString &program, const QStringList &arguments)
    { return QProcess::execute(program, arguments); }
    int execute(const QString &program)
    { return QProcess::execute(program); }
    bool startDetached(const QString &program, const QStringList &arguments)
    { return QProcess::startDetached(program, arguments); }
    bool startDetached(const QString &program)
    { return QProcess::startDetached(program); }
    QStringList systemEnvironment()
    { return QProcess::systemEnvironment(); }
    QString toString() const
    { return QLatin1String("Process() { [native code] }"); }
};

class ProcessPrototype : public QProcess,
                         public QScriptable
{
    Q_OBJECT
public:
    ProcessPrototype() : QProcess() { }

public Q_SLOTS:
    void closeReadChannel(int channel)
    {
        DECLARE_SELF2(Process, closeReadChannel, );
        self->closeReadChannel(QProcess::ProcessChannel(channel));
    }
    void closeWriteChannel()
    {
        DECLARE_SELF2(Process, closeWriteChannel, );
        self->closeWriteChannel();
    }
    QStringList environment() const
    {
        DECLARE_SELF2(Process, environment, QStringList());
        return self->environment();
    }
    int error() const
    {
        DECLARE_SELF2(Process, error, QProcess::UnknownError);
        return self->error();
    }
    int exitCode() const
    {
        DECLARE_SELF2(Process, exitCode, -1);
        return self->exitCode();
    }
    int exitStatus() const
    {
        DECLARE_SELF2(Process, exitStatus, QProcess::NormalExit);
        return self->exitStatus();
    }
    Q_PID pid() const
    {
        DECLARE_SELF2(Process, pid, 0);
        return self->pid();
    }
    int processChannelMode() const
    {
        DECLARE_SELF2(Process, processChannelMode, QProcess::SeparateChannels);
        return self->processChannelMode();
    }
    QByteArray readAllStandardError()
    {
        DECLARE_SELF2(Process, readAllStandardError, QByteArray());
        return self->readAllStandardError();
    }
    QByteArray readAllStandardOutput()
    {
        DECLARE_SELF2(Process, readAllStandardOutput, QByteArray());
        return self->readAllStandardOutput();
    }
    int readChannel() const
    {
        DECLARE_SELF2(Process, readChannel, QProcess::StandardOutput);
        return self->readChannel();
    }
    void setEnvironment(const QStringList &environment)
    {
        DECLARE_SELF2(Process, setEnvironment, );
        self->setEnvironment(environment);
    }
    void setProcessChannelMode(int mode)
    {
        DECLARE_SELF2(Process, setProcessChannelMode, );
        self->setProcessChannelMode(QProcess::ProcessChannelMode(mode));
    }
    void setReadChannel(int channel)
    {
        DECLARE_SELF2(Process, setReadChannel, );
        self->setReadChannel(QProcess::ProcessChannel(channel));
    }
    void setStandardErrorFile(const QString &fileName, int mode = QIODevice::Truncate)
    {
        DECLARE_SELF2(Process, setStandardErrorFile, );
        self->setStandardErrorFile(fileName, QIODevice::OpenMode(mode));
    }
    void setStandardInputFile(const QString &fileName)
    {
        DECLARE_SELF2(Process, setStandardInputFile, );
        self->setStandardInputFile(fileName);
    }
    void setStandardOutputFile(const QString &fileName, int mode = QIODevice::Truncate)
    {
        DECLARE_SELF2(Process, setStandardOutputFile, );
        self->setStandardOutputFile(fileName, QIODevice::OpenMode(mode));
    }
    void setStandardOutputProcess(QProcess *destination)
    {
        DECLARE_SELF2(Process, setStandardOutputProcess, );
        self->setStandardOutputProcess(destination);
    }
    void setWorkingDirectory(const QString &dir)
    {
        DECLARE_SELF2(Process, setWorkingDirectory, );
        self->setWorkingDirectory(dir);
    }
    void start(const QString &program, const QStringList &arguments, int mode = QIODevice::ReadWrite)
    {
        DECLARE_SELF2(Process, start, );
        self->start(program, arguments, QIODevice::OpenMode(mode));
    }
    void start(const QString &program, int mode = QIODevice::ReadWrite)
    {
        DECLARE_SELF2(Process, start, );
        self->start(program, QIODevice::OpenMode(mode));
    }
    int state() const
    {
        DECLARE_SELF2(Process, state, QProcess::NotRunning);
        return self->state();
    }
    bool waitForFinished(int msecs = 30000)
    {
        DECLARE_SELF2(Process, waitForFinished, false);
        return self->waitForFinished(msecs);
    }
    bool waitForStarted(int msecs = 30000)
    {
        DECLARE_SELF2(Process, waitForStarted, false);
        return self->waitForStarted(msecs);
    }
    QString workingDirectory() const
    {
        DECLARE_SELF2(Process, workingDirectory, QString());
        return self->workingDirectory();
    }
    QString toString() const
    {
        DECLARE_SELF2(Process, toString, QString());
        return QString::fromLatin1("Process");
    }
};

/////////////////////////////////////////////////////////////

QScriptValue constructProcessClass(QScriptEngine *eng)
{
    ProcessPrototype *processProtoObject = new ProcessPrototype();
    QScriptValue proto = eng->newQObject(processProtoObject, QScriptEngine::ScriptOwnership);
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QIODevice*>()));
    eng->setDefaultPrototype(qMetaTypeId<QProcess*>(), proto);

    ProcessConstructor *processCtorObject = new ProcessConstructor();
    QScriptValue ctor = eng->newQObject(processCtorObject, QScriptEngine::ScriptOwnership);
    ctor.setProperty("prototype", proto);
    proto.setProperty("constructor", ctor);
    return ctor;
}

#include "process.moc"
