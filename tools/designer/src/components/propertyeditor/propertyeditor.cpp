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

#include "propertyeditor.h"
#include "newdynamicpropertydialog.h"
#include "dynamicpropertysheet.h"
#include "shared_enums_p.h"

// sdk
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QDesignerWidgetDataBaseInterface>
// shared
#include <qdesigner_utils_p.h>
#include <qdesigner_propertycommand_p.h>
#include <metadatabase_p.h>

#include <QtGui/QAction>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QApplication>
#include <QtGui/QVBoxLayout>
#include <QtGui/QScrollArea>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QActionGroup>
#include <QtGui/QLabel>

#include <QSignalMapper>

#include "qttreepropertybrowser.h"
#include "qtgroupboxpropertybrowser.h"
#include "qtvariantproperty.h"
#include "designerpropertymanager.h"

#include <iconloader_p.h>

#include <QDebug>

// ---------------------------------------------------------------------------------

namespace qdesigner_internal {

// A pair <ValidationMode, bool hasComment>.
typedef QPair<TextPropertyValidationMode, bool> StringPropertyParameters;

// Return a pair of validation mode and flag indicating whether property has a comment
// for textual properties.

StringPropertyParameters textPropertyValidationMode(const QObject *object,const QString &propertyName,
                                                    QVariant::Type type, bool isMainContainer)
{
    if (type == QVariant::ByteArray) {
        return StringPropertyParameters(ValidationMultiLine, false);
    }
    // object name - no comment
    if (propertyName == QLatin1String("objectName")) {
        const TextPropertyValidationMode vm =  isMainContainer ? ValidationObjectNameScope : ValidationObjectName;
        return StringPropertyParameters(vm, false);
    }

    // Accessibility. Both are texts the narrator reads
    if (propertyName == QLatin1String("accessibleDescription") || propertyName == QLatin1String("accessibleName"))
        return StringPropertyParameters(ValidationMultiLine, true);

    // Any names
    if (propertyName == QLatin1String("buddy") || propertyName.endsWith(QLatin1String("Name")))
        return StringPropertyParameters(ValidationObjectName, false);

    // Multi line?
    if (propertyName == QLatin1String("styleSheet"))
        return StringPropertyParameters(ValidationStyleSheet, false);

    if (propertyName == QLatin1String("styleSheet")     || propertyName == QLatin1String("toolTip")   ||
        propertyName.endsWith(QLatin1String("ToolTip")) || propertyName == QLatin1String("whatsThis") ||
        propertyName == QLatin1String("iconText")       || propertyName == QLatin1String("windowIconText")  ||
        propertyName == QLatin1String("html"))
        return StringPropertyParameters(ValidationMultiLine, true);


    // text only if not Action, LineEdit
    if (propertyName == QLatin1String("text") && !(qobject_cast<const QAction *>(object) || qobject_cast<const QLineEdit *>(object)))
        return StringPropertyParameters(ValidationMultiLine, true);

    // default to single
    return StringPropertyParameters(ValidationSingleLine, true);
}

QDesignerMetaDataBaseItemInterface* PropertyEditor::metaDataBaseItem() const 
{
    QObject *o = object();
    if (!o) 
        return 0;
    QDesignerMetaDataBaseInterface *db = core()->metaDataBase();
    if (!db) 
        return 0;
    return db->item(o);
}

void PropertyEditor::setupStringProperty(QtVariantProperty *property,
                const QString &propertyName, const QVariant &value, bool isMainContainer)
{
    const StringPropertyParameters params = textPropertyValidationMode(m_object, propertyName, value.type(), isMainContainer);
    // Does a meta DB entry exist - add comment
    const bool hasComment = params.second && metaDataBaseItem();
    const QString stringValue = value.type() == QVariant::ByteArray ? QString::fromUtf8(value.toByteArray()) : value.toString();
    property->setAttribute(QLatin1String("validationMode"), params.first);
    // assuming comment cannot appear or disappear for the same property in different object instance
    if (hasComment && !m_propertyToComment.contains(property)) {
        QtVariantProperty *commentProperty = m_propertyManager->addProperty(QVariant::String, tr("comment"));
        commentProperty->setValue(propertyComment(m_core, m_object, propertyName));
        property->addSubProperty(commentProperty);
        m_propertyToComment[property] = commentProperty;
        m_commentToProperty[commentProperty] = property;
    }
}

void PropertyEditor::setupPaletteProperty(QtVariantProperty *property)
{
    QPalette value = qvariant_cast<QPalette>(property->value());
    QPalette superPalette = QPalette();
    QWidget *currentWidget = qobject_cast<QWidget *>(m_object);
    if (currentWidget) {
        if (currentWidget->isWindow())
            superPalette = QApplication::palette(currentWidget);
        else {
            if (currentWidget->parentWidget())
                superPalette = currentWidget->parentWidget()->palette();
        }
    }
    m_updatingBrowser = true;
    property->setAttribute(QLatin1String("superPalette"), superPalette);
    m_updatingBrowser = false;
}

PropertyEditor::PropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent, Qt::WindowFlags flags)
    : QDesignerPropertyEditor(parent, flags), m_core(core), m_propertySheet(0)
{
    m_stackedWidget = new QStackedWidget(this);

    QToolBar *toolBar = new QToolBar(this);

    QActionGroup *actionGroup = new QActionGroup(this);
    m_treeAction = new QAction(tr("Tree View"), this);
    m_treeAction->setCheckable(true);
    m_treeAction->setIcon(createIconSet(QLatin1String("widgets/listview.png")));
    m_groupBoxAction = new QAction(tr("Group Box View"), this);
    m_groupBoxAction->setCheckable(true);
    m_groupBoxAction->setIcon(createIconSet(QLatin1String("widgets/groupbox.png")));
    actionGroup->addAction(m_treeAction);
    actionGroup->addAction(m_groupBoxAction);
    m_groupBoxAction->setChecked(true);
    connect(actionGroup, SIGNAL(triggered(QAction *)),
                this, SLOT(slotViewTriggered(QAction *)));

    QWidget *classWidget = new QWidget(toolBar);
    classWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    m_classLabel = new QLabel(classWidget);
    QHBoxLayout *l = new QHBoxLayout(classWidget);
    l->setMargin(0);
    l->addWidget(m_classLabel);
    m_classLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));

    m_addDynamicAction = new QAction(tr("Add Dynamic Property..."), this);
    m_addDynamicAction->setIcon(createIconSet(QLatin1String("plus.png")));
    m_addDynamicAction->setEnabled(false);
    connect(m_addDynamicAction, SIGNAL(triggered()), this, SLOT(slotAddDynamicProperty()));
    QToolButton *removeButton = new QToolButton(toolBar);
    m_removeDynamicAction = new QAction(tr("Remove Dynamic Property"), this);
    m_removeDynamicAction->setIcon(createIconSet(QLatin1String("minus.png")));
    m_removeDynamicAction->setEnabled(false);
    m_removeDynamicMenu = new QMenu(this);
    m_removeDynamicAction->setMenu(m_removeDynamicMenu);
    removeButton->setDefaultAction(m_removeDynamicAction);
    removeButton->setPopupMode(QToolButton::InstantPopup);

    m_removeMapper = new QSignalMapper(this);
    connect(m_removeMapper, SIGNAL(mapped(const QString &)), this, SIGNAL(removeDynamicProperty(const QString &)));

    toolBar->addWidget(classWidget);
    toolBar->addAction(m_addDynamicAction);
    //toolBar->addAction(m_removeDynamicAction);
    toolBar->addWidget(removeButton);
    toolBar->addSeparator();
    toolBar->addAction(m_treeAction);
    toolBar->addAction(m_groupBoxAction);

    QScrollArea *scroll = new QScrollArea(m_stackedWidget);
    m_groupBrowser = new QtGroupBoxPropertyBrowser(scroll);
    m_groupBrowser->layout()->setMargin(-1);
    scroll->setWidgetResizable(true);
    scroll->setWidget(m_groupBrowser);
    m_groupBoxIndex = m_stackedWidget->addWidget(scroll);

    m_treeBrowser = new QtTreePropertyBrowser(m_stackedWidget);
    m_treeBrowser->setRootIsDecorated(false);
    m_treeIndex = m_stackedWidget->addWidget(m_treeBrowser);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(toolBar);
    layout->addWidget(m_stackedWidget);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_propertyManager = new DesignerPropertyManager(m_core, this);
    DesignerEditorFactory *treeFactory = new DesignerEditorFactory(m_core, this);
    treeFactory->setSpacing(0);
    DesignerEditorFactory *groupFactory = new DesignerEditorFactory(m_core, this);
    QtVariantPropertyManager *variantManager = m_propertyManager;
    m_groupBrowser->setFactoryForManager(variantManager, groupFactory);
    m_treeBrowser->setFactoryForManager(variantManager, treeFactory);

    m_stackedWidget->setCurrentIndex(m_groupBoxIndex);
    m_currentBrowser = m_groupBrowser;

    connect(groupFactory, SIGNAL(resetProperty(QtProperty *)), this, SLOT(slotResetProperty(QtProperty *)));
    connect(treeFactory, SIGNAL(resetProperty(QtProperty *)), this, SLOT(slotResetProperty(QtProperty *)));
    connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
                this, SLOT(slotValueChanged(QtProperty *, const QVariant &)));

    m_updatingBrowser = false;
}

