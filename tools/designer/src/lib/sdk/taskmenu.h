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

#ifndef TASKMENU_H
#define TASKMENU_H

#include <QtDesigner/extension.h>

class QAction;

class QDesignerTaskMenuExtension
{
public:
    virtual ~QDesignerTaskMenuExtension() {}

    virtual QAction *preferredEditAction() const;

    virtual QList<QAction*> taskActions() const = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerTaskMenuExtension, "http://trolltech.com/Qt/IDE/TaskMenu")


inline QAction *QDesignerTaskMenuExtension::preferredEditAction() const
{ return 0; }


#endif // TASKMENU_H
