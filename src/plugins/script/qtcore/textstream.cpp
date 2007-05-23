#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QtCore/QTextStream>
#include "../global.h"

DECLARE_POINTER_METATYPE(QTextStream)

static QScriptValue newTextStream(QScriptEngine *eng, QTextStream *ts)
{
    return QScript::wrapPointer(eng, ts);
}

class TextStreamConstructor : public QObject,
                              public QScriptable
{
    Q_OBJECT

    Q_PROPERTY(int AlignLeft READ AlignLeft)
    Q_PROPERTY(int AlignRight READ AlignRight)
    Q_PROPERTY(int AlignCenter READ AlignCenter)
    Q_PROPERTY(int AlignAccountingStyle READ AlignAccountingStyle)

    Q_PROPERTY(int ShowBase READ ShowBase)
    Q_PROPERTY(int ForcePoint READ ForcePoint)
    Q_PROPERTY(int ForceSign READ ForceSign)
    Q_PROPERTY(int UppercaseBase READ UppercaseBase)
    Q_PROPERTY(int UppercaseDigits READ UppercaseDigits)

    Q_PROPERTY(int ScientificNotation READ ScientificNotation)
    Q_PROPERTY(int FixedNotation READ FixedNotation)
    Q_PROPERTY(int SmartNotation READ SmartNotation)

    Q_PROPERTY(int Ok READ Ok)
    Q_PROPERTY(int ReadPastEnd READ ReadPastEnd)
    Q_PROPERTY(int ReadCorruptData READ ReadCorruptData)

public:
    TextStreamConstructor() : QObject() { }

    int AlignLeft() const { return QTextStream::AlignLeft; }
    int AlignRight() const { return QTextStream::AlignRight; }
    int AlignCenter() const { return QTextStream::AlignCenter; }
    int AlignAccountingStyle() const { return QTextStream::AlignAccountingStyle; }

    int ShowBase() const { return QTextStream::ShowBase; }
    int ForcePoint() const { return QTextStream::ForcePoint; }
    int ForceSign() const { return QTextStream::ForceSign; }
    int UppercaseBase() const { return QTextStream::UppercaseBase; }
    int UppercaseDigits() const { return QTextStream::UppercaseDigits; }

    int ScientificNotation() const { return QTextStream::ScientificNotation; }
    int FixedNotation() const { return QTextStream::FixedNotation; }
    int SmartNotation() const { return QTextStream::SmartNotation; }

    int Ok() const { return QTextStream::Ok; }
    int ReadPastEnd() const { return QTextStream::ReadPastEnd; }
    int ReadCorruptData() const { return QTextStream::ReadCorruptData; }

public Q_SLOTS:
    QScriptValue qscript_call()
    { return newTextStream(engine(), new QTextStream()); }
    QScriptValue qscript_call(QIODevice *device)
    { return newTextStream(engine(), new QTextStream(device)); }
    QScriptValue qscript_call(QByteArray *array)
    { return newTextStream(engine(), new QTextStream(array)); }
    // ### add rest of ctor overloads
    QString toString() const
    {
        return QLatin1String("QTextStream() { [native code] }");
    }
};

