#ifndef TESTINTERFACE_H
#define TESTINTERFACE_H

#include <QtCore/qobject.h>

struct TestInterface
{
    inline virtual ~TestInterface() {}

    virtual void foobar() = 0;
};

Q_DECLARE_INTERFACE(TestInterface, "foo.bar/1.0")

#endif // TESTINTERFACE_H
