/****************************************************************************
**
** Definition of QShared struct.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSHARED_H
#define QSHARED_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


struct Q_CORE_EXPORT QShared
{
    QShared() : count( 1 ) { }
    void ref()		{ count++; }
    bool deref()	{ return !--count; }
    uint count;
};


#endif // QSHARED_H
