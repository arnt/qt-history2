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

#include "propertyeditorview.h"
// components
#include <propertyeditor.h>

PropertyEditorView::PropertyEditorView(AbstractFormEditor *core, QWidget *parent)
    : QMainWindow(parent, Qt::WStyle_Tool),
      m_core(core)
{
    setWindowTitle(tr("Property Editor"));

    PropertyEditor *editor = new PropertyEditor(core, this);
    setCentralWidget(editor);

    core->setPropertyEditor(editor);

    (void) statusBar();
}

PropertyEditorView::~PropertyEditorView()
{
}

void PropertyEditorView::showEvent(QShowEvent *ev)
{
    emit visibilityChanged(isVisible());
    QMainWindow::showEvent(ev);
}

void PropertyEditorView::hideEvent(QHideEvent *ev)
{
    emit visibilityChanged(isVisible());
    QMainWindow::hideEvent(ev);
}
