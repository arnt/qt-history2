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

#include "widgetfactory.h"
#include "widgetdatabase.h"
#include "qlayout_widget.h"
#include "qdesigner_widget.h"
#include "qdesigner_tabwidget.h"
#include "qdesigner_toolbox.h"
#include "qdesigner_stackedbox.h"
#include "qdesigner_promotedwidget.h"
#include "abstractformwindow.h"

// shared
#include "layoutinfo.h"
#include "spacer_widget.h"
#include "layout.h"

// sdk
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/container.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstractwidgetdatabase.h>
#include <QtDesigner/abstractmetadatabase.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/customwidget.h>

#include <QtGui/QtGui>
#include <QtCore/qdebug.h>

WidgetFactory::WidgetFactory(QDesignerFormEditorInterface *core, QObject *parent)
    : QDesignerWidgetFactoryInterface(parent),
      m_core(core)
{
}

WidgetFactory::~WidgetFactory()
{
}

void WidgetFactory::loadPlugins()
{
    PluginManager *pluginManager = m_core->pluginManager();

    m_customFactory.clear();
    QStringList plugins = pluginManager->registeredPlugins();

    foreach (QString plugin, plugins) {
        QObject *o = pluginManager->instance(plugin);

        if (QDesignerCustomWidgetInterface *c = qobject_cast<QDesignerCustomWidgetInterface*>(o)) {
            if (!c->isInitialized())
                c->initialize(core());

            m_customFactory.insert(c->name(), c);
        }
    }
}

QWidget *WidgetFactory::createWidget(const QString &widgetName, QWidget *parentWidget) const
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parentWidget))
        parentWidget = promoted->child();

    QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(parentWidget);

    QWidget *w = 0;

    if (QDesignerCustomWidgetInterface *f = m_customFactory.value(widgetName)) {
        return f->createWidget(parentWidget);
    } else if (widgetName == QLatin1String("Line")) {
        w = new Line(parentWidget);
    } else if (widgetName == QLatin1String("QLabel")) {
        w = new QDesignerLabel(parentWidget);
    } else if (widgetName == QLatin1String("QTabWidget")) {
        w = new QDesignerTabWidget(parentWidget);
    } else if (widgetName == QLatin1String("QStackedWidget")) {
        w = new QDesignerStackedWidget(parentWidget);
    } else if (widgetName == QLatin1String("QToolBox")) {
        w = new QDesignerToolBox(parentWidget);
    } else if (widgetName == QLatin1String("Spacer")) {
        w = new Spacer(parentWidget);
    } else if (widgetName == QLatin1String("QLayoutWidget")) {
        w = fw ? new QLayoutWidget(fw, parentWidget) : new QWidget(parentWidget);
    } else if (widgetName == QLatin1String("QDialog")) {
        if (fw) {
             w = new QDesignerDialog(fw, parentWidget);
        } else {
            w = new QDialog(parentWidget);
        }
    } else if (widgetName == QLatin1String("QWidget")) {
        if (fw && parentWidget &&
             (qobject_cast<QDesignerFormWindowInterface*>(parentWidget) || qt_extension<QDesignerContainerExtension*>(m_core->extensionManager(), parentWidget))) {
             w = new QDesignerWidget(fw, qobject_cast<QDesignerFormWindowInterface*>(parentWidget) ? parentWidget : 0);
        } else {
            w = new QWidget(parentWidget);
        }
    }

#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C) /*DECLARE_WIDGET(W, C)*/
#define DECLARE_WIDGET(W, C) else if (widgetName == QLatin1String(#W)) { Q_ASSERT(w == 0); w = new W(parentWidget); }

#include "widgets.table"

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET

    if (fw && !w) {
        QDesignerWidgetDataBaseInterface *db = fw->core()->widgetDataBase();
        QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfClassName(widgetName));
        if (item != 0 && item->isPromoted()) {
            QWidget *child = createWidget(item->extends(), 0);
            if (child != 0) {
                w = new QDesignerPromotedWidget(item, child, parentWidget);
                child->setParent(w, 0);
            }
        } else {
            // qDebug() << "widget" << widgetName << "not found! Created a generic custom widget";

            // step 1) create a new entry in widget database
            QDesignerWidgetDataBaseItemInterface *item = new WidgetDataBaseItem(widgetName, tr("%1 Widget").arg(widgetName));
            item->setCustom(true);
            item->setPromoted(true); // ### ??
            item->setExtends(QLatin1String("QWidget"));
            item->setIncludeFile(widgetName.toLower() + QLatin1String(".h"));
            db->append(item);

            // step 2) create the actual widget
            QWidget *actualWidget = new QWidget();

            // step 3) create a QDesignerPromotedWidget
            QDesignerPromotedWidget *promotedWidget = new QDesignerPromotedWidget(item, actualWidget, parentWidget);
            w = promotedWidget;
        }
    }

    if (w != 0) {
        initialize(w);
    }

    return w;
}

