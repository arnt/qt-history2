#ifndef INTERFACE_FROM-FRAMEWORK_H
#define INTERFACE_FROM_FRAMEWORK_H

#include <Test/testinterface.h>

class TestComponent : public QObject, public TestInterface
{
    Q_OBJECT
    Q_INTERFACES(TestInterface)
public:

    virtual inline foobar() { }
};

#endif // INTERFACE_FROM_FRAMEWORK_H
