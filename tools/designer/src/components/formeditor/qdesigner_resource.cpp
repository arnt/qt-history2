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

#include "qdesigner_resource.h"
#include "formwindow.h"
#include "widgetdatabase.h"
#include "layout.h"
#include "qlayout_widget.h"
#include "qdesigner_widget.h"
#include "qdesigner_tabwidget.h"
#include "qdesigner_toolbox.h"
#include "qdesigner_stackedbox.h"
#include "qdesigner_customwidget.h"

// shared
#include <layoutinfo.h>
#include <spacer_widget.h>
#include <qdesigner_promotedwidget.h>

// sdk
#include <container.h>
#include <propertysheet.h>
#include <extrainfo.h>
#include <qextensionmanager.h>
#include <abstractwidgetfactory.h>
#include <abstractmetadatabase.h>
#include <abstractformeditor.h>
#include <abstracticoncache.h>
#include <abstractformwindowtool.h>
#include <ui4.h>

#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBox>
#include <QtGui/QTabBar>
#include <QtXml/QDomDocument>
#include <QtCore/QDir>

#include <QtCore/qdebug.h>

QDesignerResource::QDesignerResource(FormWindow *formWindow)
   : m_formWindow(formWindow), m_core(formWindow->core())
{
    m_topLevelSpacerCount = 0;
    m_copyWidget = false;

    m_internal_to_qt.insert("QDesignerWidget", "QWidget");
    m_internal_to_qt.insert("QDesignerStackedWidget", "QStackedWidget");
    m_internal_to_qt.insert("QLayoutWidget", "QWidget");
    m_internal_to_qt.insert("QDesignerTabWidget", "QTabWidget");
    m_internal_to_qt.insert("QDesignerDialog", "QDialog");
    m_internal_to_qt.insert("QDesignerLabel", "QLabel");
    m_internal_to_qt.insert("QDesignerToolBox", "QToolBox");

    // invert
    QHashIterator<QString, QString> it(m_internal_to_qt);
    while (it.hasNext()) {
        it.next();

        if (it.value() == QLatin1String("QDesignerWidget")
                || it.value() == QLatin1String("QLayoutWidget"))
            continue;

        m_qt_to_internal.insert(it.value(), it.key());
    }
}

QDesignerResource::~QDesignerResource()
{
}

void QDesignerResource::save(QIODevice *dev, QWidget *widget)
{
    m_topLevelSpacerCount = 0;

    Resource::save(dev, widget);

    if (m_topLevelSpacerCount != 0) {
        QMessageBox::warning(widget->window(), QObject::tr("Qt Designer"),
               QObject::tr("This file contains top level spacers.<br>"
                           "They have <b>NOT</b> been saved into the form.<br>"
                           "Perhaps you forgot to create a layout?"),
                           QMessageBox::Ok, 0);
    }
}

void QDesignerResource::saveDom(DomUI *ui, QWidget *widget)
{
    Resource::saveDom(ui, widget);

    if (m_formWindow) {
        for (int index = 0; index < m_formWindow->toolCount(); ++index) {
            AbstractFormWindowTool *tool = m_formWindow->tool(index);
            Q_ASSERT(tool != 0);
            tool->saveToDom(ui, widget);
        }
    }
}

QWidget *QDesignerResource::create(DomUI *ui, QWidget *parentWidget)
{
    QString version = ui->attributeVersion();
    if (version != "4.0") {
        QMessageBox::warning(parentWidget->window(), QObject::tr("Qt Designer"),
               QObject::tr("This file was created using designer from Qt-%1 and "
                           "could not be read. "
                           "Please run it through <b>uic3 -convert</b> to convert "
                           "it to Qt4's ui format.").arg(version),
                               QMessageBox::Ok, 0);
        return 0;
    }

    if (IExtraInfo *extra = qt_extension<IExtraInfo*>(core()->extensionManager(), m_formWindow)) {
        extra->saveExtraInfo(ui, m_formWindow);
    }

    m_isMainWidget = true;
    QWidget *mainWidget = Resource::create(ui, parentWidget);
    if (mainWidget == 0)
        return 0;

    if (m_formWindow) {
        for (int index = 0; index < m_formWindow->toolCount(); ++index) {
            AbstractFormWindowTool *tool = m_formWindow->tool(index);
            Q_ASSERT(tool != 0);
            tool->loadFromDom(ui, mainWidget);
        }
    }

    return mainWidget;
}