const char *WidgetFactory::classNameOf(QObject* o)
{
    if (qobject_cast<QDesignerTabWidget*>(o))
        return "QTabWidget";
    else if (qobject_cast<QDesignerStackedWidget*>(o))
        return "QStackedWidget";
    else if (qobject_cast<QDesignerToolBox*>(o))
        return "QToolBox";
    else if (qobject_cast<QDesignerDialog*>(o))
        return "QDialog";
    else if (qobject_cast<QDesignerWidget*>(o))
        return "QWidget";
    else if (qobject_cast<QDesignerLabel*>(o))
        return "QLabel";
    else if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(o)) {
        return promoted->customClassName();
    }
    return o->metaObject()->className();
}

/*!  Creates a layout on the widget \a widget of the type \a type
  which can be \c HBox, \c VBox or \c Grid.
*/

QLayout *WidgetFactory::createLayout(QWidget *widget, QLayout *parentLayout, int type) const // ### (sizepolicy)
{
    QDesignerMetaDataBaseInterface *metaDataBase = core()->metaDataBase();

    if (parentLayout == 0) {
        widget = containerOfWidget(widget);
    }

    Q_ASSERT(metaDataBase->item(widget) != 0); // ensure the widget is managed

    if (parentLayout == 0 && metaDataBase->item(widget->layout()) == 0) {
        parentLayout = widget->layout();
    }

    QWidget *parentWidget = parentLayout != 0 ? 0 : widget;

    QLayout *layout = 0;
    switch (type) {
    case LayoutInfo::HBox:
        layout = new QHBoxLayout(parentWidget);
        break;
    case LayoutInfo::VBox:
        layout = new QVBoxLayout(parentWidget);
        break;
    case LayoutInfo::Grid:
        layout = new QGridLayout(parentWidget);
        break;
    case LayoutInfo::Stacked:
        layout = new QStackedLayout(parentWidget);
        break;
    default:
        Q_ASSERT(0);
        return 0;
    } // end switch

    metaDataBase->add(layout); // add the layout in the MetaDataBase

    if (QLayoutWidget *layoutWidget = qobject_cast<QLayoutWidget*>(widget)) {
        layoutWidget->setLayoutMargin(0);
    }

    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), layout)) {
        sheet->setChanged(sheet->indexOf(QLatin1String("margin")), true);
        sheet->setChanged(sheet->indexOf(QLatin1String("spacing")), true);
        sheet->setChanged(sheet->indexOf(QLatin1String("alignment")), true);
    }

    if (widget && metaDataBase->item(widget->layout()) == 0) {
        Q_ASSERT(layout->parent() == 0);
        QBoxLayout *box = qobject_cast<QBoxLayout*>(widget->layout());
        Q_ASSERT(box != 0); // we support only unmanaged box layouts
        box->addLayout(layout);
    }

    return layout;
}

/*!  Returns the widget into which children should be inserted when \a
  w is a container known to the designer.

  Usually that is \a w itself, sometimes it is different (e.g. a
  tabwidget is known to the designer as a container but the child
  widgets should be inserted into the current page of the
  tabwidget. So in this case this function returns the current page of
  the tabwidget.)
 */
QWidget* WidgetFactory::containerOfWidget(QWidget *w) const
{
    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), w))
        return container->widget(container->currentIndex());

    return w;
}

/*!  Returns the actual designer widget of the container \a w. This is
  normally \a w itself, but might be a parent or grand parent of \a w
  (e.g. when working with a tabwidget and \a w is the container which
  contains and layouts childs, but the actual widget known to the
  designer is the tabwidget which is the parent of \a w. So this
  function returns the tabwidget then.)
*/

QWidget* WidgetFactory::widgetOfContainer(QWidget *w) const
{
    // ### cleanup
    if (!w)
        return 0;

    if (w->parentWidget() && w->parentWidget()->parentWidget() &&
         w->parentWidget()->parentWidget()->parentWidget() &&
         qobject_cast<QToolBox*>(w->parentWidget()->parentWidget()->parentWidget()))
        return w->parentWidget()->parentWidget()->parentWidget();
    while (w) {
        if (core()->widgetDataBase()->isContainer(w) ||
             w && qobject_cast<QDesignerFormWindowInterface*>(w->parentWidget()))
            return w;
        w = w->parentWidget();
    }
    return w;
}

QDesignerFormEditorInterface *WidgetFactory::core() const
{
    return m_core;
}

void WidgetFactory::initialize(QObject *object) const
{
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_core->extensionManager(), object);

    if (object->metaObject()->indexOfProperty("focusPolicy") != -1)
        object->setProperty("focusPolicy", Qt::NoFocus);

    if (!sheet)
        return;

    sheet->setChanged(sheet->indexOf(QLatin1String("objectName")), true);
    sheet->setChanged(sheet->indexOf(QLatin1String("geometry")), true);

    if (qobject_cast<Spacer*>(object))
        sheet->setChanged(sheet->indexOf(QLatin1String("sizeHint")), true);

    int o = sheet->indexOf(QLatin1String("orientation"));
    if (o != -1)
        sheet->setChanged(o, true);
}
