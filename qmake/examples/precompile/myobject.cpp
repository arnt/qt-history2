#include <iostream>
#include <qobject.h>
#include "stable.h"    // Last stable header
#include "myobject.h"  // Own header

MyObject::MyObject()
    : QObject()
{
    std::cout << "MyObject::MyObject()\n";
}

MyObject::~MyObject()
{
    qDebug("MyObject::~MyObject()");
}
