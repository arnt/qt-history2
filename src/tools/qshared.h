/****************************************************************************
** $Id: //depot/qt/main/src/tools/qshared.h#5 $
**
** Definition of QShared struct
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSHARED_H
#define QSHARED_H

#include "qglobal.h"

class QShared					// really a struct
{
public:
    QShared()		{ count = 1; }
    void ref()		{ count++; }
    bool deref()	{ return !--count; }
    uint count;
};

#endif // QSHARED_H