PropertyEditor::~PropertyEditor()
{

}

void PropertyEditor::slotViewTriggered(QAction *action)
{
    m_currentBrowser->clear();
    int idx = 0;
    if (action == m_treeAction) {
        m_currentBrowser = m_treeBrowser;
        idx = m_treeIndex;
    } else {
        m_currentBrowser = m_groupBrowser;
        idx = m_groupBoxIndex;
    }
    QListIterator<QtProperty *> itGroup(m_groups);
    while (itGroup.hasNext()) {
        QtProperty *group = itGroup.next();
        m_currentBrowser->addProperty(group);
    }
    m_stackedWidget->setCurrentIndex(idx);
}

void PropertyEditor::slotAddDynamicProperty()
{
    if (!m_propertySheet)
        return;

    const QDesignerDynamicPropertySheetExtension *dynamicSheet =
            qt_extension<QDesignerDynamicPropertySheetExtension*>(m_core->extensionManager(), m_object);

    if (!dynamicSheet)
        return;

    NewDynamicPropertyDialog dlg(this);
    QStringList reservedNames;
    for (int i = 0; i < m_propertySheet->count(); i++) {
        if (!dynamicSheet->isDynamicProperty(i) || m_propertySheet->isVisible(i))
            reservedNames.append(m_propertySheet->propertyName(i));
    }
    dlg.setReservedNames(reservedNames);
    if (dlg.exec() == QDialog::Accepted) {
        const QString newName = dlg.propertyName();
        const QVariant newValue = dlg.propertyValue();

        emit addDynamicProperty(newName, newValue);
    }
}

