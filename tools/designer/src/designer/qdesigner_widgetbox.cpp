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

#include "qdesigner.h"
#include "qdesigner_widgetbox.h"
#include "qdesigner_workbench.h"

#include <widgetbox/widgetbox.h>

#include <abstractformeditor.h>
#include <abstractformwindowmanager.h>

#include <QtCore/qdebug.h>
#include <QtGui/QAction>
#include <QtGui/QShortcut>

QDesignerWidgetBox::QDesignerWidgetBox(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("WidgetBox"));
    WidgetBox *widget = new WidgetBox(workbench->core(), this);
    workbench->core()->setWidgetBox(widget);

    setCentralWidget(widget);

    setWindowTitle(tr("Widget Box"));
    QShortcut *shortcut = new QShortcut(this);
    shortcut->setContext(Qt::ApplicationShortcut);
    shortcut->setKey(QKeySequence(Qt::Key_Backspace));
    connect(shortcut, SIGNAL(activated()),
            workbench->core()->formWindowManager()->actionDelete(), SIGNAL(triggered()));
}

QDesignerWidgetBox::~QDesignerWidgetBox()
{
}

QRect QDesignerWidgetBox::geometryHint() const
{
    QRect g = workbench()->availableGeometry();

    return QRect(workbench()->marginHint(), workbench()->marginHint(),
                 g.width() * 1/4, g.height() * 5/6);
}
