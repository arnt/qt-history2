#ifndef BASECLASS_H
#define BASECLASS_H

#include <QtGlobal>

QT_USE_NAMESPACE

class BaseClass
{
public:
    virtual ~BaseClass();

#ifdef BASECLASS_NOT_ABSTRACT
    virtual void wasAPureVirtualFunction();
#else
    virtual void wasAPureVirtualFunction() = 0;
#endif
};

#endif