QDesignerFormEditorInterface *PropertyEditor::core() const
{
    return m_core;
}

bool PropertyEditor::isReadOnly() const
{
    return false;
}

void PropertyEditor::setReadOnly(bool readOnly)
{
    qDebug() << "PropertyEditor::setReadOnly() request";
}

void PropertyEditor::setPropertyValue(const QString &name, const QVariant &value, bool changed)
{
    if (!m_nameToProperty.contains(name))
        return;
    QtVariantProperty *property = m_nameToProperty.value(name);
    updateBrowserValue(property, value);
    property->setModified(changed);
}

void PropertyEditor::setPropertyComment(const QString &name, const QString &value)
{
    if (m_nameToProperty.contains(name)) {
        QtVariantProperty *property = m_nameToProperty.value(name);
        if (m_propertyToComment.contains(property)) {
            QtVariantProperty *commentProperty = m_propertyToComment.value(property);
            updateBrowserValue(commentProperty, value);
        }
    }
}

void PropertyEditor::updatePropertySheet()
{
    if (!m_propertySheet)
        return;

    for (int i = 0; i < m_propertySheet->count(); ++i) {
        const QString propertyName = m_propertySheet->propertyName(i);
        if (m_nameToProperty.contains(propertyName)) {
            QtVariantProperty *property = m_nameToProperty.value(propertyName);
            updateBrowserValue(property, m_propertySheet->property(i));
        }
    }
}

