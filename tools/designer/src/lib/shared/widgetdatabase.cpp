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

#include "widgetdatabase_p.h"
#include "widgetfactory_p.h"
#include "spacer_widget_p.h"
#include "abstractlanguage.h"

#include <pluginmanager_p.h>
#include <QtDesigner/customwidget.h>
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/QExtensionManager>

#include <qalgorithms.h>
#include <QtCore/qdebug.h>
#include <QtCore/QMetaProperty>

namespace {
    const bool debugWidgetDataBase=false;
    const  QLatin1String qWidgetName("QWidget");
}

namespace qdesigner_internal {

// ----------------------------------------------------------
WidgetDataBaseItem::WidgetDataBaseItem(const QString &name, const QString &group)
    : m_name(name),
      m_group(group),
      m_compat(0),
      m_container(0),
      m_form(0),
      m_custom(0),
      m_promoted(0)
{
}
    

QString WidgetDataBaseItem::name() const
{
    return m_name;
}

void WidgetDataBaseItem::setName(const QString &name)
{
    m_name = name;
}

QString WidgetDataBaseItem::group() const
{
    return m_group;
}

void WidgetDataBaseItem::setGroup(const QString &group)
{
    m_group = group;
}

QString WidgetDataBaseItem::toolTip() const
{
    return m_toolTip;
}

void WidgetDataBaseItem::setToolTip(const QString &toolTip)
{
    m_toolTip = toolTip;
}

QString WidgetDataBaseItem::whatsThis() const
{
    return m_whatsThis;
}

void WidgetDataBaseItem::setWhatsThis(const QString &whatsThis)
{
    m_whatsThis = whatsThis;
}

QString WidgetDataBaseItem::includeFile() const
{
    return m_includeFile;
}

void WidgetDataBaseItem::setIncludeFile(const QString &includeFile)
{
    m_includeFile = includeFile;
}

QIcon WidgetDataBaseItem::icon() const
{
    return m_icon;
}

void WidgetDataBaseItem::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

bool WidgetDataBaseItem::isCompat() const
{
    return m_compat;
}

void WidgetDataBaseItem::setCompat(bool b)
{
    m_compat = b;
}

bool WidgetDataBaseItem::isContainer() const
{
    return m_container;
}

void WidgetDataBaseItem::setContainer(bool b)
{
    m_container = b;
}

bool WidgetDataBaseItem::isCustom() const
{
    return m_custom;
}

void WidgetDataBaseItem::setCustom(bool b)
{
    m_custom = b;
}

QString WidgetDataBaseItem::pluginPath() const
{
    return m_pluginPath;
}

void WidgetDataBaseItem::setPluginPath(const QString &path)
{
    m_pluginPath = path;
}

bool WidgetDataBaseItem::isPromoted() const
{
    return m_promoted;
}

void WidgetDataBaseItem::setPromoted(bool b)
{
    m_promoted = b;
}

QString WidgetDataBaseItem::extends() const
{
    return m_extends;
}

void WidgetDataBaseItem::setExtends(const QString &s)
{
    m_extends = s;
}

void WidgetDataBaseItem::setDefaultPropertyValues(const QList<QVariant> &list)
{
    m_defaultPropertyValues = list;
}

QList<QVariant> WidgetDataBaseItem::defaultPropertyValues() const
{
    return m_defaultPropertyValues;
}

// ----------------------------------------------------------
WidgetDataBase::WidgetDataBase(QDesignerFormEditorInterface *core, QObject *parent)
    : QDesignerWidgetDataBaseInterface(parent),
      m_core(core)
{
#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C) DECLARE_WIDGET(W, C)
#define DECLARE_WIDGET(W, C) append(new WidgetDataBaseItem(QString::fromUtf8(#W)));

#include "widgets.table"

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET
#undef DECLARE_WIDGET_1

    append(new WidgetDataBaseItem(QString::fromUtf8("Line")));
    append(new WidgetDataBaseItem(QString::fromUtf8("Spacer")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QSplitter")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QLayoutWidget")));
    // QDesignerWidget is used as central widget and as container for tab widgets, etc.
    WidgetDataBaseItem *designerWidgetItem = new WidgetDataBaseItem(QString::fromUtf8("QDesignerWidget"));
    designerWidgetItem->setContainer(true);
    append(designerWidgetItem);
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerDialog")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerToolBar")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerMenu")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerMenuBar")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerDockWidget")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QDesignerQ3WidgetStack")));
    append(new WidgetDataBaseItem(QString::fromUtf8("QAction")));

    // ### remove me
    // ### check the casts

