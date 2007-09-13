/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDESIGNER_PROPERTYEDITOR_H
#define QDESIGNER_PROPERTYEDITOR_H

#include "qdesigner_toolwindow.h"

QT_BEGIN_NAMESPACE

class QDesignerWorkbench;

class QDesignerPropertyEditor: public QDesignerToolWindow
{
    Q_OBJECT
public:
    explicit QDesignerPropertyEditor(QDesignerWorkbench *workbench);
    virtual ~QDesignerPropertyEditor();

    virtual QRect geometryHint() const;

protected:
    virtual void showEvent(QShowEvent *event);
};

QT_END_NAMESPACE

#endif // QDESIGNER_PROPERTYEDITOR_H
