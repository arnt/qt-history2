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

#include <QtGui/QLabel>
#include <QtGui/QPixmap>

#include <ui4.h>
#include <qdesigner_resource.h>

#include "formwindow_dnditem.h"
#include "formwindow.h"

static QWidget *decorationFromWidget(QWidget *w)
{
    QLabel *label = new QLabel(0, Qt::ToolTip);
    label->setPixmap(QPixmap::grabWidget(w));

    return label;
}

static DomUI *widgetToDom(QWidget *widget, FormWindow *form)
{
    QDesignerResource builder(form);
    return builder.copy(QList<QWidget*>() << widget);
}

FormWindowDnDItem::FormWindowDnDItem(AbstractDnDItem::DropType type, FormWindow *form,
                                        QWidget *widget, const QPoint &global_mouse_pos)
    : QDesignerDnDItem(type, form)
{
    QWidget *decoration = decorationFromWidget(widget);
    QPoint pos = widget->mapToGlobal(QPoint(0, 0));
    decoration->move(pos);
    DomUI *dom_ui = widgetToDom(widget, form);

    init(dom_ui, widget, decoration, global_mouse_pos);
}

