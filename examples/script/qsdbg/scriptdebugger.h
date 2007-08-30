#ifndef SCRIPTDEBUGGER_H
#define SCRIPTDEBUGGER_H

#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

class QScriptEngine;
class QScriptValue;
class QTextStream;

class ScriptDebuggerPrivate;
class ScriptDebugger
{
public:
    ScriptDebugger(QScriptEngine *engine);
    virtual ~ScriptDebugger();

    void breakAtNextStatement();

    void setBreakpoint(const QString &fileName, int lineNumber);
    void setBreakpoint(const QString &functionName, const QString &fileName = QString());
    void setBreakpoint(const QScriptValue &function);

    QTextStream *inputStream() const;
    void setInputStream(QTextStream *inputStream);

    QTextStream *outputStream() const;
    void setOutputStream(QTextStream *outputStream);

    QTextStream *errorStream() const;
    void setErrorStream(QTextStream *errorStream);

protected:
    ScriptDebugger(QScriptEngine *engine, ScriptDebuggerPrivate &dd);
    ScriptDebuggerPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(ScriptDebugger)
    Q_DISABLE_COPY(ScriptDebugger)
};

#endif // SCRIPTDEBUGGER_H
