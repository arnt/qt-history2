
#include "layoutinfo.h"

#include <abstractformeditor.h>
#include <container.h>
#include <qextensionmanager.h>

#include <QMap>
#include <QLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>

LayoutInfo::Type LayoutInfo::layoutType(AbstractFormEditor *core, QWidget *w, QLayout *&layout)
{
    layout = 0;

    if (IContainer *container = qt_extension<IContainer*>(core->extensionManager(), w))
        w = container->widget(container->currentIndex());

    if (qt_cast<QSplitter*>(w))
        return static_cast<QSplitter*>(w)->orientation() == Qt::Horizontal ? HBox : VBox;

    if (!w || !w->layout())
        return NoLayout;
    QLayout *lay = w->layout();

    if (qt_cast<QGroupBox*>(w)) {
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
LayoutInfo::Type LayoutInfo::layoutType(AbstractFormEditor *core, QLayout *layout)
{
    Q_UNUSED(core)

    if (qt_cast<QHBoxLayout*>(layout))
        return HBox;
    else if (qt_cast<QVBoxLayout*>(layout))
        return VBox;
    else if (qt_cast<QGridLayout*>(layout))
        return Grid;
    return NoLayout;
}

/*!
  \overload
*/
LayoutInfo::Type LayoutInfo::layoutType(AbstractFormEditor *core, QWidget *w)
{
    QLayout *l = 0;
    return layoutType(core, w, l);
}


QWidget *LayoutInfo::layoutParent(AbstractFormEditor *core, QLayout *layout)
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

void LayoutInfo::deleteLayout(AbstractFormEditor *core, QWidget *widget)
{
    if (!widget)
        return;

    if (IContainer *container = qt_extension<IContainer*>(core->extensionManager(), widget))
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

