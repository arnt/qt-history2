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

#include <QtGui/QStackedWidget>
#include <QtGui/QLabel>

#include <abstractformwindowmanager.h>

#include "qdesigner.h"
#include "qdesigner_resourceeditor.h"
#include "qdesigner_workbench.h"

#include <resourceeditor/resourceeditor.h>

#include <abstractformeditor.h>

QDesignerResourceEditor::QDesignerResourceEditor(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("ResourceEditor"));
    
    m_stack = new QStackedWidget(this);
    QLabel *label = new QLabel(tr("No opened forms"));
    label->setAlignment(Qt::AlignCenter);
    m_stack->addWidget(label);
    m_stack->addWidget(new ResourceEditor(workbench->core()));
    
    setCentralWidget(m_stack);

    connect(workbench->core()->formWindowManager(), SIGNAL(formWindowAdded(AbstractFormWindow*)),
            this, SLOT(updateStack()));
    connect(workbench->core()->formWindowManager(), SIGNAL(formWindowRemoved(AbstractFormWindow*)),
            this, SLOT(updateStack()));
    
    setWindowTitle(tr("Resource Editor"));
}

QDesignerResourceEditor::~QDesignerResourceEditor()
{
}

QRect QDesignerResourceEditor::geometryHint() const
{
    QRect g = workbench()->availableGeometry();
    int margin = workbench()->marginHint();

    QRect r(QPoint(0, 0), QSize(g.width() * 1/4, g.height() * 1/6));

    r.moveCenter(g.center());
    r.moveTop(g.top() + margin);

    return r;
}

void QDesignerResourceEditor::updateStack()
{
    if (workbench()->core()->formWindowManager()->formWindowCount() == 0)
        m_stack->setCurrentIndex(0);
    else
        m_stack->setCurrentIndex(1);
}

