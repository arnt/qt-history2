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
#include "qdesigner_widget.h"
#include "qdesigner_tabwidget.h"
#include "qdesigner_toolbox.h"
#include "qdesigner_stackedbox.h"
#include "qdesigner_customwidget.h"
#include "widgetdatabase.h"
#include "formwindow.h"
#include "layout.h"

// sdk
#include <propertysheet.h>
#include <container.h>
#include <qextensionmanager.h>
#include <abstractmetadatabase.h>
#include <abstractformeditor.h>
#include <layoutinfo.h>
#include <spacer.h>
#include <customwidget.h>

#include <QtGui/QtGui>
#include <qdebug.h>

WidgetFactory::WidgetFactory(AbstractFormEditor *core, QObject *parent)
    : AbstractWidgetFactory(parent),
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

        if (ICustomWidget *c = qt_cast<ICustomWidget*>(o)) {
            if (!c->isInitialized())
                c->initialize(core());

            m_customFactory.insert(c->name(), c);
        }
    }
}

QWidget *WidgetFactory::createWidget(const QString &widgetName, QWidget *parentWidget) const
{
    FormWindow *fw = FormWindow::findFormWindow(parentWidget);

    QWidget *w = 0;

    if (ICustomWidget *f = m_customFactory.value(widgetName)) {
        return f->createWidget(parentWidget);
    } else if (widgetName == QLatin1String("Line")) {
        w = new QFrame(parentWidget);
    } else if (widgetName == QLatin1String("QLabel")) {
        w = new QDesignerLabel(parentWidget);
    } else if (widgetName == QLatin1String("QTabWidget")) {
        w = new QDesignerTabWidget(parentWidget);
    } else if (widgetName == QLatin1String("QStackedWidget")) {
        w = new QDesignerStackedWidget(parentWidget);
    } else if (widgetName == QLatin1String("QToolBox")) {
        w = new QDesignerToolBox(parentWidget);
    } else if (widgetName == QLatin1String("QLabel")) {
        w = new QDesignerLabel(parentWidget);
    } else if (widgetName == QLatin1String("QLayoutWidget")) {
        w = fw ? new QLayoutWidget(fw, parentWidget) : new QWidget(parentWidget);
    } else if (widgetName == QLatin1String("QToolBar")) { // ### remove me
        w = new QToolBar(qt_cast<QMainWindow*>(parentWidget));
    } else if (widgetName == QLatin1String("Spacer")) {
        w = new Spacer(parentWidget);
    } else if (widgetName == QLatin1String("QDialog")) {
        if (fw) {
             w = new QDesignerDialog(fw, parentWidget);
        } else {
            w = new QDialog(parentWidget);
        }
    } else if (widgetName == QLatin1String("QWidget")) {
        if (fw && parentWidget &&
             (qt_cast<FormWindow*>(parentWidget) || qt_extension<IContainer*>(m_core->extensionManager(), parentWidget))) {
             w = new QDesignerWidget(fw, qt_cast<FormWindow*>(parentWidget) ? parentWidget : 0);
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
        QDesignerCustomWidget *customWidget = new QDesignerCustomWidget(fw, parentWidget);
        customWidget->setWidgetClassName(widgetName);
        qWarning("widget %s not found", widgetName.toLatin1().constData());
        w = customWidget;
    }

    if (w) {
        w->setParent(w->parentWidget(), 0);
        initialize(w);
    }

    return w;
}

const char *WidgetFactory::classNameOf(QObject* o)
{
    if (qt_cast<QDesignerTabWidget*>(o))
        return "QTabWidget";
    else if (qt_cast<QDesignerStackedWidget*>(o))
        return "QStackedWidget";
    else if (qt_cast<QDesignerToolBox*>(o))
        return "QToolBox";
    else if (qt_cast<QDesignerDialog*>(o))
        return "QDialog";
    else if (qt_cast<QDesignerWidget*>(o))
        return "QWidget";
    else if (qt_cast<QDesignerLabel*>(o))
        return "QLabel";
    return o->metaObject()->className();
}

/*!  Creates a layout on the widget \a widget of the type \a type
  which can be \c HBox, \c VBox or \c Grid.
*/

QLayout *WidgetFactory::createLayout(QWidget *widget, QLayout *layout, int type) const
{
    AbstractMetaDataBase *metaDataBase = core()->metaDataBase();

    if (!layout)
        widget = containerOfWidget(widget);

    metaDataBase->add(widget);

    QLayout *l = 0;
    Qt::Alignment align = Qt::AlignAuto;
    if (layout) {
        switch (type) {
        case LayoutInfo::HBox:
            l = new QHBoxLayout(layout);
            break;
        case LayoutInfo::VBox:
            l = new QVBoxLayout(layout);
            break;
        case LayoutInfo::Grid:
            l = new QGridLayout(layout);
            break;
        case LayoutInfo::Stacked:
            l = new QStackedLayout(layout);
            break;
        case LayoutInfo::NoLayout:
            return 0;
        default:
            Q_ASSERT( 0 );
        }
        metaDataBase->add(l);
    } else {
        switch (type) {
        case LayoutInfo::HBox:
            l = new QHBoxLayout(widget);
            break;
        case LayoutInfo::VBox:
            l = new QVBoxLayout(widget);
            break;
        case LayoutInfo::Grid:
            l = new QGridLayout(widget);
            break;
        case LayoutInfo::Stacked:
            l = new QStackedLayout(widget);
            break;
        case LayoutInfo::NoLayout:
            return 0;
        default:
            Q_ASSERT( 0 );
        }
        core()->metaDataBase()->add(l);
    }
    l->setAlignment(align);

    if (QLayoutWidget *l = qt_cast<QLayoutWidget*>(widget)) {
        l->layout()->setMargin(1); // ### this should be 0
    }

    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(core()->extensionManager(), l)) {
        sheet->setChanged(sheet->indexOf("margin"), true);
        sheet->setChanged(sheet->indexOf("spacing"), true);
        sheet->setChanged(sheet->indexOf("alignment"), true);
    }

    return l;
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
    if (!w)
        return w;

    // ### use the IContainer extension
    else if (qt_cast<QTabWidget*>(w))
        return static_cast<QTabWidget*>(w)->currentWidget();
    else if (qt_cast<QStackedWidget*>(w))
        return static_cast<QStackedWidget*>(w)->currentWidget();
    else if (qt_cast<QToolBox*>(w))
        return static_cast<QToolBox*>(w)->widget(static_cast<QToolBox*>(w)->currentIndex());
    else if (qt_cast<QMainWindow*>(w))
        return static_cast<QMainWindow*>(w)->centralWidget();
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
         qt_cast<QToolBox*>(w->parentWidget()->parentWidget()->parentWidget()))
        return w->parentWidget()->parentWidget()->parentWidget();
    while (w) {
        if (core()->widgetDataBase()->isContainer(w) ||
             w && qt_cast<FormWindow*>(w->parentWidget()))
            return w;
        w = w->parentWidget();
    }
    return w;
}

AbstractFormEditor *WidgetFactory::core() const
{
    return m_core;
}

void WidgetFactory::initialize(QObject *object) const
{
    IPropertySheet *sheet = qt_extension<IPropertySheet*>(m_core->extensionManager(), object);

    if (object->metaObject()->indexOfProperty("focusPolicy") != -1)
        object->setProperty("focusPolicy", Qt::NoFocus);

    if (!sheet)
        return;

    sheet->setChanged(sheet->indexOf("objectName"), true);
    sheet->setChanged(sheet->indexOf("geometry"), true);

    if (qt_cast<Spacer*>(object))
        sheet->setChanged(sheet->indexOf("sizeHint"), true);
}