static ActionListElt createActionListElt(DomAction *action)
{
    ActionListElt result;

    result.name = action->attributeName();
    result.menu = action->attributeMenu();
    QList<DomProperty*> prop_list = action->elementProperty();
    foreach (DomProperty *prop, prop_list) {
        if (prop->attributeName() == QLatin1String("objectName")) {
            result.objectName
                = prop->elementString() == 0 ? QString() : prop->elementString()->text();
        } else if (prop->attributeName() == QLatin1String("icon")) {
            result.icon
                = prop->elementIconSet() == 0 ? QString() : prop->elementIconSet()->text();
        } else if (prop->attributeName() == QLatin1String("iconText")) {
            result.iconText
                = prop->elementString() == 0 ? QString() : prop->elementString()->text();
        } else if (prop->attributeName() == QLatin1String("shortcut")) {
            result.shortcut
                = prop->elementString() == 0 ? QString() : prop->elementString()->text();
        }
    }

    return result;
}

static DomAction *createDomAction(const ActionListElt &elt)
{
    DomAction *result = new DomAction;

    result->setAttributeName(elt.name);
    result->setAttributeMenu(elt.menu);

    QList<DomProperty*> prop_list;

    DomProperty *prop = new DomProperty;
    prop->setAttributeName("objectName");
    DomString *string = new DomString;
    string->setText(elt.objectName);
    prop->setElementString(string);
    prop_list.append(prop);

    prop = new DomProperty;
    prop->setAttributeName("icon");
    DomResourcePixmap *pixmap = new DomResourcePixmap;
    pixmap->setText(elt.icon);
    prop->setElementIconSet(pixmap);
    prop_list.append(prop);

    prop = new DomProperty;
    prop->setAttributeName("iconText");
    string = new DomString;
    string->setText(elt.iconText);
    prop->setElementString(string);
    prop_list.append(prop);

    prop = new DomProperty;
    prop->setAttributeName("shortcut");
    string = new DomString;
    string->setText(elt.shortcut);
    prop->setElementString(string);
    prop_list.append(prop);

    result->setElementProperty(prop_list);

    return result;
}

QWidget *QDesignerResource::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parentWidget))
        parentWidget = promoted->child();

    QString className = ui_widget->attributeClass();
    if (!m_isMainWidget && className == QLatin1String("QWidget") && ui_widget->elementLayout().size()) {
        // ### check if elementLayout.size() == 1
        ui_widget->setAttributeClass("QLayoutWidget");
    }

    QWidget *w = Resource::create(ui_widget, parentWidget);
    if (!w)
        return 0;

    ui_widget->setAttributeClass(className); // fix the class name

    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(w)) {
        QWidget *central_widget = createWidget(QLatin1String("QWidget"), mainWindow, "__qt_central_widget");
        Q_ASSERT(qobject_cast<QDesignerWidget*>(central_widget));
        mainWindow->setCentralWidget(central_widget);
    }

    QList<DomActionRef*> action_ref_list = ui_widget->elementAddAction();
    foreach (DomActionRef *action_ref, action_ref_list) {
        if (action_ref->hasAttributeName())
            m_formWindow->widgetToActionMap().add(w, action_ref->attributeName());
    }

    QList<DomAction*> action_list = ui_widget->elementAction();
    foreach (DomAction *action, action_list)
        m_formWindow->actionList().append(createActionListElt(action));

    if (IExtraInfo *extra = qt_extension<IExtraInfo*>(core()->extensionManager(), m_formWindow)) {
        extra->loadExtraInfo(w, ui_widget);
    }

    return w;
}

QLayout *QDesignerResource::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parentWidget))
        parentWidget = promoted->child();

    QLayout *l = Resource::create(ui_layout, layout, parentWidget);

    if (QGridLayout *gridLayout = qobject_cast<QGridLayout*>(l))
        QLayoutSupport::createEmptyCells(gridLayout);

    if (IExtraInfo *extra = qt_extension<IExtraInfo*>(core()->extensionManager(), layout)) {
        extra->loadExtraInfo(layout, ui_layout);
    }

    return l;
}

