#ifndef SCRIPTBREAKPOINTMANAGER_P_H
#define SCRIPTBREAKPOINTMANAGER_P_H

#include <QtCore/qobjectdefs.h>

class ScriptBreakpointInfo
{
public:
    enum Type {
        File,
        FunctionName,
        Function,
        Invalid
    };

    Type type;
    QString functionName;
    QString fileName;
    int lineNumber;
    QScriptValue function;
    bool enabled;
    QString condition;
    int ignoreCount;
    bool singleShot;

    ScriptBreakpointInfo(const QString &fileName, int lineNumber)
        : type(File), fileName(fileName), lineNumber(lineNumber),
          enabled(true), ignoreCount(0), singleShot(false)
        { }
    ScriptBreakpointInfo(const QString &functionName, const QString &fileName = QString())
        : type(FunctionName), functionName(functionName), fileName(fileName),
          enabled(true), ignoreCount(0), singleShot(false)
        { }
    ScriptBreakpointInfo(const QScriptValue &function)
        : type(Function), function(function),
          enabled(true), ignoreCount(0), singleShot(false)
        { }
    ScriptBreakpointInfo()
        : type(Invalid)
        { }
};

class ScriptBreakpointManager;
class ScriptBreakpointManagerPrivate
{
    Q_DECLARE_PUBLIC(ScriptBreakpointManager)
public:
    ScriptBreakpointManagerPrivate();
    ~ScriptBreakpointManagerPrivate();

    QList<ScriptBreakpointInfo> breakpoints;

    ScriptBreakpointManager *q_ptr;
};

#endif
