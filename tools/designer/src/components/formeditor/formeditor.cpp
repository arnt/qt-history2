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
#include "undomanager.h"
#include "default_propertysheet.h"
#include "default_container.h"
#include "default_layoutdecoration.h"
#include "qlayoutwidget_propertysheet.h"
#include "spacer_propertysheet.h"

#include <signalsloteditor.h>

#include <pluginmanager.h>
#include <qextensionmanager.h>

FormEditor::FormEditor(QObject *parent)
    : AbstractFormEditor(parent)
{
    WidgetDataBase *widgetDatabase = new WidgetDataBase(this);
    setWidgetDataBase(widgetDatabase);
    
    MetaDataBase *metaDataBase = new MetaDataBase(this);
    setMetaDataBase(metaDataBase);
    
    WidgetFactory *widgetFactory = new WidgetFactory(this);
    setWidgetFactory(widgetFactory);
    
    FormWindowManager *formManager = new FormWindowManager(this, parent);
    setFormManager(formManager);
    
    UndoManager *undoManager = new UndoManager(this, parent);
    setUndoManager(undoManager);

    QExtensionManager *mgr = new QExtensionManager(this);
    mgr->registerExtensions(new QDesignerPropertySheetFactory(mgr),         Q_TYPEID(IPropertySheet));
    mgr->registerExtensions(new QDesignerContainerFactory(mgr),             Q_TYPEID(IContainer));
    mgr->registerExtensions(new QDesignerLayoutDecorationFactory(mgr),      Q_TYPEID(ILayoutDecoration));
    mgr->registerExtensions(new QLayoutWidgetPropertySheetFactory(mgr),     Q_TYPEID(IPropertySheet));
    mgr->registerExtensions(new SpacerPropertySheetFactory(mgr),            Q_TYPEID(IPropertySheet));
    setExtensionManager(mgr);

    SignalSlotEditor::registerExtensions(this);

    PluginManager pluginManager;
    pluginManager.registerPath("/home/rraggi/dev/ide/main/plugins");
        
    // load the plugins
    widgetDatabase->loadPlugins();
    widgetFactory->loadPlugins();
}

FormEditor::~FormEditor()
{
    delete formManager();
}