QLayoutItem *QDesignerResource::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parentWidget))
        parentWidget = promoted->child();

    if (ui_layoutItem->kind() == DomLayoutItem::Spacer) {
        QHash<QString, DomProperty*> properties = propertyMap(ui_layoutItem->elementSpacer()->elementProperty());

        Spacer *spacer = (Spacer*) m_core->widgetFactory()->createWidget("Spacer", parentWidget);

        spacer->setInteraciveMode(false);
        applyProperties(spacer, ui_layoutItem->elementSpacer()->elementProperty());
        spacer->setInteraciveMode(true);

        if (m_formWindow) {
            m_formWindow->manageWidget(spacer);
            if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(m_core->extensionManager(), spacer))
                sheet->setChanged(sheet->indexOf("orientation"), true);
        }

        return new QWidgetItem(spacer);
    } else if (ui_layoutItem->kind() == DomLayoutItem::Layout && parentWidget) {
        DomLayout *ui_layout = ui_layoutItem->elementLayout();
        QLayoutWidget *layoutWidget = new QLayoutWidget(m_formWindow, parentWidget);
        applyProperties(layoutWidget, ui_layout->elementProperty());

        if (m_formWindow) {
            m_formWindow->manageWidget(layoutWidget);
        }

        (void) create(ui_layout, 0, layoutWidget);
        return new QWidgetItem(layoutWidget);
    }
    return Resource::create(ui_layoutItem, layout, parentWidget);
}

void QDesignerResource::changeObjectName(QObject *o, QString objName)
{
    if (m_formWindow)
        m_formWindow->unify(o, objName, true);

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(o)) {
        if (objName.startsWith(QLatin1String("__qt__promoted_"))) {
            promoted->setObjectName(objName);
            promoted->child()->setObjectName(objName.mid(15));
        } else {
            promoted->child()->setObjectName(objName);
            promoted->setObjectName(QLatin1String("__qt__promoted_") + objName);
        }
    } else {
        o->setObjectName(objName);
    }

}

void QDesignerResource::applyProperties(QObject *o, const QList<DomProperty*> &properties)
{
    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(m_core->extensionManager(), o)) {
        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);
            QString propertyName = properties.at(i)->attributeName();
            int index = sheet->indexOf(propertyName);
            if (index != -1) {
                QVariant v;
                if (p->kind() == DomProperty::IconSet || p->kind() == DomProperty::Pixmap) {
                    DomResourcePixmap *resource = 0;
                    if (p->kind() == DomProperty::IconSet)
                        resource = p->elementIconSet();
                    else
                        resource = p->elementPixmap();

                    if (resource != 0) {
                        QString icon_path = resource->text();
                        QString qrc_path = resource->attributeResource();

                        if (qrc_path.isEmpty())
                            icon_path = m_formWindow->absolutePath(icon_path);
                        else
                            qrc_path = m_formWindow->absolutePath(qrc_path);

                        if (p->kind() == DomProperty::IconSet) {
                            QIcon icon = m_core->iconCache()->nameToIcon(icon_path, qrc_path);
                            v = qVariantFromValue(icon);
                        } else {
                            QPixmap pixmap = m_core->iconCache()->nameToPixmap(icon_path, qrc_path);
                            v = qVariantFromValue(pixmap);
                        }
                    }
                } else {
                    v = toVariant(o->metaObject(), p);
                }
                sheet->setProperty(index, v);
                sheet->setChanged(index, true);
            }

            if (propertyName == QLatin1String("objectName"))
                changeObjectName(o, o->objectName());
        }
    }
}

QWidget *QDesignerResource::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &_name)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parentWidget))
        parentWidget = promoted->child();

    QString name = _name;
    QString className = widgetName;
    if (m_isMainWidget)
        m_isMainWidget = false;

    QWidget *w = m_core->widgetFactory()->createWidget(className, parentWidget);
    if (!w)
        return 0;

    if (name.isEmpty()) {
        AbstractWidgetDataBase *db = m_core->widgetDataBase();
        if (AbstractWidgetDataBaseItem *item = db->item(db->indexOfObject(w)))
            name = qtify(item->name());
    }

    changeObjectName(w, name);

    IContainer *container = qt_extension<IContainer*>(m_core->extensionManager(), parentWidget);
    if (!parentWidget || !container) {
        m_formWindow->manageWidget(w);
    } else {
        m_core->metaDataBase()->add(w);
    }

    return w;
}

