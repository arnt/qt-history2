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

#include <QtGui/QToolBar>

#include "qdesigner.h"
#include "qdesigner_resourceeditor.h"
#include "qdesigner_workbench.h"

#include <resourceeditor/resourceeditor.h>

#include <abstractformeditor.h>

QDesignerResourceEditor::QDesignerResourceEditor(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("ResourceEditor"));
    ResourceEditor *widget = new ResourceEditor(workbench->core(), this);

    setCentralWidget(widget);

    setWindowTitle(tr("Resource Editor"));

    QToolBar *tool_bar = new QToolBar(this);
    addToolBar(tool_bar);
    tool_bar->addAction(widget->actionInsert());
    tool_bar->addAction(widget->actionDelete());
}

QDesignerResourceEditor::~QDesignerResourceEditor()
{
}

QRect QDesignerResourceEditor::geometryHint() const
{
    QRect g = workbench()->availableGeometry();
    int margin = workbench()->marginHint();
    int spacing = 40;

    QSize sz(g.width() * 1/4, g.height() * 4/6);

    return QRect((g.width() - sz.width() - margin), (margin + g.height() * 1/6) + spacing,
                  sz.width(), sz.height());
}
