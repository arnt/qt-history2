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

#include "formeditor.h"
#include "metadatabase_p.h"
#include "widgetdatabase_p.h"
#include "widgetfactory_p.h"
#include "formwindowmanager.h"
#include "qmainwindow_container.h"
#include "qdockwidget_container.h"
#include "qworkspace_container.h"
#include "qmdiarea_container.h"
#include "default_container.h"
#include "default_layoutdecoration.h"
#include "default_actionprovider.h"
#include "qlayoutwidget_propertysheet.h"
#include "spacer_propertysheet.h"
#include "line_propertysheet.h"
#include "layout_propertysheet.h"
#include "qdesigner_stackedbox_p.h"
#include "qdesigner_toolbox_p.h"
#include "qdesigner_tabwidget_p.h"
#include "qtbrushmanager.h"
#include "brushmanagerproxy.h"
#include "iconcache.h"

// sdk
#include <QtDesigner/QExtensionManager>

// shared
#include <pluginmanager_p.h>
#include <qdesigner_taskmenu_p.h>
#include <qdesigner_membersheet_p.h>
#include <qdesigner_promotion_p.h>

namespace qdesigner_internal {

FormEditor::FormEditor(QObject *parent)
    : QDesignerFormEditorInterface(parent)
{
    QDesignerPluginManager *pluginManager = new QDesignerPluginManager(this);
    setPluginManager(pluginManager);

    WidgetDataBase *widgetDatabase = new WidgetDataBase(this);
    setWidgetDataBase(widgetDatabase);

    MetaDataBase *metaDataBase = new MetaDataBase(this);
    setMetaDataBase(metaDataBase);

    WidgetFactory *widgetFactory = new WidgetFactory(this);
    setWidgetFactory(widgetFactory);

    FormWindowManager *formWindowManager = new FormWindowManager(this, this);
    setFormManager(formWindowManager);

    QExtensionManager *mgr = new QExtensionManager(this);
    const QString containerExtensionId = Q_TYPEID(QDesignerContainerExtension);

    QDesignerStackedWidgetContainerFactory::registerExtension(mgr, containerExtensionId);
    QDesignerTabWidgetContainerFactory::registerExtension(mgr, containerExtensionId);
    QDesignerToolBoxContainerFactory::registerExtension(mgr, containerExtensionId);
    QMainWindowContainerFactory::registerExtension(mgr, containerExtensionId);
    QDockWidgetContainerFactory::registerExtension(mgr, containerExtensionId);
    QWorkspaceContainerFactory::registerExtension(mgr, containerExtensionId);
    QMdiAreaContainerFactory::registerExtension(mgr, containerExtensionId);

    mgr->registerExtensions(new QDesignerLayoutDecorationFactory(mgr),      Q_TYPEID(QDesignerLayoutDecorationExtension));
    mgr->registerExtensions(new QDesignerActionProviderFactory(mgr),        Q_TYPEID(QDesignerActionProviderExtension));

    QDesignerDefaultPropertySheetFactory::registerExtension(mgr);
    QLayoutWidgetPropertySheetFactory::registerExtension(mgr);
    SpacerPropertySheetFactory::registerExtension(mgr);
    LinePropertySheetFactory::registerExtension(mgr);
    LayoutPropertySheetFactory::registerExtension(mgr);
    QStackedWidgetPropertySheetFactory::registerExtension(mgr);
    QToolBoxWidgetPropertySheetFactory::registerExtension(mgr);
    QTabWidgetPropertySheetFactory::registerExtension(mgr);

    mgr->registerExtensions(new QDesignerTaskMenuFactory(mgr),              Q_TYPEID(QDesignerTaskMenuExtension));

    mgr->registerExtensions(new QDesignerMemberSheetFactory(mgr),           Q_TYPEID(QDesignerMemberSheetExtension));

    setExtensionManager(mgr);

    setIconCache(new IconCache(this));

    QtBrushManager *brushManager = new QtBrushManager(this);
    setBrushManager(brushManager);

    BrushManagerProxy *brushProxy = new BrushManagerProxy(this, this);
    brushProxy->setBrushManager(brushManager);
    setPromotion(new QDesignerPromotion(this));
}

FormEditor::~FormEditor()
{
    delete formWindowManager();
    delete promotion();
}
}
