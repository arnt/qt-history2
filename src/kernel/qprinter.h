/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter.h#1 $
**
** Definition of printer classes
**
** Author  : Eirik Eng
** Created : 940927
**
** Copyright (c) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QPRINTER_H
#define QPRINTER_H

#include "qpaintd.h"


//
// WARNING: No printer support in this version of Qt!
//

class QPrinter : public QPaintDevice
{
public:
    QPrinter();
};


#endif // QPRINTER_H
