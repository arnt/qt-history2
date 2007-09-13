#define BASECLASS_NOT_ABSTRACT
#include "baseclass.h"

BaseClass::~BaseClass()
{
}

void BaseClass::wasAPureVirtualFunction()
{
    qDebug("BaseClass::wasAPureVirtualFunction()");
}
