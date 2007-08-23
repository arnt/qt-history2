/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qscriptengine.h>
#include <QtGui/QtGui>
#include <QTest>

#include <qlocalsocket.h>
#include <qlocalserver.h>

class QScriptLocalSocket : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString peerName WRITE connectToName READ peerName)

public:
    QScriptLocalSocket(QObject *parent = 0) : QObject(parent)
    {
        lc = new QLocalSocket(this);
    }

public slots:
    QString peerName()
    {
        return lc->peerName();
    }

    void connectToName(const QString &name) {
        lc->connectToName(name);
    }

    void sleep(int x) const
    {
        QTest::qSleep(x);
    }

    bool isConnected() {
        return (lc->state() == QLocalSocket::ConnectedState);
    }

    void open() {
        lc->open(QIODevice::ReadWrite);
    }

    bool waitForConnected() {
        return lc->waitForConnected(100000);
    }
    void waitForReadyRead() {
        lc->waitForReadyRead();
    }

    void write(const QString &string) {
        QTextStream out(lc);
        out << string << endl;
    }

    bool waitForBytesWritten(int t = 3000) {
        return lc->waitForBytesWritten(t);
    }

    QString readLine() {
        QTextStream in(lc);
        return in.readLine();
    }

    QString errorString() {
        return lc->errorString();
    }

    void close() {
        lc->close();
    }

public:
    QLocalSocket *lc;
};

class QScriptLocalServer : public QLocalServer
{
    Q_OBJECT
    Q_PROPERTY(int maxPendingConnections WRITE setMaxPendingConnections READ maxPendingConnections)
    Q_PROPERTY(QString name WRITE listen READ serverName)
    Q_PROPERTY(bool listening READ isListening)

public:
    QScriptLocalServer(QObject *parent = 0) : QLocalServer(parent)
    {
    }

public slots:
    bool listen(const QString &name) {
        return QLocalServer::listen(name);
    }

    QScriptLocalSocket *nextConnection() {
        QLocalSocket *other = nextPendingConnection();
        QScriptLocalSocket *s = new QScriptLocalSocket(this);
        delete s->lc;
        s->lc = other;
        return s;
    }

    bool waitForNewConnection() {
        return QLocalServer::waitForNewConnection(30000);
    }

    QString errorString() {
        return QLocalServer::errorString();
    }


};

template <typename T>
static QScriptValue _q_ScriptValueFromQObject(QScriptEngine *engine, T* const &in)
{
    return engine->newQObject(in);
}
template <typename T>
static void _q_ScriptValueToQObject(const QScriptValue &v, T* &out)
{    out = qobject_cast<T*>(v.toQObject());
}
template <typename T>
static int _q_ScriptRegisterQObjectMetaType(QScriptEngine *engine, const QScriptValue &prototype)
{
    return qScriptRegisterMetaType<T*>(engine, _q_ScriptValueFromQObject<T>, _q_ScriptValueToQObject<T>, prototype);
}

Q_SCRIPT_DECLARE_QMETAOBJECT(QScriptLocalSocket, QObject*);
Q_SCRIPT_DECLARE_QMETAOBJECT(QScriptLocalServer, QObject*);

static void interactive(QScriptEngine &eng)
{
    QTextStream qin(stdin, QFile::ReadOnly);

    const char *qscript_prompt = "qs> ";
    const char *dot_prompt = ".... ";
    const char *prompt = qscript_prompt;

    QString code;

    forever {
        QString line;

        printf("%s", prompt);
        fflush(stdout);

        line = qin.readLine();
        if (line.isNull())
        break;

        code += line;
        code += QLatin1Char('\n');

        if (line.trimmed().isEmpty()) {
            continue;

        } else if (! eng.canEvaluate(code)) {
            prompt = dot_prompt;

        } else {
            QScriptValue result = eng.evaluate(code);
            code.clear();
            prompt = qscript_prompt;
            if (!result.isUndefined())
                fprintf(stderr, "%s\n", qPrintable(result.toString()));
        }
    }
}
Q_DECLARE_METATYPE(QScriptLocalSocket*)
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QScriptEngine eng;
    QScriptValue globalObject = eng.globalObject();

    QScriptValue lss = eng.scriptValueFromQMetaObject<QScriptLocalServer>();
    eng.globalObject().setProperty("QScriptLocalServer", lss);

    _q_ScriptRegisterQObjectMetaType<QScriptLocalSocket>(&eng, QScriptValue());

    QScriptValue lsc = eng.scriptValueFromQMetaObject<QScriptLocalSocket>();
    eng.globalObject().setProperty("QScriptLocalSocket", lsc);

    if (! *++argv) {
        interactive(eng);
        return EXIT_SUCCESS;
    }

    QStringList arguments;
    for (int i = 0; i < argc - 1; ++i)
        arguments << QString::fromLocal8Bit(argv[i]);

    while (!arguments.isEmpty()) {
        QString fn = arguments.takeFirst();

        if (fn == QLatin1String("-i")) {
            interactive(eng);
            break;
        }

        QString contents;

        if (fn == QLatin1String("-")) {
            QTextStream stream(stdin, QFile::ReadOnly);
            contents = stream.readAll();
        } else {
            QFile file(fn);
	    if (!file.exists()) {
                fprintf(stderr, "%s doesn't exists\n", qPrintable(fn));
	        return EXIT_FAILURE;
	    }
            if (file.open(QFile::ReadOnly)) {
                QTextStream stream(&file);
                contents = stream.readAll();
                file.close();
            }
        }

        if (contents.isEmpty())
            continue;

        if (contents[0] == '#') {
            contents.prepend("//");
            QScriptValue args = eng.newArray();
            args.setProperty("0", QScriptValue(&eng, fn));
            int i = 1;
            while (!arguments.isEmpty())
                args.setProperty(i++, QScriptValue(&eng, arguments.takeFirst()));
            eng.currentContext()->activationObject().setProperty("args", args);
        }
        QScriptValue r = eng.evaluate(contents);
        if (eng.hasUncaughtException()) {
            int line = eng.uncaughtExceptionLineNumber();
            fprintf(stderr, "%d: %s\n\t%s\n\n", line, qPrintable(fn), qPrintable(r.toString()));
            return EXIT_FAILURE;
        }
        if (r.isNumber())
            return r.toInt32();
    }

    return EXIT_SUCCESS;
}

#include "main.moc"
