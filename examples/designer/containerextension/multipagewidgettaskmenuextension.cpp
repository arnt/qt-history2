/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtDesigner/QtDesigner>

#include <QAction>

#include "multipagewidgettaskmenuextension.h"
#include "multipagewidget.h"

MultiPageWidgetTaskMenuExtension::MultiPageWidgetTaskMenuExtension(MultiPageWidget *widget, QObject *parent)
    : QObject(parent)
{
    myWidget = widget;

    addPageAction = new QAction(tr("Add Page"), this);
    connect(addPageAction, SIGNAL(triggered()), this, SLOT(addPage()));

    removePageAction = new QAction(tr("Delete Page"), this);
    connect(removePageAction, SIGNAL(triggered()), this, SLOT(removePage()));
}

void MultiPageWidgetTaskMenuExtension::addPage()
{
    QDesignerFormWindowInterface *formWindow;
    formWindow = QDesignerFormWindowInterface::findFormWindow(myWidget);
    if (formWindow) {
        QDesignerFormEditorInterface *core = formWindow->core();

        QWidget *page = core->widgetFactory()->createWidget("QWidget", myWidget);
        core->metaDataBase()->add(page);
        myWidget->addPage(page);
    }
}

void MultiPageWidgetTaskMenuExtension::removePage()
{
    myWidget->removePage(myWidget->currentIndex());
}

QList<QAction *> MultiPageWidgetTaskMenuExtension::taskActions() const
{
    QList<QAction *> list;
    list.append(addPageAction);
    list.append(removePageAction);
    return list;
}
