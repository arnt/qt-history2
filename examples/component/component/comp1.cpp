#include "comp1.h"

// {8AA0BD9B-8136-47F3-8C4F-BC8C08E93D34} 
QUuid Component1::cid = QUuid( 0x8aa0bd9b, 0x8136, 0x47f3, 0x8c, 0x4f, 0xbc, 0x8c, 0x08, 0xe9, 0x3d, 0x34 );

Component1::Component1()
{
}

void Component1::sayHello()
{
    qDebug( "Hi there!" );
}
