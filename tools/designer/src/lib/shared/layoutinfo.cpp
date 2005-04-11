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

#include "layoutinfo.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/container.h>
#include <QtDesigner/qextensionmanager.h>

#include <QtCore/QMap>
#include <QLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>

LayoutInfo::Type LayoutInfo::layoutType(QDesignerFormEditorInterface *core, QWidget *w, QLayout *&layout)
{
    layout = 0;

    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), w))
        w = container->widget(container->currentIndex());

    if (qobject_cast<QSplitter*>(w))
        return static_cast<QSplitter*>(w)->orientation() == Qt::Horizontal ? HBox : VBox;

    if (!w || !w->layout())
        return NoLayout;
    QLayout *lay = w->layout();

    if (qobject_cast<QGroupBox*>(w)) {
        QList<QLayout*> l = qFindChildren<QLayout*>(lay);
        if (l.size())
            lay = l.first();
    }
    layout = lay;

    return layoutType(core, lay);
}

/*!
  \overload
*/
LayoutInfo::Type LayoutInfo::layoutType(QDesignerFormEditorInterface *core, QLayout *layout)
{
    Q_UNUSED(core)

    if (qobject_cast<QHBoxLayout*>(layout))
        return HBox;
    else if (qobject_cast<QVBoxLayout*>(layout))
        return VBox;
    else if (qobject_cast<QGridLayout*>(layout))
        return Grid;
    return NoLayout;
}

/*!
  \overload
*/
LayoutInfo::Type LayoutInfo::layoutType(QDesignerFormEditorInterface *core, QWidget *w)
{
    QLayout *l = 0;
    return layoutType(core, w, l);
}


QWidget *LayoutInfo::layoutParent(QDesignerFormEditorInterface *core, QLayout *layout)
{
    Q_UNUSED(core)

    QObject *o = layout;
    while (o) {
        if (o->isWidgetType())
            return static_cast<QWidget*>(o);
        o = o->parent();
    }
    return 0;
}

void LayoutInfo::deleteLayout(QDesignerFormEditorInterface *core, QWidget *widget)
{
    if (!widget)
        return;

    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), widget))
        widget = container->widget(container->currentIndex());

    delete widget->layout();
}

void LayoutInfo::cells(QLayout *layout, IntervalList *rows, IntervalList *columns)
{
    QMap<Interval, int> rowDict;
    QMap<Interval, int> columnDict;
    
    int i = 0;
    while (QLayoutItem *item = layout->itemAt(i)) {
        ++i;
        
        QRect g = item->geometry();
        columnDict.insert(Interval(g.left(), g.right()), 1);
        rowDict.insert(Interval(g.top(), g.bottom()), 1);
    }
        
    if (columns)
        *columns = columnDict.keys();
    
    if (rows)
        *rows = rowDict.keys();
}
