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

#ifndef ABSTRACTUNDOMANAGER_H
#define ABSTRACTUNDOMANAGER_H

#include "sdk_global.h"

#include <QObject>

class AbstractFormEditor;

class QT_SDK_EXPORT AbstractUndoManager: public QObject
{
    Q_OBJECT
public:
    AbstractUndoManager(QObject *parent);
    virtual ~AbstractUndoManager();

    virtual AbstractFormEditor *core() const = 0;
};

#endif // ABSTRACTUNDOMANAGER_H
