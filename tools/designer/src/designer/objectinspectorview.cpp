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

#include "objectinspectorview.h"

// components
#include <objectinspector.h>

// sdk
#include <abstractformeditor.h>

ObjectInspectorView::ObjectInspectorView(AbstractFormEditor *core, QWidget *parent)
    : QMainWindow(parent, Qt::WStyle_Tool),
      m_core(core)
{
    setWindowTitle(tr("Object Inspector"));

    ObjectInspector *editor = new ObjectInspector(core, this);
    setCentralWidget(editor);

    core->setObjectInspector(editor);

    (void) statusBar();
}

ObjectInspectorView::~ObjectInspectorView()
{
}

void ObjectInspectorView::hideEvent(QHideEvent *ev)
{
    emit visibilityChanged(false);
    QMainWindow::hideEvent(ev);
}

void ObjectInspectorView::showEvent(QShowEvent *ev)
{
    emit visibilityChanged(true);
    QMainWindow::showEvent(ev);
}