void PropertyEditor::updateBrowserValue(QtVariantProperty *property, const QVariant &value)
{
    QVariant v = value;
    if (property->propertyType() == QtVariantPropertyManager::enumTypeId()) {
        const EnumType e = qvariant_cast<EnumType>(v);
        v = e.names.indexOf(e.items.key(e.value));
    } else if (property->propertyType() == DesignerPropertyManager::designerFlagTypeId()) {
        const FlagType f = qvariant_cast<FlagType>(v);
        v = f.value;
    } else if (property->propertyType() == DesignerPropertyManager::designerAlignmentTypeId()) {
        const FlagType f = qvariant_cast<FlagType>(v);
        v = f.value;
    }
    m_updatingBrowser = true;
    property->setValue(v);
    m_updatingBrowser = false;
}

int PropertyEditor::toBrowserType(const QVariant &value, const QString &propertyName) const
{
    if (qVariantCanConvert<EnumType>(value))
        return DesignerPropertyManager::enumTypeId();
    if (qVariantCanConvert<FlagType>(value)) {
        if (propertyName == QLatin1String("alignment"))
            return DesignerPropertyManager::designerAlignmentTypeId();
        return DesignerPropertyManager::designerFlagTypeId();
    }
    return value.userType();
}

QString PropertyEditor::removeScope(const QString &value) const
{
    int pos = value.lastIndexOf(QLatin1String("::"));
    if (pos < 0)
        return value;
    return value.mid(pos + 2);
}

QString PropertyEditor::realClassName(QObject *object) const
{
    if (!object)
        return 0;

    const QString qLayoutWidget = QLatin1String("QLayoutWidget");
    const QString designerPrefix = QLatin1String("QDesigner");

    QString className = QLatin1String(object->metaObject()->className());
    const QDesignerWidgetDataBaseInterface *db = core()->widgetDataBase();
    if (QDesignerWidgetDataBaseItemInterface *widgetItem = db->item(db->indexOfObject(object, true))) {
        className = widgetItem->name();

        if (object->isWidgetType() && className == qLayoutWidget
                && static_cast<QWidget*>(object)->layout()) {
            className = QLatin1String(static_cast<QWidget*>(object)->layout()->metaObject()->className());
        }

        //item->setIcon(0, widgetItem->icon());
    }

    if (className.startsWith(designerPrefix))
        className.remove(1, designerPrefix.size() - 1);

    return className;
}