class TextStreamPrototype : public QObject,
                            public QTextStream,
                            public QScriptable
{
    Q_OBJECT
public:
    TextStreamPrototype() : QTextStream() { }

public Q_SLOTS:
    bool atEnd() const
    {
        DECLARE_SELF2(TextStream, atEnd, true);
        return self->atEnd();
    }
    bool autoDetectUnicode() const
    {
        DECLARE_SELF2(TextStream, autoDetectUnicode, false);
        return self->autoDetectUnicode();
    }
    QTextCodec *codec() const
    {
        DECLARE_SELF2(TextStream, codec, 0);
        return self->codec();
    }
    QIODevice *device() const
    {
        DECLARE_SELF2(TextStream, device, 0);
        return self->device();
    }
    int fieldAlignment() const
    {
        DECLARE_SELF2(TextStream, fieldAlignment, 0);
        return self->fieldAlignment();
    }
    int fieldWidth() const
    {
        DECLARE_SELF2(TextStream, fieldWidth, 0);
        return self->fieldWidth();
    }
    void flush()
    {
        DECLARE_SELF2(TextStream, flush, );
        self->flush();
    }
    bool generateByteOrderMark() const
    {
        DECLARE_SELF2(TextStream, generateByteOrderMark, false);
        return self->generateByteOrderMark();
    }
    int integerBase() const
    {
        DECLARE_SELF2(TextStream, integerBase, 0);
        return self->integerBase();
    }
    int numberFlags() const
    {
        DECLARE_SELF2(TextStream, numberFlags, 0);
        return self->numberFlags();
    }
    QChar padChar() const
    {
        DECLARE_SELF2(TextStream, padChar, QChar());
        return self->padChar();
    }
    qint64 pos() const
    {
        DECLARE_SELF2(TextStream, pos, -1);
        return self->pos();
    }
    QString read(qint64 maxlen)
    {
        DECLARE_SELF2(TextStream, read, QString());
        return self->read(maxlen);
    }
    QString readAll()
    {
        DECLARE_SELF2(TextStream, readAll, QString());
        return self->readAll();
    }
    QString readLine(qint64 maxlen = 0)
    {
        DECLARE_SELF2(TextStream, readLine, QString());
        return self->readLine(maxlen);
    }
    int realNumberNotation() const
    {
        DECLARE_SELF2(TextStream, realNumberNotation, 0);
        return self->realNumberNotation();
    }
    void reset()
    {
        DECLARE_SELF2(TextStream, reset, );
        self->reset();
    }
    void resetStatus()
    {
        DECLARE_SELF2(TextStream, resetStatus, );
        self->resetStatus();
    }
    bool seek(qint64 pos)
    {
        DECLARE_SELF2(TextStream, seek, false);
        return self->seek(pos);
    }
    void setAutoDetectUnicode(bool enabled)
    {
        DECLARE_SELF2(TextStream, setAutoDetectUnicode, );
        self->setAutoDetectUnicode(enabled);
    }
    void setCodec(QTextCodec *codec)
    {
        DECLARE_SELF2(TextStream, setCodec, );
        self->setCodec(codec);
    }
    void setDevice(QIODevice *device)
    {
        DECLARE_SELF2(TextStream, setDevice, );
        self->setDevice(device);
    }
    void setFieldAlignment(int mode)
    {
        DECLARE_SELF2(TextStream, setFieldAlignment, );
        self->setFieldAlignment(QTextStream::FieldAlignment(mode));
    }
    void setFieldWidth(int width)
    {
        DECLARE_SELF2(TextStream, setFieldWidth, );
        self->setFieldWidth(width);
    }
    void setGenerateByteOrderMark(bool generate)
    {
        DECLARE_SELF2(TextStream, setGenerateByteOrderMark, );
        self->setGenerateByteOrderMark(generate);
    }
    void setIntegerBase(int base)
    {
        DECLARE_SELF2(TextStream, setIntegerBase, );
        self->setIntegerBase(base);
    }
    void setNumberFlags(int flags)
    {
        DECLARE_SELF2(TextStream, setNumberFlags, );
        self->setNumberFlags(QTextStream::NumberFlags(flags));
    }
    void setPadChar(QChar ch)
    {
        DECLARE_SELF2(TextStream, setPadChar, );
        self->setPadChar(ch);
    }
    void setRealNumberNotation(int notation)
    {
        DECLARE_SELF2(TextStream, setRealNumberNotation, );
        self->setRealNumberNotation(QTextStream::RealNumberNotation(notation));
    }
    void setRealNumberPrecision(int precision)
    {
        DECLARE_SELF2(TextStream, setRealNumberPrecision, );
        self->setRealNumberPrecision(precision);
    }
    void setStatus(int status)
    {
        DECLARE_SELF2(TextStream, setStatus, );
        self->setStatus(QTextStream::Status(status));
    }
    void skipWhiteSpace()
    {
        DECLARE_SELF2(TextStream, skipWhiteSpace, );
        self->skipWhiteSpace();
    }
    int status() const
    {
        DECLARE_SELF2(TextStream, status, 0);
        return self->status();
    }
    QString toString() const
    {
        DECLARE_SELF2(TextStream, toString, QString());
        return QString::fromLatin1("QTextStream");
    }
};

/////////////////////////////////////////////////////////////

QScriptValue constructTextStreamClass(QScriptEngine *eng)
{
    TextStreamPrototype *textStreamProtoObject = new TextStreamPrototype();
    QScriptValue proto = eng->newQObject(textStreamProtoObject, QScriptEngine::ScriptOwnership);
    proto.setPrototype(eng->globalObject().property("Object").property("prototype"));
    QScript::registerPointerMetaType<QTextStream>(eng, proto);

    TextStreamConstructor *textStreamCtorObject = new TextStreamConstructor();
    QScriptValue ctor = eng->newQObject(textStreamCtorObject, QScriptEngine::ScriptOwnership);
    ctor.setPrototype(eng->globalObject().property("Function").property("prototype"));
    ctor.setProperty("prototype", proto);
    proto.setProperty("constructor", ctor);
    return ctor;
}

#include "textstream.moc"