#if 0 // ### enable me after 4.1
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QToolBar"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDesignerToolBar"))))->setContainer(true);
#endif

    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QTabWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QGroupBox"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QStackedWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QToolBox"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QFrame"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QLayoutWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDesignerWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDesignerDialog"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QSplitter"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QMainWindow"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDockWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDesignerDockWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDesignerQ3WidgetStack"))))->setContainer(true);

    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QWidget"))))->setContainer(true);
    static_cast<WidgetDataBaseItem *>(item(indexOfClassName(QLatin1String("QDialog"))))->setContainer(true);
}

WidgetDataBase::~WidgetDataBase()
{
}

QDesignerFormEditorInterface *WidgetDataBase::core() const
{
    return m_core;
}

int WidgetDataBase::indexOfObject(QObject *object, bool /*resolveName*/) const
{
    QExtensionManager *mgr = m_core->extensionManager();
    QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*> (mgr, m_core);

    QString id;

    if (lang)
        id = lang->classNameOf(object);

    if (id.isEmpty())
        id = WidgetFactory::classNameOf(m_core,object);

    return QDesignerWidgetDataBaseInterface::indexOfClassName(id);
}

QDesignerWidgetDataBaseItemInterface *WidgetDataBase::item(int index) const
{
    return QDesignerWidgetDataBaseInterface::item(index);
}

void WidgetDataBase::loadPlugins()
{
    QDesignerPluginManager *pluginManager = m_core->pluginManager();

    QStringList plugins = pluginManager->registeredPlugins();

    QMutableListIterator<QDesignerWidgetDataBaseItemInterface *> it(m_items);
    while (it.hasNext()) {
        QDesignerWidgetDataBaseItemInterface *item = it.next();

        if (item->isCustom()) {
            it.remove();
            delete item;
        }
    }

    pluginManager->ensureInitialized();

    foreach (QString plugin, plugins) {
        QObject *o = pluginManager->instance(plugin);

        if (QDesignerCustomWidgetInterface *c = qobject_cast<QDesignerCustomWidgetInterface*>(o)) {
            WidgetDataBaseItem *item = createCustomWidgetItem(c);
            item->setPluginPath(plugin);
            append(item);
        } else if (QDesignerCustomWidgetCollectionInterface *coll = qobject_cast<QDesignerCustomWidgetCollectionInterface*>(o)) {
            foreach (QDesignerCustomWidgetInterface *c, coll->customWidgets()) {
                WidgetDataBaseItem *item = createCustomWidgetItem(c);
                item->setPluginPath(plugin);
                append(item);
            }
        }
    }
}

WidgetDataBaseItem *WidgetDataBase::createCustomWidgetItem(const QDesignerCustomWidgetInterface *c) const
{
    WidgetDataBaseItem *item = new WidgetDataBaseItem(c->name(), c->group());
    item->setContainer(c->isContainer());
    item->setCustom(true);
    item->setIcon(c->icon());
    item->setIncludeFile(c->includeFile());
    item->setToolTip(c->toolTip());
    item->setWhatsThis(c->whatsThis());
    return item;
}

QList<QVariant> WidgetDataBase::defaultPropertyValues(const QString &name)
{
    const WidgetFactory factory(m_core);
    // Create non-widgets, widgets in order
    QObject* object = factory.createObject(name, 0);
    if (!object)
        object = factory.createWidget(name, 0);
    if (!object) {
        qWarning() << "** WARNING Factory failed to create " << name;
        return QList<QVariant>();
    }
    // Get properties from sheet.
    QList<QVariant> result;
    if (const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_core->extensionManager(), object)) {
        for (int i = 0; i < sheet->count(); ++i) {
            result.append(sheet->property(i));
        }
    }
    delete object;
    return result;
}

