#ifndef COMP2_H
#define COMP2_H

#include "comp.h"

class Component2 : public Component
{
public:
    Component2();
    ~Component2()
    {
	qDebug( "Destroyed comp2" );
    }

    void sayHello();

    static QUuid cid;
};

#endif // COMP2_H