QLayout *QDesignerResource::createLayout(const QString &layoutName, QObject *parent, const QString &name)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parent))
        parent = promoted->child();

    QWidget *layoutBase = 0;
    QLayout *layout = qobject_cast<QLayout*>(parent);

    if (parent->isWidgetType())
        layoutBase = static_cast<QWidget*>(parent);
    else {
        Q_ASSERT( layout != 0 );
        layoutBase = layout->parentWidget();
    }

    LayoutInfo::Type layoutType = LayoutInfo::Grid;
    if (layoutName == QLatin1String("QVBoxLayout"))
        layoutType = LayoutInfo::VBox;
    else if (layoutName == QLatin1String("QHBoxLayout"))
        layoutType = LayoutInfo::HBox;
    else if (layoutName == QLatin1String("QStackedLayout"))
        layoutType = LayoutInfo::Stacked;

    if (QLayout *lay = m_core->widgetFactory()->createLayout(layoutBase, layout, layoutType)) {
        changeObjectName(lay, name);
        return lay;
    }

    return 0;
}

// save
DomWidget *QDesignerResource::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
    AbstractMetaDataBaseItem *item = m_core->metaDataBase()->item(widget);
    if (!item)
        return 0;

    if (qobject_cast<Spacer*>(widget) && m_copyWidget == false) {
        ++m_topLevelSpacerCount;
        return 0;
    }

    int widgetInfoIndex = m_core->widgetDataBase()->indexOfObject(widget, false);
    if (widgetInfoIndex != -1) {
        AbstractWidgetDataBaseItem *widgetInfo = m_core->widgetDataBase()->item(widgetInfoIndex);
        if (widgetInfo->isCustom())
            m_usedCustomWidgets.insert(widgetInfo, true);
    }

    DomWidget *w = 0;

    if (QDesignerTabWidget *tabWidget = qobject_cast<QDesignerTabWidget*>(widget))
        w = saveWidget(tabWidget, ui_parentWidget);
    else if (QDesignerStackedWidget *stackedWidget = qobject_cast<QDesignerStackedWidget*>(widget))
        w = saveWidget(stackedWidget, ui_parentWidget);
    else if (QDesignerToolBox *toolBox = qobject_cast<QDesignerToolBox*>(widget))
        w = saveWidget(toolBox, ui_parentWidget);
    else if (IContainer *container = qt_extension<IContainer*>(m_core->extensionManager(), widget))
        w = saveWidget(widget, container, ui_parentWidget);
    else if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(widget))
        w = Resource::createDom(promoted->child(), ui_parentWidget, recursive);
    else
        w = Resource::createDom(widget, ui_parentWidget, recursive);

    Q_ASSERT( w != 0 );

    QString className = w->attributeClass();
    if (QDesignerCustomWidget *customWidget = qobject_cast<QDesignerCustomWidget*>(widget))
        w->setAttributeClass(customWidget->widgetClassName());
    else if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(widget))
        w->setAttributeClass(promoted->item()->name());
    else if (m_internal_to_qt.contains(className))
        w->setAttributeClass(m_internal_to_qt.value(className));

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(widget)) {
        w->setAttributeName(promoted->child()->objectName());
        QList<DomProperty*> prop_list = w->elementProperty();
        foreach (DomProperty *prop, prop_list) {
            if (prop->attributeName() == QLatin1String("geometry")) {
                if (DomRect *rect = prop->elementRect()) {
                    rect->setElementX(widget->x());
                    rect->setElementY(widget->y());
                }
                break;
            }
        }
    } else {
        w->setAttributeName(widget->objectName());
    }

    QList<DomActionRef*> ref_list;
    QStringList action_list = m_formWindow->widgetToActionMap().actions(widget);
    foreach (QString s, action_list) {
        DomActionRef *ref = new DomActionRef;
        ref->setAttributeName(s);
        ref_list.append(ref);
    }
    if (!ref_list.isEmpty())
        w->setElementAddAction(ref_list);

    if (widget == m_formWindow->mainContainer()) {
        QList<DomAction*> dom_action_list;
        ActionList action_list = m_formWindow->actionList();
        foreach (ActionListElt elt, action_list) {
            dom_action_list.append(createDomAction(elt));
        }
        if (!dom_action_list.isEmpty())
            w->setElementAction(dom_action_list);
    }


    if (IExtraInfo *extra = qt_extension<IExtraInfo*>(core()->extensionManager(), widget)) {
        extra->saveExtraInfo(w, widget);
    }

    return w;
}

