#ifndef PRINTINTERFACE_H
#define PRINTINTERFACE_H

#include <qcom.h>

// {1DA52229-5C33-40C3-925C-53777D39E458} 
#ifndef IID_Print
#define IID_Print QUuid( 0x1da52229, 0x5c33, 0x40c3, 0x92, 0x5c, 0x53, 0x77, 0x7d, 0x39, 0xe4, 0x58)
#endif

struct PrintInterface : public QUnknownInterface
{
    virtual void sayHello() = 0;
};

#endif // PRINTINTERFACE_H
