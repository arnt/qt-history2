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

#include "qdesigner_customwidget.h"
#include "formwindow.h"

#include <abstractformeditor.h>
#include "widgetdatabase.h"

QDesignerCustomWidget::QDesignerCustomWidget(FormWindow *formWindow, QWidget *parent)
    : QDesignerWidget(formWindow, parent),
      m_widgetClassName(QLatin1String("QWidget"))
{
}

QDesignerCustomWidget::~QDesignerCustomWidget()
{
}

AbstractWidgetDataBaseItem *QDesignerCustomWidget::widgetItem() const
{
    AbstractFormEditor *core = formWindow()->core();
    int index = core->widgetDataBase()->indexOfClassName(widgetClassName());
    if (index != -1)
        return core->widgetDataBase()->item(index);

    qWarning("no widget item for %s", widgetClassName().toLatin1().constData());
    return 0;
}

QString QDesignerCustomWidget::widgetClassName() const
{
    return m_widgetClassName;
}

void QDesignerCustomWidget::setWidgetClassName(const QString &widgetClassName)
{
    m_widgetClassName = widgetClassName;
    createWidgetItem();
}

void QDesignerCustomWidget::createWidgetItem()
{
    if (!widgetItem()) {
        WidgetDataBaseItem *item = new WidgetDataBaseItem();
        item->setName(m_widgetClassName);
        formWindow()->core()->widgetDataBase()->append(item);
    }
}

bool QDesignerCustomWidget::isCompat() const
{
    if (AbstractWidgetDataBaseItem *item = widgetItem())
        return item->isCompat();

    return false;
}

void QDesignerCustomWidget::setCompat(bool compat)
{
    if (AbstractWidgetDataBaseItem *item = widgetItem()) {
        item->setCompat(compat);
        update();
    }
}

bool QDesignerCustomWidget::isContainer() const
{
    if (AbstractWidgetDataBaseItem *item = widgetItem())
        return item->isContainer();

    return false;
}

void QDesignerCustomWidget::setContainer(bool container)
{
    if (AbstractWidgetDataBaseItem *item = widgetItem()) {
        item->setContainer(container);
        update();
    }
}

