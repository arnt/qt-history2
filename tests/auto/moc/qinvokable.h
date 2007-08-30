
#include <QObject>

class InvokableBeforeReturnType : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE const char *foo() const { return ""; }
};

class InvokableBeforeInline : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE inline void foo() {}
    Q_INVOKABLE virtual void bar() {}
};
