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

#ifndef ABSTRACTIMAGECOLLECTION_H
#define ABSTRACTIMAGECOLLECTION_H

#include "sdk_global.h"
#include <QObject>

class AbstractFormEditor;

class QT_SDK_EXPORT AbstractImageCollection: public QObject
{
    Q_OBJECT
public:
    AbstractImageCollection(QObject *parent = 0);
    virtual ~AbstractImageCollection();

    virtual AbstractFormEditor *core() const = 0;

    virtual QString fileName() const = 0;
    virtual QString prefix() const = 0;

    virtual int count() const = 0;
    virtual QString item(int index) const = 0;
};

#endif // ABSTRACTIMAGECOLLECTION_H
