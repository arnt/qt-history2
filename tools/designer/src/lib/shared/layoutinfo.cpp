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

#include "layoutinfo_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerMetaDataBaseInterface>
#include <QtDesigner/QExtensionManager>

#include <QtGui/QHBoxLayout>
#include <QtGui/QSplitter>
#include <QtCore/QDebug>

namespace qdesigner_internal {
/*!
  \overload
*/
LayoutInfo::Type LayoutInfo::layoutType(const QDesignerFormEditorInterface *core, const QLayout *layout)
{
    Q_UNUSED(core)
    if (qobject_cast<const QHBoxLayout*>(layout))
        return HBox;
    else if (qobject_cast<const QVBoxLayout*>(layout))
        return VBox;
    else if (qobject_cast<const QGridLayout*>(layout))
        return Grid;
    return NoLayout;
}

/*!
  \overload
*/
LayoutInfo::Type LayoutInfo::layoutType(const QDesignerFormEditorInterface *core, const QWidget *w)
{
    if (const QSplitter *splitter = qobject_cast<const QSplitter *>(w))
        return  splitter->orientation() == Qt::Horizontal ? HSplitter : VSplitter;
    return layoutType(core, w->layout());
}

QWidget *LayoutInfo::layoutParent(const QDesignerFormEditorInterface *core, QLayout *layout)
{
    Q_UNUSED(core)

    QObject *o = layout;
    while (o) {
        if (QWidget *widget = qobject_cast<QWidget*>(o))
            return widget;

        o = o->parent();
    }
    return 0;
}

void LayoutInfo::deleteLayout(const QDesignerFormEditorInterface *core, QWidget *widget)
{
    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), widget))
        widget = container->widget(container->currentIndex());

    Q_ASSERT(widget != 0);

    QLayout *layout = managedLayout(core, widget);

    if (layout == 0 || core->metaDataBase()->item(layout) != 0) {
        delete layout;
        return;
    }

    qDebug() << "trying to delete an unmanaged layout:" << "widget:" << widget << "layout:" << layout;
}

bool LayoutInfo::isWidgetLaidout(const QDesignerFormEditorInterface *core, QWidget *widget)
{
    Q_UNUSED(core);

    const QWidget *parent = widget->parentWidget();

    if (qobject_cast<const QSplitter*>(parent)) { // ### generalize
        return true;
    }

    if (parent && parent->layout()) {
        if (parent->layout()->indexOf(widget) != -1)
            return true;

        const QList<QLayout*> childLayouts = qFindChildren<QLayout*>(parent->layout());
        foreach (QLayout *childLayout, childLayouts) {
            if (childLayout->indexOf(widget) != -1)
                return true;
        }
    }

    return false;
}

QLayout *LayoutInfo::internalLayout(const QWidget *widget)
{
    QLayout *widgetLayout = widget->layout();
    if (widgetLayout && widget->inherits("Q3GroupBox")) {
        if (widgetLayout->count()) {
            widgetLayout = widgetLayout->itemAt(0)->layout();
        } else {
            widgetLayout = 0;
        }
    }
    return widgetLayout;
}


QLayout *LayoutInfo::managedLayout(const QDesignerFormEditorInterface *core, const QWidget *widget)
{
    if (widget == 0)
        return 0;

    QLayout *layout = widget->layout();
    if (!layout)
        return 0;

    return managedLayout(core, layout);
}

QLayout *LayoutInfo::managedLayout(const QDesignerFormEditorInterface *core, QLayout *layout)
{
    QDesignerMetaDataBaseInterface *metaDataBase = core->metaDataBase();

    if (!metaDataBase)
        return layout;

    const QDesignerMetaDataBaseItemInterface *item = metaDataBase->item(layout);
    if (item == 0) {
        layout = qFindChild<QLayout*>(layout);
        item = metaDataBase->item(layout);
    }
    if (!item)
        return 0;
    return layout;
}
} // namespace qdesigner_internal
