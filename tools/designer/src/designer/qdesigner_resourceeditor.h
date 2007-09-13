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

#ifndef QDESIGNER_RESOURCEEDITOR_H
#define QDESIGNER_RESOURCEEDITOR_H

#include "qdesigner_toolwindow.h"

QT_BEGIN_NAMESPACE

class QDesignerWorkbench;
class QDesignerAbstractFormWindowInterface;

class QDesignerResourceEditor: public QDesignerToolWindow
{
    Q_OBJECT
public:
    explicit QDesignerResourceEditor(QDesignerWorkbench *workbench);
    virtual ~QDesignerResourceEditor();

    virtual QRect geometryHint() const;
};

QT_END_NAMESPACE

#endif // QDESIGNER_RESOURCEEDITOR_H