DomLayout *QDesignerResource::createDom(QLayout *layout, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
    if (!m_core->metaDataBase()->item(layout)) {
        return 0;
    }

    m_chain.push(layout);

    DomLayout *l = Resource::createDom(layout, ui_layout, ui_parentWidget);

    QString className = l->attributeClass();
    if (m_internal_to_qlayout.contains(className))
        l->setAttributeClass(m_internal_to_qlayout.value(className));
    m_chain.pop();

    if (IExtraInfo *extra = qt_extension<IExtraInfo*>(core()->extensionManager(), layout)) {
        extra->saveExtraInfo(l, layout);
    }

    return l;
}

DomLayoutItem *QDesignerResource::createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
    DomLayoutItem *ui_item = 0;

    if (Spacer *s = qobject_cast<Spacer*>(item->widget())) {
        if (!m_core->metaDataBase()->item(s))
            return 0;

        DomSpacer *spacer = new DomSpacer();
        QList<DomProperty*> properties = computeProperties(item->widget());
        // ### filter the properties
        spacer->setElementProperty(properties);

        ui_item = new DomLayoutItem();
        ui_item->setElementSpacer(spacer);
        m_laidout.insert(item->widget(), true);
    } else if (QLayoutWidget *layoutWidget = qobject_cast<QLayoutWidget*>(item->widget())) {
        Q_ASSERT(layoutWidget->layout());
        DomLayout *l = createDom(layoutWidget->layout(), ui_layout, ui_parentWidget);
        ui_item = new DomLayoutItem();
        ui_item->setElementLayout(l);
        m_laidout.insert(item->widget(), true);
    } else if (!item->spacerItem()) { // we use spacer as fake item in the Designer
        ui_item = Resource::createDom(item, ui_layout, ui_parentWidget);
    }

    if (m_chain.size() && item->widget()) {
        if (QGridLayout *grid = qobject_cast<QGridLayout*>(m_chain.top())) {
            int index = Utils::indexOfWidget(grid, item->widget());

            int row, column, rowspan, colspan;
            grid->getItemPosition(index, &row, &column, &rowspan, &colspan);
            ui_item->setAttributeRow(row);
            ui_item->setAttributeColumn(column);

            if (colspan != 1)
                ui_item->setAttributeColSpan(colspan);

            if (rowspan != 1)
                ui_item->setAttributeRowSpan(rowspan);
        }
    }

    return ui_item;
}


QString QDesignerResource::saveAuthor()
{
    return m_formWindow->author();
}

QString QDesignerResource::saveComment()
{
    return m_formWindow->comment();
}

void QDesignerResource::createCustomWidgets(DomCustomWidgets *dom_custom_widgets)
{
    if (dom_custom_widgets == 0)
        return;
    QList<DomCustomWidget*> custom_widget_list = dom_custom_widgets->elementCustomWidget();
    AbstractWidgetDataBase *db = m_formWindow->core()->widgetDataBase();
    foreach(DomCustomWidget *custom_widget, custom_widget_list) {
        WidgetDataBaseItem *item
            = new WidgetDataBaseItem(custom_widget->elementClass());
        QString base_class = custom_widget->elementExtends();
        item->setExtends(base_class);
        item->setPromoted(!base_class.isEmpty());
        item->setGroup(base_class.isEmpty() ? QObject::tr("Custom Widgets")
                                                : QObject::tr("Promoted Widgets"));
        if (DomHeader *header = custom_widget->elementHeader())
            item->setIncludeFile(header->text());
        item->setContainer(custom_widget->elementContainer());
        item->setCustom(true);
        db->append(item);
    }
}

void QDesignerResource::createAuthor(const QString &author)
{
    m_formWindow->setAuthor(author);
}

void QDesignerResource::createComment(const QString &comment)
{
    m_formWindow->setComment(comment);
}

DomTabStops *QDesignerResource::saveTabStops()
{
    AbstractMetaDataBaseItem *item = m_core->metaDataBase()->item(m_formWindow);
    Q_ASSERT(item);

    QStringList tabStops;
    foreach (QWidget *widget, item->tabOrder())
        tabStops.append(widget->objectName());

    if (tabStops.count()) {
        DomTabStops *dom = new DomTabStops;
        dom->setElementTabStop(tabStops);
        return dom;
    }

    return 0;
}