void PropertyEditor::setObject(QObject *object)
{
    m_object = object;
    m_removeDynamicMenu->clear();

    QString objectName;
    QString className;
    if (m_object) {
        objectName = m_object->objectName();
        className = realClassName(m_object);
    }

    m_classLabel->setText(tr("%1\n%2").arg(objectName).arg(className));
    m_classLabel->setToolTip(tr("Object: %1\nClass: %2").arg(objectName).arg(className));

    QMap<QString, QtVariantProperty *> toRemove = m_nameToProperty;

    const QDesignerDynamicPropertySheetExtension *dynamicSheet =
            qt_extension<QDesignerDynamicPropertySheetExtension*>(m_core->extensionManager(), m_object);

    // list of properties to remove
    // remove them
    // traverse the sheet, in case property exists just set a value, otherwise - create it.

    QExtensionManager *m = m_core->extensionManager();

    m_propertySheet = qobject_cast<QDesignerPropertySheetExtension*>(m->extension(object, Q_TYPEID(QDesignerPropertySheetExtension)));
    if (m_propertySheet) {
        for (int i = 0; i < m_propertySheet->count(); ++i) {
            if (!m_propertySheet->isVisible(i))
                continue;

            const QString propertyName = m_propertySheet->propertyName(i);
            if (m_propertySheet->indexOf(propertyName) != i)
                continue;
            const QString groupName = m_propertySheet->propertyGroup(i);
            if (toRemove.contains(propertyName)) {
                QtVariantProperty *property = toRemove.value(propertyName);
                if (m_propertyToGroup.value(property) == groupName && toBrowserType(m_propertySheet->property(i), propertyName) == property->propertyType())
                    toRemove.remove(propertyName);
            }
        }
    }

    QMapIterator<QString, QtVariantProperty *> itRemove(toRemove);
    while (itRemove.hasNext()) {
        itRemove.next();

        QtVariantProperty *property = itRemove.value();
        delete property;
        if (m_propertyToComment.contains(property)) {
            QtVariantProperty *commentProperty = m_propertyToComment.value(property);
            delete commentProperty;
            m_commentToProperty.remove(commentProperty);
            m_propertyToComment.remove(property);
        }
        m_nameToProperty.remove(itRemove.key());
        m_propertyToGroup.remove(property);
    }

    bool isMainContainer = false;
    if (QWidget *widget = qobject_cast<QWidget*>(object)) {
        if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(widget)) {
            isMainContainer = (fw->mainContainer() == widget);
        }
    }

    QStringList dynamicProperties;
    m_groups.clear();

    if (m_propertySheet) {
        QtProperty *lastProperty = 0;
        QtProperty *lastGroup = 0;
        for (int i = 0; i < m_propertySheet->count(); ++i) {
            if (!m_propertySheet->isVisible(i))
                continue;

            const QString propertyName = m_propertySheet->propertyName(i);
            if (m_propertySheet->indexOf(propertyName) != i)
                continue;
            const QVariant value = m_propertySheet->property(i);

            int type = toBrowserType(value, propertyName);

            /*
               switch (value.type()) {
               case QVariant::UInt:
               p = new UIntProperty(value.toUInt(), pname);
               break;
               case QVariant::LongLong:
               p = new LongLongProperty(value.toLongLong(), pname);
               break;
               case QVariant::ULongLong:
               p = new ULongLongProperty(value.toULongLong(), pname);
               break;
               case QVariant::Char:
               p = new CharProperty(value.toChar(), pname);
               break;
               case QVariant::ByteArray:
               case QVariant::String: 
               p = createStringProperty(object, pname, value, isMainContainer);
               break;
            break;
            case QVariant::Url:
            p = new UrlProperty(value.toUrl(), pname);
            break;
            case QVariant::StringList:
            p = new StringListProperty(qvariant_cast<QStringList>(value), pname);
            break;
            default:
            // ### qDebug() << "property" << pname << "with type" << value.type() << "not supported yet!";
            break;
        } // end switch
        */
            QtVariantProperty *property = 0;
            bool newProperty = false;
            if (m_nameToProperty.contains(propertyName)) {
                property = m_nameToProperty.value(propertyName);
            } else {
                property = m_propertyManager->addProperty(type, propertyName);
                newProperty = true;
                if (property && type == DesignerPropertyManager::enumTypeId()) {
                    const EnumType e = qvariant_cast<EnumType>(value);
                    QStringList names;
                    QStringListIterator it(e.names);
                    while (it.hasNext())
                        names.append(removeScope(it.next()));
                    m_updatingBrowser = true;
                    property->setAttribute(QLatin1String("enumNames"), names);
                    m_updatingBrowser = false;
                } else if (property && type == DesignerPropertyManager::designerFlagTypeId()) {
                    const FlagType f = qvariant_cast<FlagType>(value);
                    QList<QPair<QString, uint> > flags;
                    QStringListIterator it(f.names);
                    while (it.hasNext()) {
                        const QString name = it.next();
                        const uint val = f.items.value(name).toUInt();
                        flags.append(qMakePair(removeScope(name), val));
                    }
                    m_updatingBrowser = true;
                    QVariant v;
                    qVariantSetValue(v, flags);
                    property->setAttribute(QLatin1String("flags"), v);
                    m_updatingBrowser = false;
                }
            }

            if (property != 0) {
                if (dynamicSheet && dynamicSheet->isDynamicProperty(i))
                    dynamicProperties.append(propertyName);

                if (type == QVariant::String)
                    setupStringProperty(property, propertyName, value, isMainContainer);

                if (type == QVariant::Palette)
                    setupPaletteProperty(property);

                updateBrowserValue(property, value);
                if (m_propertyToComment.contains(property)) {
                    updateBrowserValue(m_propertyToComment.value(property), propertyComment(m_core, m_object, propertyName));
                }
                property->setAttribute(QLatin1String("resetable"), m_propertySheet->hasReset(i));
                property->setModified(m_propertySheet->isChanged(i));

                const QString groupName = m_propertySheet->propertyGroup(i);
                QtVariantProperty *groupProperty = 0;
                if (m_nameToGroup.contains(groupName)) {
                    groupProperty = m_nameToGroup.value(groupName);
                } else {
                    groupProperty = m_propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), groupName);
                    m_currentBrowser->insertProperty(groupProperty, lastGroup);
                    m_nameToGroup[groupName] = groupProperty;
                }

                if (lastGroup != groupProperty) {
                    lastProperty = 0;
                    lastGroup = groupProperty;
                }
                if (!m_groups.contains(groupProperty))
                    m_groups.append(groupProperty);

                if (newProperty) {
                    groupProperty->insertSubProperty(property, lastProperty);
                    m_nameToProperty[propertyName] = property;
                    m_propertyToGroup[property] = groupName;
                }
                lastProperty = property;
            } else {
                qDebug() << "Property" << propertyName << "of type" << type << "not supported yet!";
            }
        }
    }
    QMap<QString, QtVariantProperty *> groups = m_nameToGroup;
    QMapIterator<QString, QtVariantProperty *> itGroup(groups);
    while (itGroup.hasNext()) {
        QtVariantProperty *groupProperty = itGroup.next().value();
        if (groupProperty->subProperties().count() == 0) {
            delete groupProperty;
            m_nameToGroup.remove(itGroup.key());
        }
    }
    const bool addEnabled = dynamicSheet ? dynamicSheet->dynamicPropertiesAllowed() : false;
    const bool removeEnabled = addEnabled && dynamicProperties.count();
    m_addDynamicAction->setEnabled(addEnabled);
    m_removeDynamicAction->setEnabled(removeEnabled);
    QStringListIterator it(dynamicProperties);
    while (it.hasNext()) {
        QString property = it.next();
        QAction *action = new QAction(property, m_removeDynamicMenu);
        connect(action, SIGNAL(triggered()), m_removeMapper, SLOT(map()));
        m_removeMapper->setMapping(action, property);
        m_removeDynamicMenu->addAction(action);
    }
}

