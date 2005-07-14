/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3SHARED_H
#define Q3SHARED_H

#include "QtCore/qglobal.h"

QT_MODULE(Qt3SupportLight)

struct Q_COMPAT_EXPORT Q3Shared
{
    Q3Shared() : count( 1 ) { }
    void ref()		{ count++; }
    bool deref()	{ return !--count; }
    uint count;
};


#endif // Q3SHARED_H