void QDesignerResource::applyTabStops(QWidget *widget, DomTabStops *tabStops)
{
    if (!tabStops)
        return;

    QList<QWidget*> tabOrder;
    foreach (QString widgetName, tabStops->elementTabStop()) {
        if (QWidget *w = qFindChild<QWidget*>(widget, widgetName)) {
            tabOrder.append(w);
        }
    }

    AbstractMetaDataBaseItem *item = m_core->metaDataBase()->item(m_formWindow);
    Q_ASSERT(item);
    item->setTabOrder(tabOrder);
}

DomWidget *QDesignerResource::saveWidget(QWidget *widget, IContainer *container, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = Resource::createDom(widget, ui_parentWidget, false);
    QList<DomWidget*> ui_widget_list;

    for (int i=0; i<container->count(); ++i) {
        QWidget *page = container->widget(i);
        Q_ASSERT(page);

        DomWidget *ui_page = createDom(page, ui_widget);
        Q_ASSERT( ui_page != 0 );

        ui_widget_list.append(ui_page);
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QDesignerStackedWidget *widget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = Resource::createDom(widget, ui_parentWidget, false);
    QList<DomWidget*> ui_widget_list;
    if (IContainer *container = qt_extension<IContainer*>(m_core->extensionManager(), widget)) {
        for (int i=0; i<container->count(); ++i) {
            QWidget *page = container->widget(i);
            Q_ASSERT(page);

            DomWidget *ui_page = createDom(page, ui_widget);
            Q_ASSERT( ui_page != 0 );

            ui_widget_list.append(ui_page);
        }
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QDesignerTabWidget *widget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = Resource::createDom(widget, ui_parentWidget, false);
    QList<DomWidget*> ui_widget_list;

    if (IContainer *container = qt_extension<IContainer*>(m_core->extensionManager(), widget)) {
        for (int i=0; i<container->count(); ++i) {
            QWidget *page = container->widget(i);
            Q_ASSERT(page);

            DomWidget *ui_page = createDom(page, ui_widget);
            Q_ASSERT( ui_page != 0 );

            // attribute `title'
            DomProperty *p = new DomProperty();
            p->setAttributeName("title");
            DomString *str = new DomString();
            str->setText(widget->tabText(i));
            p->setElementString(str);

            QList<DomProperty*> ui_attribute_list;
            ui_attribute_list.append(p);

            ui_page->setElementAttribute(ui_attribute_list);

            ui_widget_list.append(ui_page);
        }
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QDesignerToolBox *widget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = Resource::createDom(widget, ui_parentWidget, false);
    QList<DomWidget*> ui_widget_list;

    if (IContainer *container = qt_extension<IContainer*>(m_core->extensionManager(), widget)) {
        for (int i=0; i<container->count(); ++i) {
            QWidget *page = container->widget(i);
            Q_ASSERT(page);

            DomWidget *ui_page = createDom(page, ui_widget);
            Q_ASSERT( ui_page != 0 );

            // attribute `label'
            DomProperty *p = new DomProperty();
            p->setAttributeName("label");
            DomString *str = new DomString();
            str->setText(widget->itemText(i));
            p->setElementString(str); // ### check f tb->indexOf(page) == i ??

            QList<DomProperty*> ui_attribute_list;
            ui_attribute_list.append(p);

            ui_page->setElementAttribute(ui_attribute_list);

            ui_widget_list.append(ui_page);
        }
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

bool QDesignerResource::checkProperty(QDesignerStackedWidget *widget, const QString &prop) const
{
    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(m_core->extensionManager(), widget))
        return sheet->isAttribute(sheet->indexOf(prop)) == false;

    return true;
}

bool QDesignerResource::checkProperty(QObject *obj, const QString &prop) const
{
    if (!checkProperty(qobject_cast<QDesignerTabWidget*>(obj), prop))
        return false;
    else if (!checkProperty(qobject_cast<QDesignerToolBox*>(obj), prop))
        return false;
    else if (!checkProperty(qobject_cast<QLayoutWidget*>(obj), prop))
        return false;

    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(m_core->extensionManager(), obj))
        return sheet->isChanged(sheet->indexOf(prop));

    return false;
}

bool QDesignerResource::checkProperty(QLayoutWidget *widget, const QString &prop) const
{
    if (!widget)
        return true;

    return widget->QWidget::metaObject()->indexOfProperty(prop.toLatin1()) != -1;
}

bool QDesignerResource::checkProperty(QDesignerTabWidget *widget, const QString &prop) const
{
    if (!widget)
        return true;

    return widget->QTabWidget::metaObject()->indexOfProperty(prop.toLatin1()) != -1;
}

bool QDesignerResource::checkProperty(QDesignerToolBox *widget, const QString &prop) const
{
    if (!widget)
        return true;

    return widget->QToolBox::metaObject()->indexOfProperty(prop.toLatin1()) != -1;
}

bool QDesignerResource::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    QGridLayout *grid = qobject_cast<QGridLayout*>(layout);

    if (grid && item->widget()) {
        int rowSpan = ui_item->hasAttributeRowSpan() ? ui_item->attributeRowSpan() : 1;
        int colSpan = ui_item->hasAttributeColSpan() ? ui_item->attributeColSpan() : 1;
        grid->addWidget(item->widget(), ui_item->attributeRow(), ui_item->attributeColumn(),
                        rowSpan, colSpan, item->alignment());
        return true;
    }

    return Resource::addItem(ui_item, item, layout);
}

bool QDesignerResource::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    if (Resource::addItem(ui_widget, widget, parentWidget))
        return true;

    if (IContainer *container = qt_extension<IContainer*>(m_core->extensionManager(), parentWidget)) {
        container->addWidget(widget);
        return true;
    }

    return false;
}

void QDesignerResource::copy(QIODevice *dev, const QList<QWidget*> &selection)
{
    m_copyWidget = true;

    DomUI *ui = copy(selection);
    QDomDocument doc;
    doc.appendChild(ui->write(doc));
    dev->write(doc.toString().toUtf8());

    m_laidout.clear();

    delete ui;
    m_copyWidget = false;
}

DomUI *QDesignerResource::copy(const QList<QWidget*> &selection)
{
    m_copyWidget = true;

    DomUI *ui = new DomUI();
    ui->setAttributeVersion(QLatin1String("4.0"));

    DomWidget *ui_widget = new DomWidget();
    QList<DomWidget*> ui_widget_list;
    ui_widget->setAttributeName("__qt_fake_top_level");

    for (int i=0; i<selection.size(); ++i) {
        QWidget *w = selection.at(i);
        DomWidget *ui_child = createDom(w, ui_widget);
        if (!ui_child)
            continue;

        ui_widget_list.append(ui_child);
    }

    ui_widget->setElementWidget(ui_widget_list);
    ui->setElementWidget(ui_widget);

    m_laidout.clear();

    m_copyWidget = false;

    return ui;
}

QList<QWidget*> QDesignerResource::paste(DomUI *ui, QWidget *parentWidget)
{
    int saved = m_isMainWidget;
    m_isMainWidget = false;
    QList<QWidget*> createdWidgets;

    DomWidget *topLevel = ui->elementWidget();
    QList<DomWidget*> widgets = topLevel->elementWidget();
    for (int i=0; i<widgets.size(); ++i) {
        QWidget *w = create(widgets.at(i), parentWidget);
        if (!w)
            continue;

        w->move(w->pos() + m_formWindow->grid());
        // ### change the init properties of w
        createdWidgets.append(w);
    }

    m_isMainWidget = saved;

    return createdWidgets;
}

QList<QWidget*> QDesignerResource::paste(QIODevice *dev, QWidget *parentWidget)
{
    QDomDocument doc;
    if (!doc.setContent(dev))
        return QList<QWidget*>();

    QDomElement root = doc.firstChild().toElement();
    DomUI ui;
    ui.read(root);
    return paste(&ui, parentWidget);
}

void QDesignerResource::layoutInfo(DomWidget *widget, QObject *parent, int *margin, int *spacing)
{
    Resource::layoutInfo(widget, parent, margin, spacing);

    if (margin && qobject_cast<QLayoutWidget*>(parent))
        *margin = 0;
}

QString QDesignerResource::qtify(const QString &name)
{
    QString qname = name;

    if (qname.at(0) == QLatin1Char('Q') || qname.at(0) == QLatin1Char('K'))
        qname = qname.mid(1);

    int i=0;
    while (i < qname.length()) {
        if (qname.at(i).toLower() != qname.at(i))
            qname[i] = qname.at(i).toLower();
        else
            break;

        ++i;
    }

    return qname;
}

DomCustomWidgets *QDesignerResource::saveCustomWidgets()
{
    if (m_usedCustomWidgets.isEmpty())
        return 0;

    QList<DomCustomWidget*> custom_widget_list;
    foreach (AbstractWidgetDataBaseItem *item, m_usedCustomWidgets.keys()) {
        DomCustomWidget *custom_widget = new DomCustomWidget;
        custom_widget->setElementClass(item->name());
        custom_widget->setElementContainer(item->isContainer());

        DomHeader *header = new DomHeader;
        header->setText(item->includeFile());
        custom_widget->setElementHeader(header);
        custom_widget->setElementExtends(item->extends());

        custom_widget_list.append(custom_widget);
    }

    DomCustomWidgets *customWidgets = new DomCustomWidgets;
    customWidgets->setElementCustomWidget(custom_widget_list);
    return customWidgets;
}

QList<DomProperty*> QDesignerResource::computeProperties(QObject *object)
{
    QList<DomProperty*> properties;
    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(m_core->extensionManager(), object)) {
        for (int index = 0; index < sheet->count(); ++index) {
            QString propertyName = sheet->propertyName(index);
            QVariant value = sheet->property(index);

            if (!sheet->isChanged(index))
                continue;

            if (DomProperty *p = createProperty(object, propertyName, value)) {
                properties.append(p);
            }
        }
    }
    return properties;
}