QString PropertyEditor::currentPropertyName() const
{
    qDebug() << "PropertyEditor::currentPropertyName() request";
    return QString();
}

void PropertyEditor::slotResetProperty(QtProperty *property)
{
    QDesignerFormWindowInterface *form = m_core->formWindowManager()->activeFormWindow();
    if (!form)
        return;

    if (m_propertyManager->resetFontSubProperty(property))
        return;

    if (!m_propertyToGroup.contains(property))
        return;

    emit resetProperty(property->propertyName());
}

void PropertyEditor::slotValueChanged(QtProperty *property, const QVariant &value)
{
    if (m_updatingBrowser)
        return;

    if (!m_propertySheet)
        return;

    QtVariantProperty *varProp = m_propertyManager->variantProperty(property);

    if (!varProp)
        return;

    if (m_commentToProperty.contains(varProp)) {
        QtVariantProperty *commentParentProperty = m_commentToProperty.value(varProp);
        emit propertyCommentChanged(commentParentProperty->propertyName(), value.toString());
        return;
    }

    if (!m_propertyToGroup.contains(property))
        return;

    if (varProp->propertyType() == QtVariantPropertyManager::enumTypeId()) {
        EnumType e = qvariant_cast<EnumType>(m_propertySheet->property(m_propertySheet->indexOf(property->propertyName())));
        QMapIterator<QString, QVariant> it(e.items);
        const int val = value.toInt();
        const QString valName = varProp->attributeValue(QLatin1String("enumNames")).toStringList().at(val);
        while (it.hasNext()) {
            QString enumName = it.next().key();
            if (removeScope(enumName) == valName) {
                const QVariant newValue = it.value();
                e.value = newValue;
                QVariant v;
                qVariantSetValue(v, e);
                emit propertyChanged(property->propertyName(), v);
                return;
            }
        }
        return;
    }

    if (varProp->propertyType() == QVariant::Palette) {
        QPalette pal = qvariant_cast<QPalette>(value);
        if (!pal.resolve()) {
            emit resetProperty(property->propertyName());
            return;
        }
    }

    if (varProp->propertyType() == QVariant::Font) {
        QFont font = qvariant_cast<QFont>(value);
        if (!font.resolve()) {
            emit resetProperty(property->propertyName());
            return;
        }
    }

    emit propertyChanged(property->propertyName(), value);
}

}
