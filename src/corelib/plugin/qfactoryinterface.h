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

#ifndef QFACTORYINTERFACE_H
#define QFACTORYINTERFACE_H

#include "QtCore/qobject.h"
#include "QtCore/qstringlist.h"

struct QFactoryInterface
{
    virtual ~QFactoryInterface() {}
    virtual QStringList keys() const = 0;
};

Q_DECLARE_INTERFACE(QFactoryInterface, "com.trolltech.Qt.QFactoryInterface")

#endif // QFACTORYINTERFACE_H
