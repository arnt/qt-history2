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


#include "qdesigner_workbench.h"
#include "qdesigner_resourceeditor.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerComponents>

QDesignerResourceEditor::QDesignerResourceEditor(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("ResourceEditor"));
    QWidget *widget = QDesignerComponents::createResourceEditor(workbench->core(), this);

    setCentralWidget(widget);

    setWindowTitle(tr("Resource Editor"));
}

QDesignerResourceEditor::~QDesignerResourceEditor()
{
}

QRect QDesignerResourceEditor::geometryHint() const
{
    const QRect g = workbench()->availableGeometry();
    const int margin = workbench()->marginHint();

    const QSize sz(g.width() * 1/3, g.height() * 1/6);
    QRect r(QPoint(0, 0), sz);
    r.moveCenter(g.center());
    r.moveBottom(g.bottom() - margin);

    return r;
}