void WidgetDataBase::grabDefaultPropertyValues()
{
    for (int i = 0; i < count(); ++i) {
        QDesignerWidgetDataBaseItemInterface *dbItem = item(i);
        const QList<QVariant> default_prop_values = defaultPropertyValues(dbItem->name());
        dbItem->setDefaultPropertyValues(default_prop_values);

    }
}
    
namespace {
    // Clone an item.
    WidgetDataBaseItem *cloneItem(const QDesignerWidgetDataBaseItemInterface *item)    {
        WidgetDataBaseItem *rc = new WidgetDataBaseItem(item->name(), item->group());        
        rc->setToolTip(item->toolTip());
        rc->setWhatsThis(item->whatsThis());
        rc->setIncludeFile(item->includeFile());       
        rc->setIcon(item->icon());
        rc->setCompat(item->isCompat());
        rc->setContainer(item->isContainer());
        rc->setCustom(item->isCustom());
        rc->setPluginPath(item->pluginPath());
        rc->setPromoted(item->isPromoted());
        rc->setExtends(item->extends());
        rc->setDefaultPropertyValues(item->defaultPropertyValues());
        return rc;
    }

}
    
/* Appends a derived class to the database inheriting the data of the base class. Used
   for custom and promoted widgets.

   Depending on whether an entry exists, the existing or a newly created entry is
   returned. A return value of 0 indicates that the base class could not be found. */

QDESIGNER_SHARED_EXPORT QDesignerWidgetDataBaseItemInterface *
        appendDerived(QDesignerWidgetDataBaseInterface *db,
                      const QString &className, const QString &group,
                      const QString &baseClassName, const QString &includeFile,
                      bool promoted, bool custom)
        {
    if (debugWidgetDataBase) 
        qDebug() << "appendDerived " << className << " derived from " << baseClassName;
    // Check whether item already exists.
    QDesignerWidgetDataBaseItemInterface *derivedItem = 0;
    const int existingIndex = db->indexOfClassName(className);
    if ( existingIndex != -1)
        derivedItem =  db->item(existingIndex);
    if (derivedItem) {
        // Check the existing item for base class mismatch. This will likely
        // happen when loading a file written by an instance with missing plugins.
        // In that case, just warn and ignore the file properties.
        // 
        // An empty base class indicates that it is not known (for example, for custom plugins).
        // In this case, the widget DB is later updated once the widget is created
        // by DOM (by querying the metaobject). Suppress the warning.
        const QString existingBaseClass = derivedItem->extends();
        if (existingBaseClass.isEmpty() || baseClassName ==  existingBaseClass)
            return derivedItem;
        
        // Warn about mismatches
        qWarning() << "** WARNING The base class of " << className << " (" << baseClassName 
                   << ") does not correspond to the one in the widget database (" <<  existingBaseClass << ").";
        return derivedItem;
    }
    // Create this item, inheriting its base properties
    const int baseIndex = db->indexOfClassName(baseClassName);
    if (baseIndex == -1) {
        if (debugWidgetDataBase) 
            qDebug() << "appendDerived failed due to missing base class";
        return 0;
    }
    const QDesignerWidgetDataBaseItemInterface *baseItem = db->item(baseIndex);
    derivedItem = cloneItem(baseItem);
    // Sort of hack: If base class is QWidget, we most likely
    // do not want to inherit the container attribute.
    if (baseItem->name() == qWidgetName) 
        derivedItem->setContainer(false);
    // set new props
    derivedItem->setName(className);
    derivedItem->setGroup(group);
    derivedItem->setCustom(custom);
    derivedItem->setPromoted(promoted);
    derivedItem->setExtends(baseClassName);
    derivedItem->setIncludeFile(includeFile);
    db->append(derivedItem);
    return derivedItem;
}
 

} // namespace qdesigner_internal
