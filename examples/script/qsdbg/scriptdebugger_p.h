#ifndef SCRIPTDEBUGGER_P_H
#define SCRIPTDEBUGGER_P_H

#include <QtCore/qobjectdefs.h>

#include <QtScript/qscriptengineagent.h>
#include <QtScript/qscriptvalue.h>

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qstack.h>

class QScriptContext;
class QTextStream;
class ScriptInfo;
class ScriptBreakpointManager;

class QScriptDebugger;
class ScriptDebuggerPrivate
    : public QScriptEngineAgent
{
    Q_DECLARE_PUBLIC(ScriptDebugger)
public:
    enum Mode {
        Run,
        StepInto,
        StepOver
    };

    ScriptDebuggerPrivate(QScriptEngine *engine);
    ~ScriptDebuggerPrivate();

    // QScriptEngineAgent interface
    void scriptLoad(qint64 id, const QString &program,
                    const QString &fileName, int lineNumber);
    void scriptUnload(qint64 id);

    void positionChange(qint64 scriptId,
                        int lineNumber, int columnNumber);

    void functionEntry(qint64 scriptId);
    void functionExit(qint64 scriptId,
                      const QScriptValue &returnValue);

    void exceptionThrow(qint64 scriptId,
                        const QScriptValue &exception, bool hasHandler);


    void interactive();
    bool executeCommand(const QString &command, const QStringList &args);

    void setMode(Mode mode);
    Mode mode() const;

    int frameCount() const;
    void setCurrentFrameIndex(int index);
    int currentFrameIndex() const;

    QScriptContext *frameContext(int index) const;
    QScriptContext *currentFrameContext() const;

    QString contextString(QScriptContext *context) const;
    ScriptInfo *scriptInfo(QScriptContext *context) const;

    int listLineNumber() const;
    void setListLineNumber(int lineNumber);

    QString readLine();
    void output(const QString &text);
    void message(const QString &text);
    void errorMessage(const QString &text);

    // attributes
    QTextStream *m_defaultInputStream;
    QTextStream *m_defaultOutputStream;
    QTextStream *m_defaultErrorStream;
    QTextStream *m_inputStream;
    QTextStream *m_outputStream;
    QTextStream *m_errorStream;

    ScriptBreakpointManager *m_bpManager;
    Mode m_mode;
    QMap<qint64, ScriptInfo*> m_scripts;
    QMap<QScriptContext*, QStack<qint64> > m_contextProgramIds;

    QString m_lastInteractiveCommand;
    QString m_commandPrefix;
    int m_stepDepth;
    int m_currentFrameIndex;
    int m_listLineNumber;

    ScriptDebugger *q_ptr;
};

#endif
