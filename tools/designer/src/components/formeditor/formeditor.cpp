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

#include "formeditor.h"
#include "metadatabase.h"
#include "widgetdatabase.h"
#include "widgetfactory.h"
#include "formwindowmanager.h"
#include "default_container.h"
#include "default_layoutdecoration.h"
#include "qlayoutwidget_propertysheet.h"
#include "spacer_propertysheet.h"
#include "iconcache.h"

#include <signalsloteditor.h>

#include <pluginmanager.h>
#include <qextensionmanager.h>
#include <qdesigner_taskmenu.h>
#include <qdesigner_propertysheet.h>
#include <qdesigner_promotedwidget.h>

FormEditor::FormEditor(QObject *parent)
    : AbstractFormEditor(parent)
{
    PluginManager *pluginManager = new PluginManager(this);
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
    mgr->registerExtensions(new QDesignerPropertySheetFactory(mgr),         Q_TYPEID(IPropertySheet));
    mgr->registerExtensions(new QDesignerContainerFactory(mgr),             Q_TYPEID(IContainer));
    mgr->registerExtensions(new QDesignerLayoutDecorationFactory(mgr),      Q_TYPEID(ILayoutDecoration));
    mgr->registerExtensions(new QLayoutWidgetPropertySheetFactory(mgr),     Q_TYPEID(IPropertySheet));
    mgr->registerExtensions(new SpacerPropertySheetFactory(mgr),            Q_TYPEID(IPropertySheet));
    mgr->registerExtensions(new PromotedWidgetPropertySheetFactory(mgr),    Q_TYPEID(IPropertySheet));

    mgr->registerExtensions(new QDesignerTaskMenuFactory(mgr),              Q_TYPEID(ITaskMenu));

    setExtensionManager(mgr);

    SignalSlotEditor::registerExtensions(this);

    // load the plugins
    widgetDatabase->loadPlugins();
    widgetDatabase->grabDefaultPropertyValues();
    widgetFactory->loadPlugins();

    setIconCache(new IconCache(this));

}

FormEditor::~FormEditor()
{
    delete formWindowManager();
}
