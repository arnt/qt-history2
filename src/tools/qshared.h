/****************************************************************************
** $Id: //depot/qt/main/src/tools/qshared.h#11 $
**
** Definition of QShared struct
**
** Created : 940112
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSHARED_H
#define QSHARED_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


struct QShared
{
    QShared()		{ count = 1; }
    void ref()		{ count++; }
    bool deref()	{ return !--count; }
    uint count;
};


#endif // QSHARED_H
