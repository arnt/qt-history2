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

#include "widgetboxview.h"

// components
#include <widgetbox.h>
#include <abstractformeditor.h>

WidgetBoxView::WidgetBoxView(AbstractFormEditor *core, QWidget *parent)
    : QMainWindow(parent, Qt::WStyle_Tool),
      m_core(core)
{
    setWindowTitle(tr("Widget Box"));

    WidgetBox *editor = new WidgetBox(core, WidgetBox::TreeMode, this);
    setCentralWidget(editor);

    core->setWidgetBox(editor);

    (void) statusBar();
}

WidgetBoxView::~WidgetBoxView()
{
}

void WidgetBoxView::showEvent(QShowEvent *ev)
{
    emit visibilityChanged(isVisible());
    QMainWindow::showEvent(ev);
}

void WidgetBoxView::hideEvent(QHideEvent *ev)
{
    emit visibilityChanged(isVisible());
    QMainWindow::hideEvent(ev);
}
