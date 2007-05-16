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

#include "widgetbox_dnditem.h"
#include "ui4_p.h"
#include <spacer_widget_p.h>
#include <qdesigner_formbuilder_p.h>
#include <formscriptrunner_p.h>

namespace qdesigner_internal {
/*******************************************************************************
** WidgetBoxResource
*/

class WidgetBoxResource : public QDesignerFormBuilder
{
public:
    WidgetBoxResource(QDesignerFormEditorInterface *core);

    virtual QWidget *createWidget(DomWidget *ui_widget, QWidget *parentWidget)
    { return QDesignerFormBuilder::createWidget(ui_widget, parentWidget); }

protected:
    using QDesignerFormBuilder::create;
    using QDesignerFormBuilder::createWidget;

    virtual QWidget *create(DomWidget *ui_widget, QWidget *parents);
    virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name);
};

WidgetBoxResource::WidgetBoxResource(QDesignerFormEditorInterface *core) :
    QDesignerFormBuilder(core, DisableScripts)
{
}


QWidget *WidgetBoxResource::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    if (widgetName == QLatin1String("Spacer")) {
        Spacer *spacer = new Spacer(parentWidget);
        spacer->setObjectName(name);
        return spacer;
    }

    return QDesignerFormBuilder::createWidget(widgetName, parentWidget, name);
}

QWidget *WidgetBoxResource::create(DomWidget *ui_widget, QWidget *parent)
{
    QWidget *result = QDesignerFormBuilder::create(ui_widget, parent);
    result->setFocusPolicy(Qt::NoFocus);
    result->setObjectName(ui_widget->attributeName());

    return result;
}

/*******************************************************************************
** WidgetBoxResource
*/

static QSize geometryProp(const DomWidget *dw)
{
    const QList<DomProperty*> prop_list = dw->elementProperty();
    const QString geometry = QLatin1String("geometry");
    foreach (DomProperty *prop, prop_list) {
        if (prop->attributeName() !=  geometry)
            continue;
        DomRect *dr = prop->elementRect();
        if (dr == 0)
            continue;
        return QSize(dr->elementWidth(), dr->elementHeight());
    }
    return QSize();
}

static QSize domWidgetSize(DomWidget *dw)
{
    QSize size = geometryProp(dw);
    if (size.isValid())
        return size;

    foreach (const DomWidget *child, dw->elementWidget()) {
        size = geometryProp(child);
        if (size.isValid())
            return size;
    }

    foreach (const DomLayout *dl, dw->elementLayout()) {
        foreach (DomLayoutItem *item, dl->elementItem()) {
            const DomWidget *child = item->elementWidget();
            if (child == 0)
                continue;
            size = geometryProp(child);
            if (size.isValid())
                return size;
        }
    }

    return QSize();
}

static QWidget *decorationFromDomWidget(DomWidget *dom_widget, QDesignerFormEditorInterface *core)
{
    QWidget *result = new QWidget(0, Qt::ToolTip);

    WidgetBoxResource builder(core);
    QWidget *w = builder.createWidget(dom_widget, result);
    QSize size = domWidgetSize(dom_widget);
    const QSize minimumSize = w->minimumSizeHint();
    if (!size.isValid())
        size = w->sizeHint();
    if (size.width() < minimumSize.width())
        size.setWidth(minimumSize.width());
    if (size.height() < minimumSize.height())
        size.setHeight(minimumSize.height());
    w->setGeometry(QRect(QPoint(0, 0), size));
    result->resize(size);
    return result;
}

WidgetBoxDnDItem::WidgetBoxDnDItem(QDesignerFormEditorInterface *core,
                                    DomWidget *dom_widget,
                                    const QPoint &global_mouse_pos)
    : QDesignerDnDItem(CopyDrop)
{
    DomWidget *root_dom_widget = new DomWidget;
    QList<DomWidget*> child_list;
    child_list.append(dom_widget);
    root_dom_widget->setElementWidget(child_list);
    DomUI *dom_ui = new DomUI();
    dom_ui->setElementWidget(root_dom_widget);

    QWidget *decoration = decorationFromDomWidget(dom_widget, core);
    decoration->move(global_mouse_pos - QPoint(5, 5));

    init(dom_ui, 0, decoration, global_mouse_pos);
}
}
