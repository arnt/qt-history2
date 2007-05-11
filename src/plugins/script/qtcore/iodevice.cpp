#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QtCore/QIODevice>
#include "../global.h"

Q_DECLARE_METATYPE(QIODevice*)

class IODeviceConstructor : public QObject,
                            public QScriptable
{
    Q_OBJECT

    Q_PROPERTY(int NotOpen READ NotOpen)
    Q_PROPERTY(int ReadOnly READ ReadOnly)
    Q_PROPERTY(int WriteOnly READ WriteOnly)
    Q_PROPERTY(int ReadWrite READ ReadWrite)
    Q_PROPERTY(int Append READ Append)
    Q_PROPERTY(int Truncate READ Truncate)
    Q_PROPERTY(int Text READ Text)
    Q_PROPERTY(int Unbuffered READ Unbuffered)

public:
    IODeviceConstructor() : QObject() { }

    int NotOpen() const { return QIODevice::NotOpen; }
    int ReadOnly() const { return QIODevice::ReadOnly; }
    int WriteOnly() const { return QIODevice::WriteOnly; }
    int ReadWrite() const { return QIODevice::ReadWrite; }
    int Append() const { return QIODevice::Append; }
    int Truncate() const { return QIODevice::Truncate; }
    int Text() const { return QIODevice::Text; }
    int Unbuffered() const { return QIODevice::Unbuffered; }

public Q_SLOTS:
    QString toString() const
    {
        return QLatin1String("IODevice() { [native code] }");
    }
};

class IODevicePrototype : public QIODevice,
                          public QScriptable
{
    Q_OBJECT
public:
    IODevicePrototype() : QIODevice() { }

public Q_SLOTS:
    bool atEnd() const
    {
        DECLARE_SELF2(IODevice, atEnd, true);
        return self->atEnd();
    }
    qint64 bytesAvailable() const
    {
        DECLARE_SELF2(IODevice, bytesAvailable, 0);
        return self->bytesAvailable();
    }
    qint64 bytesToWrite() const
    {
        DECLARE_SELF2(IODevice, bytesToWrite, 0);
        return self->bytesToWrite();
    }
    bool canReadLine() const
    {
        DECLARE_SELF2(IODevice, canReadLine, false);
        return self->canReadLine();
    }
    void close()
    {
        DECLARE_SELF2(IODevice, close, );
        self->close();
    }
    QString errorString() const
    {
        DECLARE_SELF2(IODevice, errorString, QString());
        return self->errorString();
    }
    // ### getChar(): can't bind it since it takes * argument
    // Use exceptions instead?
    bool isOpen() const
    {
        DECLARE_SELF2(IODevice, isOpen, false);
        return self->isOpen();
    }
    bool isReadable() const
    {
        DECLARE_SELF2(IODevice, isReadable, false);
        return self->isReadable();
    }
    bool isSequential() const
    {
        DECLARE_SELF2(IODevice, isSequential, false);
        return self->isSequential();
    }
    bool isTextModeEnabled() const
    {
        DECLARE_SELF2(IODevice, isTextModeEnabled, false);
        return self->isTextModeEnabled();
    }
    bool isWritable() const
    {
        DECLARE_SELF2(IODevice, isWritable, false);
        return self->isWritable();
    }
    bool open(QIODevice::OpenMode mode)
    {
        DECLARE_SELF2(IODevice, open, false);
        return self->open(mode);
    }
    QIODevice::OpenMode openMode() const
    {
        DECLARE_SELF2(IODevice, openMode, QIODevice::NotOpen);
        return self->openMode();
    }
    QByteArray peek(qint64 maxSize)
    {
        DECLARE_SELF2(IODevice, peek, QByteArray());
        return self->peek(maxSize);
    }
    qint64 pos() const
    {
        DECLARE_SELF2(IODevice, pos, 0);
        return self->pos();
    }
    bool putChar(char c)
    {
        DECLARE_SELF2(IODevice, putChar, false);
        return self->putChar(c);
    }
    QByteArray read(qint64 maxSize)
    {
        DECLARE_SELF2(IODevice, read, QByteArray());
        return self->read(maxSize);
    }
    QByteArray readAll()
    {
        DECLARE_SELF2(IODevice, readAll, QByteArray());
        return self->readAll();
    }
    QByteArray readLine(qint64 maxSize = 0)
    {
        DECLARE_SELF2(IODevice, readLine, QByteArray());
        return self->readLine(maxSize);
    }
    bool reset()
    {
        DECLARE_SELF2(IODevice, reset, false);
        return self->reset();
    }
    bool seek(qint64 pos)
    {
        DECLARE_SELF2(IODevice, seek, false);
        return self->seek(pos);
    }
    void setTextModeEnabled(bool enabled)
    {
        DECLARE_SELF2(IODevice, setTextModeEnabled, );
        self->setTextModeEnabled(enabled);
    }
    qint64 size() const
    {
        DECLARE_SELF2(IODevice, size, 0);
        return self->size();
    }
    void ungetChar(char c)
    {
        DECLARE_SELF2(IODevice, ungetChar, );
        self->ungetChar(c);
    }
    bool waitForBytesWritten(int msecs)
    {
        DECLARE_SELF2(IODevice, waitForBytesWritten, false);
        return self->waitForBytesWritten(msecs);
    }
    bool waitForReadyRead(int msecs)
    {
        DECLARE_SELF2(IODevice, waitForReadyRead, false);
        return self->waitForReadyRead(msecs);
    }
    qint64 write(const QByteArray &byteArray)
    {
        DECLARE_SELF2(IODevice, write, -1);
        return self->write(byteArray);
    }
    QString toString() const
    {
        DECLARE_SELF2(IODevice, toString, QString());
        return QString::fromLatin1("IODevice");
    }

protected:
    virtual qint64 readData(char *, qint64)
    { return 0; }
    virtual qint64 writeData(const char *, qint64)
    { return 0; }
};

/////////////////////////////////////////////////////////////

QScriptValue constructIODeviceClass(QScriptEngine *eng)
{
    IODevicePrototype *ioProtoObject = new IODevicePrototype();
    QScriptValue proto = eng->newQObject(ioProtoObject, QScriptEngine::ScriptOwnership);

    eng->setDefaultPrototype(qMetaTypeId<QIODevice*>(), proto);

    IODeviceConstructor *ioCtorObject = new IODeviceConstructor();
    QScriptValue ctor = eng->newQObject(ioCtorObject, QScriptEngine::ScriptOwnership);
    ctor.setProperty("prototype", proto);
    proto.setProperty("constructor", ctor);
    return ctor;
}

#include "iodevice.moc"
