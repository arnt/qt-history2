#include "scriptbreakpointmanager.h"
#include "scriptbreakpointmanager_p.h"

ScriptBreakpointManagerPrivate::ScriptBreakpointManagerPrivate()
{
}

ScriptBreakpointManagerPrivate::~ScriptBreakpointManagerPrivate()
{
}

ScriptBreakpointManager::ScriptBreakpointManager()
    : d_ptr(new ScriptBreakpointManagerPrivate)
{
    d_ptr->q_ptr = this;
}

ScriptBreakpointManager::~ScriptBreakpointManager()
{
}

bool ScriptBreakpointManager::hasBreakpoints() const
{
    Q_D(const ScriptBreakpointManager);
    return !d->breakpoints.isEmpty();
}

int ScriptBreakpointManager::setBreakpoint(const QString &fileName, int lineNumber)
{
    Q_D(ScriptBreakpointManager);
    d->breakpoints.append(ScriptBreakpointInfo(fileName, lineNumber));
    return d->breakpoints.size() - 1;
}

int ScriptBreakpointManager::setBreakpoint(const QString &functionName, const QString &fileName)
{
    Q_D(ScriptBreakpointManager);
    d->breakpoints.append(ScriptBreakpointInfo(functionName, fileName));
    return d->breakpoints.size() - 1;
}

int ScriptBreakpointManager::setBreakpoint(const QScriptValue &function)
{
    Q_D(ScriptBreakpointManager);
    d->breakpoints.append(ScriptBreakpointInfo(function));
    return d->breakpoints.size() - 1;
}

void ScriptBreakpointManager::removeBreakpoint(int id)
{
    Q_D(ScriptBreakpointManager);
    if (id >= 0 && id < d->breakpoints.size())
        d->breakpoints[id] = ScriptBreakpointInfo();
}

int ScriptBreakpointManager::findBreakpoint(const QString &fileName, int lineNumber) const
{
    Q_D(const ScriptBreakpointManager);
    for (int i = 0; i < d->breakpoints.size(); ++i) {
        const ScriptBreakpointInfo &brk = d->breakpoints.at(i);
        if (brk.type != ScriptBreakpointInfo::File)
            continue;
        if (brk.fileName == fileName && brk.lineNumber == lineNumber)
            return i;
    }
    return -1;
}

int ScriptBreakpointManager::findBreakpoint(const QString &functionName, const QString &fileName) const
{
    Q_D(const ScriptBreakpointManager);
    for (int i = 0; i < d->breakpoints.size(); ++i) {
        const ScriptBreakpointInfo &brk = d->breakpoints.at(i);
        if (brk.type != ScriptBreakpointInfo::FunctionName)
            continue;
        if (brk.functionName == functionName && brk.fileName == fileName)
            return i;
    }
    return -1;
}

int ScriptBreakpointManager::findBreakpoint(const QScriptValue &function) const
{
    Q_D(const ScriptBreakpointManager);
    for (int i = 0; i < d->breakpoints.size(); ++i) {
        const ScriptBreakpointInfo &brk = d->breakpoints.at(i);
        if (brk.type != ScriptBreakpointInfo::Function)
            continue;
        if (brk.function.strictlyEquals(function))
            return i;
    }
    return -1;
}

bool ScriptBreakpointManager::isBreakpointEnabled(int id) const
{
    Q_D(const ScriptBreakpointManager);
    return d->breakpoints.value(id).enabled;
}

void ScriptBreakpointManager::setBreakpointEnabled(int id, bool enabled)
{
    Q_D(ScriptBreakpointManager);
    if (id >= 0 && id < d->breakpoints.size())
        d->breakpoints[id].enabled = enabled;
}

QString ScriptBreakpointManager::breakpointCondition(int id) const
{
    Q_D(const ScriptBreakpointManager);
    return d->breakpoints.value(id).condition;
}

void ScriptBreakpointManager::setBreakpointCondition(int id, const QString &expression)
{
    Q_D(ScriptBreakpointManager);
    if (id >= 0 && id < d->breakpoints.size())
        d->breakpoints[id].condition = expression;
}

int ScriptBreakpointManager::breakpointIgnoreCount(int id) const
{
    Q_D(const ScriptBreakpointManager);
    return d->breakpoints.value(id).ignoreCount;
}

void ScriptBreakpointManager::setBreakpointIgnoreCount(int id, int ignoreCount)
{
    Q_D(ScriptBreakpointManager);
    if (id >= 0 && id < d->breakpoints.size())
        d->breakpoints[id].ignoreCount = ignoreCount;
}

bool ScriptBreakpointManager::isBreakpointSingleShot(int id) const
{
    Q_D(const ScriptBreakpointManager);
    return d->breakpoints.value(id).singleShot;
}

void ScriptBreakpointManager::setBreakpointSingleShot(int id, bool singleShot)
{
    Q_D(ScriptBreakpointManager);
    if (id >= 0 && id < d->breakpoints.size())
        d->breakpoints[id].singleShot = singleShot;
}
