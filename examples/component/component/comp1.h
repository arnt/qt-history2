#ifndef COMP1_H
#define COMP1_H

#include "comp.h"

class Component1 : public Component
{
public:
    Component1();
    ~Component1()
    {
	qDebug( "Destroyed comp1" );
    }

    void sayHello();

    static QUuid cid;
};

#endif // COMP1_H
