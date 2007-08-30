#ifndef SCRIPTBREAKPOINTMANAGER_H
#define SCRIPTBREAKPOINTMANAGER_H

#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtScript/qscriptvalue.h>

class ScriptBreakpointManagerPrivate;
class ScriptBreakpointManager
{
public:
    ScriptBreakpointManager();
    ~ScriptBreakpointManager();

    bool hasBreakpoints() const;

    int setBreakpoint(const QString &fileName, int lineNumber);
    int setBreakpoint(const QString &functionName, const QString &fileName = QString());
    int setBreakpoint(const QScriptValue &function);

    void removeBreakpoint(int id);

    int findBreakpoint(const QString &fileName, int lineNumber) const;
    int findBreakpoint(const QString &functionName, const QString &fileName = QString()) const;
    int findBreakpoint(const QScriptValue &function) const;

    bool isBreakpointEnabled(int id) const;
    void setBreakpointEnabled(int id, bool enabled);

    QString breakpointCondition(int id) const;
    void setBreakpointCondition(int id, const QString &expression);

    int breakpointIgnoreCount(int id) const;
    void setBreakpointIgnoreCount(int id, int ignoreCount);

    bool isBreakpointSingleShot(int id) const;
    void setBreakpointSingleShot(int id, bool singleShot);

protected:
    ScriptBreakpointManager(ScriptBreakpointManagerPrivate &dd);
    ScriptBreakpointManagerPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(ScriptBreakpointManager)
    Q_DISABLE_COPY(ScriptBreakpointManager)
};

#endif // SCRIPTBREAKPOINTMANAGER_H
