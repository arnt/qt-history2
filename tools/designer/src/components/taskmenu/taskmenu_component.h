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

#ifndef TASKMENU_COMPONENT_H
#define TASKMENU_COMPONENT_H

#include "taskmenu_global.h"
#include <taskmenu.h>

#include <QObject>

class AbstractFormEditor;

class QT_TASKMENU_EXPORT TaskMenuComponent: public QObject
{
    Q_OBJECT
public:
    TaskMenuComponent(AbstractFormEditor *core, QObject *parent = 0);
    virtual ~TaskMenuComponent();

    AbstractFormEditor *core() const;

private:
    AbstractFormEditor *m_core;
};

#endif // TASKMENU_COMPONENT_H
