#define BASECLASS_NOT_ABSTRACT
#include "baseclass.h"

#include <QtGlobal>

BaseClass::~BaseClass()
{
}

void BaseClass::wasAPureVirtualFunction()
{
    qDebug("BaseClass::wasAPureVirtualFunction()");
}