DomProperty *QDesignerResource::createProperty(QObject *object, const QString &propertyName, const QVariant &value)
{
    if (qVariantCanConvert<EnumType>(value)) {
        EnumType e = qvariant_cast<EnumType>(value);
        int v = e.value.toInt();
        QMapIterator<QString, QVariant> it(e.items);
        while (it.hasNext()) {
            if (it.next().value().toInt() != v)
                continue;

            DomProperty *p = new DomProperty;
            p->setAttributeName(propertyName);
            p->setElementEnum(it.key());
            return p;
        }

        return 0;
    } else if (qVariantCanConvert<FlagType>(value)) {
        FlagType f = qvariant_cast<FlagType>(value);
        int v = f.value.toInt();
        QMapIterator<QString, QVariant> it(f.items);
        QStringList keys;

        while (it.hasNext()) {
            int x = it.next().value().toInt();
            if (v == x) {
                DomProperty *p = new DomProperty;
                p->setAttributeName(propertyName);
                p->setElementSet(it.key());
                return p;
            }

            keys.push_back(it.key());
        }

        if (keys.isEmpty())
            return 0;

        DomProperty *p = new DomProperty;
        p->setAttributeName(propertyName);
        p->setElementSet(keys.join(QLatin1String("|")));
        return p;
    } else if (value.type() == QVariant::Pixmap || value.type() == QVariant::Icon) {
        DomResourcePixmap *r = new DomResourcePixmap;
        QString icon_path;
        QString qrc_path;
        if (value.type() == QVariant::Icon) {
            QIcon icon = qvariant_cast<QIcon>(value);
            icon_path = m_core->iconCache()->iconToFilePath(icon);
            qrc_path = m_core->iconCache()->iconToQrcPath(icon);
        } else {
            QPixmap pixmap = qvariant_cast<QPixmap>(value);
            icon_path = m_core->iconCache()->pixmapToFilePath(pixmap);
            qrc_path = m_core->iconCache()->pixmapToQrcPath(pixmap);
        }

        if (qrc_path.isEmpty())
            icon_path = m_formWindow->relativePath(icon_path);
        else
            qrc_path = m_formWindow->relativePath(qrc_path);

        r->setText(icon_path);
        if (!qrc_path.isEmpty())
            r->setAttributeResource(qrc_path);
        DomProperty *p = new DomProperty;

        if (value.type() == QVariant::Icon)
            p->setElementIconSet(r);
        else
            p->setElementPixmap(r);

        p->setAttributeName(propertyName);
        return p;
    }

    return Resource::createProperty(object, propertyName, value);
}

void QDesignerResource::createResources(DomResources *resources)
{
    if (resources == 0)
        return;

    QList<DomResource*> dom_include = resources->elementInclude();
    foreach (DomResource *res, dom_include) {
        QString path = m_formWindow->absolutePath(res->attributeLocation());
        m_formWindow->addResourceFile(path);
    }
}

DomResources *QDesignerResource::saveResources()
{
    QStringList res_list = m_formWindow->resourceFiles();
    QList<DomResource*> dom_include;
    foreach (QString res, res_list) {
        DomResource *dom_res = new DomResource;
        dom_res->setAttributeLocation(m_formWindow->relativePath(res));
        dom_include.append(dom_res);
    }

    DomResources *dom_resources = new DomResources;
    dom_resources->setElementInclude(dom_include);

    return dom_resources;
}
