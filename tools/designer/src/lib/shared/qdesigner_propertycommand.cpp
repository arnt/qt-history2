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

#include "qdesigner_propertycommand_p.h"
#include "qdesigner_utils_p.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QSize>
#include <QtCore/QTextStream>
#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include <QtGui/QAction>
#include <qdebug.h>

namespace  {
const bool debugPropertyCommands = false;


QSize checkSize(const QSize &size)
{
    return size.boundedTo(QSize(0xFFFFFF, 0xFFFFFF));
}

QSize diffSize(QDesignerFormWindowInterface *fw)
{
    const QWidget *container = fw->core()->integration()->containerWindow(fw);
    if (!container)
        return QSize();

    const QSize diff = container->size() - fw->size(); // decoration offset of container window
    return diff;
}

void checkSizes(QDesignerFormWindowInterface *fw, const QSize &size, QSize *formSize, QSize *containerSize)
{
    const QWidget *container = fw->core()->integration()->containerWindow(fw);
    if (!container)
        return;

    const  QSize diff = diffSize(fw); // decoration offset of container window

    QSize newFormSize = checkSize(size).expandedTo(fw->mainContainer()->minimumSizeHint()); // don't try to resize to smaller size than minimumSizeHint
    QSize newContainerSize = newFormSize + diff;

    newContainerSize = newContainerSize.expandedTo(container->minimumSizeHint());
    newContainerSize = newContainerSize.expandedTo(container->minimumSize());

    newFormSize = newContainerSize - diff;

    if (formSize)
        *formSize = newFormSize;
    if (containerSize)
        *containerSize = newContainerSize;
}

/* SubProperties: When applying a changed property to a multiselection, it sometimes makes
 * sense to apply only parts (subproperties) of the property.
 * For example, if someone changes the x-value of a geometry in the property editor
 * and applies it to a multi-selection, y should not be applied as this would cause all
 * the widgets to overlap.
 * The following routines can be used to find out the changed subproperties of a property,
 * which are represented as a mask, and to apply them while leaving the others intact. */

enum RectSubPropertyMask {  SubPropertyX=1, SubPropertyY = 2, SubPropertyWidth = 4, SubPropertyHeight = 8 };
enum SizePolicySubPropertyMask { SubPropertyHSizePolicy = 1, SubPropertyHStretch = 2, SubPropertyVSizePolicy =4, SubPropertyVStretch = 8 };
enum FontSubPropertyMask { SubPropertyFamily=1, SubPropertyPointSize=2, SubPropertyBold=4,  SubPropertyItalic=8,
                           SubPropertyUnderline=16, SubPropertyStrikeOut=32, SubPropertyKerning=64, SubPropertyStyleStrategy=128};
enum AlignmentSubPropertyMask { SubPropertyHorizontalAlignment=1, SubPropertyVerticalAlignment=2 };

enum CommonSubPropertyMask { SubPropertyAll = 0xFFFFFFFF };

// Set the mask flag in mask if the properties do not match.
#define COMPARE_SUBPROPERTY(object1, object2, getter, mask, maskFlag) if (object1.getter() != object2.getter()) mask |= maskFlag;

// find changed subproperties of a rectangle
unsigned compareSubProperties(const QRect & r1, const QRect & r2)
{
    unsigned rc = 0;
    COMPARE_SUBPROPERTY(r1, r2, x, rc, SubPropertyX)
    COMPARE_SUBPROPERTY(r1, r2, y, rc, SubPropertyY)
    COMPARE_SUBPROPERTY(r1, r2, width, rc, SubPropertyWidth)
    COMPARE_SUBPROPERTY(r1, r2, height, rc, SubPropertyHeight)
    return rc;
}

// find changed subproperties of a QSize
unsigned compareSubProperties(const QSize & r1, const QSize & r2)
{
    unsigned rc = 0;
    COMPARE_SUBPROPERTY(r1, r2, width,  rc, SubPropertyWidth)
    COMPARE_SUBPROPERTY(r1, r2, height, rc, SubPropertyHeight)
    return rc;
}
// find changed subproperties of a QSizePolicy
unsigned compareSubProperties(const QSizePolicy & sp1, const QSizePolicy & sp2)
{
    unsigned rc = 0;
    COMPARE_SUBPROPERTY(sp1, sp2, horizontalPolicy,  rc, SubPropertyHSizePolicy)
    COMPARE_SUBPROPERTY(sp1, sp2, horizontalStretch, rc, SubPropertyHStretch)
    COMPARE_SUBPROPERTY(sp1, sp2, verticalPolicy,    rc, SubPropertyVSizePolicy)
    COMPARE_SUBPROPERTY(sp1, sp2, verticalStretch,   rc, SubPropertyVStretch)
    return rc;
}

// find changed subproperties of a QFont
unsigned compareSubProperties(const QFont & f1, const QFont & f2)
{
    unsigned rc = 0;
    COMPARE_SUBPROPERTY(f1, f2, family,        rc, SubPropertyFamily)
    COMPARE_SUBPROPERTY(f1, f2, pointSize,     rc, SubPropertyPointSize)
    COMPARE_SUBPROPERTY(f1, f2, bold,          rc, SubPropertyBold)
    COMPARE_SUBPROPERTY(f1, f2, italic,        rc, SubPropertyItalic)
    COMPARE_SUBPROPERTY(f1, f2, underline,     rc, SubPropertyUnderline)
    COMPARE_SUBPROPERTY(f1, f2, strikeOut,     rc, SubPropertyStrikeOut)
    COMPARE_SUBPROPERTY(f1, f2, kerning,       rc, SubPropertyKerning)
    COMPARE_SUBPROPERTY(f1, f2, styleStrategy, rc, SubPropertyStyleStrategy)
    return rc;
}

// Compare colors of a role
bool roleColorChanged(const QPalette & p1, const QPalette & p2, QPalette::ColorRole role)
{
    for (int group = QPalette::Active; group < QPalette::NColorGroups;  group++) {
        const QPalette::ColorGroup pgroup = static_cast<QPalette::ColorGroup>(group);
        if (p1.color(pgroup, role) != p2.color(pgroup, role))
            return true;
    }
    return false;
}
// find changed subproperties of a QPalette
unsigned compareSubProperties(const QPalette & p1, const QPalette & p2)
{
    unsigned rc = 0;
    unsigned maskBit = 1u;
    // generate a mask for each role
    for (int role = QPalette::WindowText;  role < QPalette::NColorRoles; role++, maskBit <<= 1u) {
        if (roleColorChanged(p1, p2, static_cast<QPalette::ColorRole>(role)))
            rc |= maskBit;
    }
    return rc;
}

// find changed subproperties of a QAlignment which is a flag combination of vertical and horizontal

unsigned compareSubProperties(Qt::Alignment a1, Qt::Alignment a2)
{
    unsigned rc = 0;
    if ((a1 & Qt::AlignHorizontal_Mask) != (a2 & Qt::AlignHorizontal_Mask))
        rc |= SubPropertyHorizontalAlignment;
    if ((a1 & Qt::AlignVertical_Mask) != (a2 & Qt::AlignVertical_Mask))
        rc |= SubPropertyVerticalAlignment;
    return rc;
}

Qt::Alignment variantToAlignment(const QVariant & q)
{
    return static_cast<Qt::Alignment>(qdesigner_internal::Utils::valueOf(q));
}
// find changed subproperties of a variant
unsigned compareSubProperties(const QVariant & q1, const QVariant & q2, qdesigner_internal::SpecialProperty specialProperty)
{
    switch (q1.type()) {
    case QVariant::Rect:
        return compareSubProperties(q1.toRect(), q2.toRect());
    case QVariant::Size:
        return compareSubProperties(q1.toSize(), q2.toSize());
    case QVariant::SizePolicy:
        return compareSubProperties(qvariant_cast<QSizePolicy>(q1), qvariant_cast<QSizePolicy>(q2));
    case QVariant::Font:
        return compareSubProperties(qvariant_cast<QFont>(q1), qvariant_cast<QFont>(q2));
    case QVariant::Palette:
        return compareSubProperties(qvariant_cast<QPalette>(q1), qvariant_cast<QPalette>(q2));
    default:
        // Enumerations, flags
        switch (specialProperty) {
        case qdesigner_internal::SP_Alignment:
            return compareSubProperties(variantToAlignment(q1), variantToAlignment(q2));
        default:
        break;
        }
        break;
    }
    return SubPropertyAll;
}

// Apply  the sub property if mask flag is set in mask
#define SET_SUBPROPERTY(rc, newValue, getter, setter, mask, maskFlag) if (mask & maskFlag) rc.setter(newValue.getter());

// apply changed subproperties to a rectangle
QRect applyRectSubProperty(const QRect &oldValue, const QRect &newValue, unsigned mask)
{
    QRect rc = oldValue;
    SET_SUBPROPERTY(rc, newValue, x,      setX,      mask, SubPropertyX)
    SET_SUBPROPERTY(rc, newValue, y,      setY,      mask, SubPropertyY)
    SET_SUBPROPERTY(rc, newValue, width,  setWidth,  mask, SubPropertyWidth)
    SET_SUBPROPERTY(rc, newValue, height, setHeight, mask, SubPropertyHeight)
    return rc;
}


// apply changed subproperties to a rectangle QSize
QSize applySizeSubProperty(const QSize &oldValue, const QSize &newValue, unsigned mask)
{
    QSize rc = oldValue;
    SET_SUBPROPERTY(rc, newValue, width,  setWidth,  mask, SubPropertyWidth)
    SET_SUBPROPERTY(rc, newValue, height, setHeight, mask, SubPropertyHeight)
    return rc;
}


// apply changed subproperties to a SizePolicy
QSizePolicy applySizePolicySubProperty(const QSizePolicy &oldValue, const QSizePolicy &newValue, unsigned mask)
{
    QSizePolicy rc = oldValue;
    SET_SUBPROPERTY(rc, newValue, horizontalPolicy,  setHorizontalPolicy,  mask, SubPropertyHSizePolicy)
    SET_SUBPROPERTY(rc, newValue, horizontalStretch, setHorizontalStretch, mask, SubPropertyHStretch)
    SET_SUBPROPERTY(rc, newValue, verticalPolicy,    setVerticalPolicy,    mask, SubPropertyVSizePolicy)
    SET_SUBPROPERTY(rc, newValue, verticalStretch,   setVerticalStretch,   mask, SubPropertyVStretch)
    return rc;
}

// apply changed subproperties to a QFont
QFont applyFontSubProperty(const QFont &oldValue, const QFont &newValue, unsigned mask)
{
    QFont  rc = oldValue;
    SET_SUBPROPERTY(rc, newValue, family,        setFamily,        mask, SubPropertyFamily)
    SET_SUBPROPERTY(rc, newValue, pointSize,     setPointSize,     mask, SubPropertyPointSize)
    SET_SUBPROPERTY(rc, newValue, bold,          setBold,          mask, SubPropertyBold)
    SET_SUBPROPERTY(rc, newValue, italic,        setItalic,        mask, SubPropertyItalic)
    SET_SUBPROPERTY(rc, newValue, underline,     setUnderline,     mask, SubPropertyUnderline)
    SET_SUBPROPERTY(rc, newValue, strikeOut,     setStrikeOut,     mask, SubPropertyStrikeOut)
    SET_SUBPROPERTY(rc, newValue, kerning,       setKerning,       mask, SubPropertyKerning)
    SET_SUBPROPERTY(rc, newValue, styleStrategy, setStyleStrategy, mask, SubPropertyStyleStrategy)
    return rc;
}

// apply changed subproperties to a QPalette
QPalette applyPaletteSubProperty(const QPalette &oldValue, const QPalette &newValue, unsigned mask)
{
    QPalette rc = oldValue;
    // apply a mask for each role
    unsigned maskBit = 1u;
    for (int role = QPalette::WindowText;  role < QPalette::NColorRoles; role++, maskBit <<= 1u) {
        if (mask & maskBit) {
            for (int group = QPalette::Active; group < QPalette::NColorGroups;  group++) {
                const QPalette::ColorGroup pgroup = static_cast<QPalette::ColorGroup>(group);
                const QPalette::ColorRole prole =  static_cast<QPalette::ColorRole>(role);
                rc.setColor(pgroup, prole, newValue.color(pgroup, prole));
            }
        }
    }
    return rc;
}

// apply changed subproperties to  a QAlignment which is a flag combination of vertical and horizontal
Qt::Alignment applyAlignmentSubProperty(Qt::Alignment oldValue, Qt::Alignment newValue, unsigned mask)
{
    // easy: both changed.
    if (mask == (SubPropertyHorizontalAlignment|SubPropertyVerticalAlignment))
        return newValue;
    // Change subprop
    const Qt::Alignment changeMask   = (mask & SubPropertyHorizontalAlignment) ? Qt::AlignHorizontal_Mask : Qt::AlignVertical_Mask;
    const Qt::Alignment takeOverMask = (mask & SubPropertyHorizontalAlignment) ? Qt::AlignVertical_Mask   : Qt::AlignHorizontal_Mask;
    return (oldValue & takeOverMask) | (newValue & changeMask);
}

// apply changed subproperties to a variant
QVariant applySubProperty(const QVariant &oldValue, const QVariant &newValue, qdesigner_internal::SpecialProperty specialProperty, unsigned mask)
{
    if (mask == SubPropertyAll)
        return newValue;

    switch (oldValue.type()) {
    case QVariant::Rect:
        return applyRectSubProperty(oldValue.toRect(), newValue.toRect(), mask);
    case QVariant::Size:
        return applySizeSubProperty(oldValue.toSize(), newValue.toSize(), mask);
    case QVariant::SizePolicy:
        return qVariantFromValue(applySizePolicySubProperty(qvariant_cast<QSizePolicy>(oldValue), qvariant_cast<QSizePolicy>(newValue), mask));
    case QVariant::Font:
        return qVariantFromValue(applyFontSubProperty(qvariant_cast<QFont>(oldValue), qvariant_cast<QFont>(newValue), mask));
    case QVariant::Palette:
        return qVariantFromValue(applyPaletteSubProperty(qvariant_cast<QPalette>(oldValue), qvariant_cast<QPalette>(newValue), mask));
    default:
        // Enumerations, flags
        switch (specialProperty) {
        case qdesigner_internal::SP_Alignment:
            return QVariant(static_cast<uint>(applyAlignmentSubProperty(variantToAlignment(oldValue),
                                                                        variantToAlignment(newValue),
                                                                        mask)));
        default:
        break;
        }
        break;
    }
    return newValue;

}
}

namespace qdesigner_internal {

// figure out special property
enum SpecialProperty getSpecialProperty(const QString& propertyName)
{
    if (propertyName == QLatin1String("objectName"))
        return SP_ObjectName;
    if (propertyName == QLatin1String("icon"))
        return SP_Icon;
    if (propertyName == QLatin1String("currentTabName"))
        return SP_CurrentTabName;
    if (propertyName == QLatin1String("geometry"))
        return SP_Geometry;
    if (propertyName == QLatin1String("windowTitle"))
        return SP_WindowTitle;
    if (propertyName == QLatin1String("minimumSize"))
        return SP_MinimumSize;
    if (propertyName == QLatin1String("maximumSize"))
        return SP_MaximumSize;
    if (propertyName == QLatin1String("alignment"))
        return SP_Alignment;
    return SP_None;
}


PropertyHelper::PropertyHelper(QObject* object,
                               SpecialProperty specialProperty,
                               QDesignerPropertySheetExtension *sheet,
                               int index) :
    m_specialProperty(specialProperty),
    m_object(object),
    m_objectType(OT_Object),
    m_propertySheet(sheet),  m_index(index),
    m_oldValue(m_propertySheet->property(m_index), m_propertySheet->isChanged(m_index))
{
    if (object->isWidgetType()) {
        m_parentWidget = (qobject_cast<QWidget*>(object))->parentWidget();
        m_objectType = OT_Widget;
    } else {
        if (qobject_cast<const QAction *>(m_object))
            m_objectType = OT_Action;
    }

    if(debugPropertyCommands)
        qDebug() << "PropertyHelper on " << m_object->objectName() << " index= " << m_index << " type = " << m_objectType;
}

// Set widget value, apply corrections and checks in case of main window.
void PropertyHelper::checkApplyWidgetValue(QDesignerFormWindowInterface *fw, QWidget* w,
                                      SpecialProperty specialProperty, QVariant &value)
{

    bool isMainContainer = false;
    if (QDesignerFormWindowCursorInterface *cursor = fw->cursor()) {
        if (cursor->isWidgetSelected(w)) {
            if (cursor->isWidgetSelected(fw->mainContainer())) {
                isMainContainer = true;
            }
        }
    }
    if (!isMainContainer)
        return;

    QWidget *container = fw->core()->integration()->containerWindow(fw);
    if (!container)
        return;


    switch (specialProperty) {
    case SP_MinimumSize: {
        const QSize diff = diffSize(fw);
        const QSize size = checkSize(value.toSize());
        container->setMinimumSize((size + diff).expandedTo(QSize(16, 16)));
        qVariantSetValue(value, size);
    }

        break;
    case SP_MaximumSize: {
        QSize fs, cs;
        checkSizes(fw, value.toSize(), &fs, &cs);
        container->setMaximumSize(cs);
        fw->mainContainer()->setMaximumSize(fs);
        qVariantSetValue(value, fs);

    }
        break;
    case SP_Geometry: {
        QRect r = value.toRect();
        QSize fs, cs;
        checkSizes(fw, r.size(), &fs, &cs);
        container->resize(cs);
        r.setSize(fs);
        qVariantSetValue(value, r);
    }
        break;
    default:
        break;
    }
}

unsigned PropertyHelper::updateMask() const
{
    unsigned rc = 0;
    switch (m_specialProperty) {
    case SP_ObjectName:
    case SP_CurrentTabName:
         rc |=  UpdateObjectInspector;
        break;
    case  SP_Icon:
        if (m_objectType == OT_Action)
            rc |=  UpdateObjectInspector;
        break;
    default:
        break;

    }
    return rc;
}


bool PropertyHelper::canMerge(const PropertyHelper &other) const
{
    return m_object == other.m_object &&  m_index == other.m_index;
}

// Update the object to reflect the changes
void PropertyHelper::updateObject(QDesignerFormWindowInterface *fw, const QVariant &oldValue, const QVariant &newValue)
{
    if(debugPropertyCommands){
         qDebug() << "PropertyHelper::updateObject(" << m_object->objectName() << ") " << oldValue << " -> " << newValue;
    }
    switch (m_objectType) {
    case OT_Widget: {
        QWidget *widget = qobject_cast<QWidget *>(m_object);
        switch (m_specialProperty) {
        case SP_Geometry:
            QDesignerFormWindowCommand::checkParent(widget, m_parentWidget);
            break;
        case SP_ObjectName:
            QDesignerFormWindowCommand::updateBuddies(fw, oldValue.toString(), newValue.toString());
            break;
        default:
            break;
        }
    } break;
    case OT_Action:
        if (m_specialProperty == SP_ObjectName) {
            QAction *act = qobject_cast<QAction *>(m_object);
            act->setData(QVariant(true)); // this triggers signal "changed" in QAction
            act->setData(QVariant(false));
        }
        break;
    default:
        break;
    }

}

PropertyHelper::Value PropertyHelper::setValue(QDesignerFormWindowInterface *fw, const QVariant &value, bool changed, unsigned subPropertyMask)
{
    // Set new whole value
    if (subPropertyMask == SubPropertyAll)
        return  applyValue(fw, m_oldValue.first, Value(value, changed));

    // apply subproperties
    const QVariant maskedNewValue = applySubProperty(m_oldValue.first, value, m_specialProperty, subPropertyMask);
    return applyValue(fw, m_oldValue.first, Value(maskedNewValue, changed));
}

// Apply the value and update. Returns corrected value
PropertyHelper::Value PropertyHelper::applyValue(QDesignerFormWindowInterface *fw, const QVariant &oldValue, Value newValue)
{
     if(debugPropertyCommands){
         qDebug() << "PropertyHelper::applyValue(" << m_object->objectName() << ") " << oldValue << " -> " << newValue.first << " changed=" << newValue.second;
     }

    if (m_objectType ==  OT_Widget) {
        checkApplyWidgetValue(fw, qobject_cast<QWidget *>(m_object), m_specialProperty, newValue.first);
    }

    m_propertySheet->setProperty(m_index, newValue.first);
    m_propertySheet->setChanged(m_index, newValue.second);

    updateObject(fw, oldValue, newValue.first);
    return newValue;
}

PropertyHelper::Value PropertyHelper::restoreOldValue(QDesignerFormWindowInterface *fw)
{
    return applyValue(fw, m_propertySheet->property(m_index), m_oldValue);
}


// find the default value in widget DB in case PropertySheet::reset fails
QVariant PropertyHelper::findDefaultValue(QDesignerFormWindowInterface *fw) const
{

    const int item_idx = fw->core()->widgetDataBase()->indexOfObject(m_object);
    if (item_idx == -1)
        return  m_oldValue.first; // We simply don't know the value in this case
    const QDesignerWidgetDataBaseItemInterface *item = fw->core()->widgetDataBase()->item(item_idx);
    const QList<QVariant> default_prop_values = item->defaultPropertyValues();
    if (m_index < default_prop_values.size())
        return default_prop_values.at(m_index);

    return m_oldValue.first; // Again, we just don't know
}

PropertyHelper::Value PropertyHelper::restoreDefaultValue(QDesignerFormWindowInterface *fw)
{

    Value defaultValue(QVariant(), false);
    const QVariant currentValue = m_propertySheet->property(m_index);
    // try to reset sheet, else try to find default
    if (m_propertySheet->reset(m_index)) {
        defaultValue.first = m_propertySheet->property(m_index);
    } else {
        defaultValue.first = findDefaultValue(fw);
        m_propertySheet->setProperty(m_index, defaultValue.first);
    }

    m_propertySheet->setChanged(m_index, defaultValue.second);

    if (m_objectType == OT_Widget) {
        checkApplyWidgetValue(fw, qobject_cast<QWidget *>(m_object), m_specialProperty, defaultValue.first);
    }

    updateObject(fw, currentValue, defaultValue.first);
    return defaultValue;
}

// ---- PropertyListCommand::PropertyDescription(


PropertyListCommand::PropertyDescription::PropertyDescription(const QString &propertyName,
                                                              QDesignerPropertySheetExtension *propertySheet,
                                                              int index) :
    m_propertyName(propertyName),
    m_propertyGroup(propertySheet->propertyGroup(index)),
    m_propertyType(propertySheet->property(index).type()),
    m_specialProperty(getSpecialProperty(propertyName))
{
}

PropertyListCommand::PropertyDescription::PropertyDescription() :
    m_propertyType(QVariant::Invalid),
    m_specialProperty(SP_None)
{
}

void PropertyListCommand::PropertyDescription::debug() const
{
    qDebug() << m_propertyName << m_propertyGroup << m_propertyType << m_specialProperty;
}

bool PropertyListCommand::PropertyDescription::equals(const PropertyDescription &p) const
{
    return m_propertyType == p.m_propertyType && m_specialProperty == p.m_specialProperty &&
           m_propertyName == p.m_propertyName && m_propertyGroup   == p.m_propertyGroup;
}


// ---- PropertyListCommand
PropertyListCommand::PropertyListCommand(QDesignerFormWindowInterface *formWindow) :
    QDesignerFormWindowCommand(QString(), formWindow)
{
}

const QString PropertyListCommand::propertyName() const
{
    return m_propertyDescription.m_propertyName;
}

SpecialProperty PropertyListCommand::specialProperty() const
{
    return m_propertyDescription.m_specialProperty;
}

// add an object
bool PropertyListCommand::add(QObject *object, const QString &propertyName)
{
    QDesignerPropertySheetExtension* sheet = propertySheet(object);
    Q_ASSERT(sheet);

    const int index = sheet->indexOf(propertyName);
    if (index == -1)
        return false;

    const PropertyDescription description(propertyName, sheet, index);

    if (m_propertyHelperList.empty()) {
        // first entry
        m_propertyDescription = description;
    } else {
        // checks: mismatch or only one object in case of name
        const bool match = m_propertyDescription.equals(description);
        if (!match || m_propertyDescription.m_specialProperty == SP_ObjectName)
            return false;
    }
    m_propertyHelperList.push_back(PropertyHelper(object, m_propertyDescription.m_specialProperty, sheet, index));
    return true;
}


// Init from a list and make sure referenceObject is added first to obtain the right property group
bool PropertyListCommand::initList(const ObjectList &list, const QString &apropertyName, QObject *referenceObject)
{
    propertyHelperList().clear();

    // Ensure the referenceObject (property editor) is first, so the right property group is chosen.
    if (referenceObject) {
        if (!add(referenceObject, apropertyName))
            return false;
    }
    foreach (QObject *o, list) {
        if (o != referenceObject)
            add(o, apropertyName);
    }

    return !propertyHelperList().empty();
}


QObject* PropertyListCommand::object(int index) const
{
    Q_ASSERT(index < m_propertyHelperList.size());
    return m_propertyHelperList[index].object();
}

QVariant PropertyListCommand::oldValue(int index) const
{
    Q_ASSERT(index < m_propertyHelperList.size());
    return m_propertyHelperList[index].oldValue();
}

void PropertyListCommand::setOldValue(const QVariant &oldValue, int index)
{
    Q_ASSERT(index < m_propertyHelperList.size());
    m_propertyHelperList[index].setOldValue(oldValue);
}
// ----- SetValueFunction: Set a new value when applied to a PropertyHelper.
class SetValueFunction {
public:
    SetValueFunction(QDesignerFormWindowInterface *formWindow, const PropertyHelper::Value &newValue, unsigned subPropertyMask);

    PropertyHelper::Value operator()(PropertyHelper&);
private:
    QDesignerFormWindowInterface *m_formWindow;
    const PropertyHelper::Value m_newValue;
    const unsigned m_subPropertyMask;
};


SetValueFunction::SetValueFunction(QDesignerFormWindowInterface *formWindow, const PropertyHelper::Value &newValue, unsigned subPropertyMask) :
    m_formWindow(formWindow),
    m_newValue(newValue),
    m_subPropertyMask(subPropertyMask)
{
}

PropertyHelper::Value SetValueFunction::operator()(PropertyHelper &ph) {
        return ph.setValue(m_formWindow, m_newValue.first, m_newValue.second, m_subPropertyMask);
}

// ----- UndoSetValueFunction: Restore old value when applied to a PropertyHelper.
class UndoSetValueFunction {
public:
    UndoSetValueFunction(QDesignerFormWindowInterface *formWindow) : m_formWindow(formWindow) {}
    PropertyHelper::Value operator()(PropertyHelper& ph) { return ph.restoreOldValue(m_formWindow); }
private:
    QDesignerFormWindowInterface *m_formWindow;
};

// ----- RestoreDefaultFunction: Restore default value when applied to a PropertyHelper.
class RestoreDefaultFunction {
public:
    RestoreDefaultFunction(QDesignerFormWindowInterface *formWindow) : m_formWindow(formWindow) {}
    PropertyHelper::Value operator()(PropertyHelper& ph) { return ph.restoreDefaultValue(m_formWindow); }
private:
    QDesignerFormWindowInterface *m_formWindow;
};

// ----- changePropertyList: Iterates over a sequence of PropertyHelpers and
// applies a function to them.
// The function returns the  corrected value which is then set in  the property editor.
// Returns a combination of update flags.
template <class PropertyListIterator, class Function>
        unsigned changePropertyList(QDesignerFormEditorInterface *core,
                                    const QString &propertyName,
                                    PropertyListIterator begin,
                                    PropertyListIterator end,
                                    Function function)
{
    unsigned updateMask = 0;
    QDesignerPropertyEditorInterface *propertyEditor = core->propertyEditor();
    bool updatedPropertyEditor = false;

    for (PropertyListIterator it = begin; it != end; ++it) {
        if (QObject* object = it->object()) { // Might have been deleted in the meantime
            const PropertyHelper::Value newValue = function(*it);
            updateMask |= it->updateMask();
            // Update property editor if it is the current object
            if (!updatedPropertyEditor && propertyEditor && object == propertyEditor->object()) {
                propertyEditor->setPropertyValue(propertyName, newValue.first,  newValue.second);
                updatedPropertyEditor = true;
            }
        }
    }
    if (!updatedPropertyEditor) updateMask |=  PropertyHelper::UpdatePropertyEditor;
    return updateMask;
}


// set a new value, return update mask
unsigned PropertyListCommand::setValue(QVariant value, bool changed, unsigned subPropertyMask)
{
    if(debugPropertyCommands)
        qDebug() << "PropertyListCommand::setValue(" << value <<  changed << subPropertyMask << ")";
    return changePropertyList(formWindow()->core(),
                              m_propertyDescription.m_propertyName, m_propertyHelperList.begin(), m_propertyHelperList.end(),
                              SetValueFunction(formWindow(), PropertyHelper::Value(value, changed), subPropertyMask));
}

// restore old value,  return update mask
unsigned PropertyListCommand::restoreOldValue()
{
    if(debugPropertyCommands)
        qDebug() << "PropertyListCommand::restoreOldValue()";

    return changePropertyList(formWindow()->core(),
                              m_propertyDescription.m_propertyName, m_propertyHelperList.begin(), m_propertyHelperList.end(),
                              UndoSetValueFunction(formWindow()));
}
// set default value,  return update mask
unsigned PropertyListCommand::restoreDefaultValue()
{
    if(debugPropertyCommands)
        qDebug() << "PropertyListCommand::restoreDefaultValue()";

    return changePropertyList(formWindow()->core(),
                              m_propertyDescription.m_propertyName, m_propertyHelperList.begin(), m_propertyHelperList.end(),
                              RestoreDefaultFunction(formWindow()));
}

// update
void PropertyListCommand::update(unsigned updateMask)
{
    if(debugPropertyCommands)
        qDebug() << "PropertyListCommand::update(" << updateMask << ')';

    if (updateMask & PropertyHelper::UpdateObjectInspector) {
        if (QDesignerObjectInspectorInterface *oi = formWindow()->core()->objectInspector())
            oi->setFormWindow(formWindow());
    }

    if (updateMask & PropertyHelper::UpdatePropertyEditor) {
        // this is needed when f.ex. undo, changes parent's palette, but
        // the child is the active widget,
        // TODO: current object?
        if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
            propertyEditor->setObject(propertyEditor->object());
        }
    }
}

void PropertyListCommand::undo()
{
    update(restoreOldValue());
}

// check if lists are aequivalent for command merging (same widgets and props)
bool PropertyListCommand::canMergeLists(const PropertyHelperList& other) const
{
    if (m_propertyHelperList.size() !=  other.size())
        return false;
    for (int i = 0; i < m_propertyHelperList.size(); i++) {
        if (!m_propertyHelperList[i].canMerge(other[i]))
            return false;
    }
    return true;
}

// ---- SetPropertyCommand ----
SetPropertyCommand::SetPropertyCommand(QDesignerFormWindowInterface *formWindow)
    :  PropertyListCommand(formWindow),
       m_subPropertyMask(SubPropertyAll)
{
}

bool SetPropertyCommand::init(QObject *object, const QString &apropertyName, const QVariant &newValue)
{
    Q_ASSERT(object);

    m_newValue = newValue;

    propertyHelperList().clear();
    if (!add(object, apropertyName))
        return false;

    setDescription();
    return true;
}

bool SetPropertyCommand::init(const ObjectList &list, const QString &apropertyName, const QVariant &newValue,
                              QObject *referenceObject)
{
    if (!initList(list, apropertyName, referenceObject))
        return false;

    m_newValue = newValue;

    if(debugPropertyCommands)
        qDebug() << "SetPropertyCommand::init()" << propertyHelperList().size() << '/' << list.size();

    setDescription();

    m_subPropertyMask = subPropertyMask(newValue, referenceObject);
    return true;
}

unsigned SetPropertyCommand::subPropertyMask(const QVariant &newValue, QObject *referenceObject)
{
    // figure out the mask of changed sub properties when comparing newValue to the current value of the reference object.
    if (!referenceObject)
        return SubPropertyAll;

    QDesignerPropertySheetExtension* sheet = propertySheet(referenceObject);
    Q_ASSERT(sheet);

    const int index = sheet->indexOf(propertyName());
    if (index == -1 || !sheet->isVisible(index))
        return SubPropertyAll;

    return compareSubProperties(sheet->property(index), newValue, specialProperty());
}

void SetPropertyCommand::setDescription()
{
    if (propertyHelperList().size() == 1) {
        setText(QApplication::translate("Command", "changed '%1' of '%2'").arg(propertyName()).arg(propertyHelperList()[0].object()->objectName()));
    } else {
        setText(QApplication::translate("Command", "changed '%1' of %2 objects").arg(propertyName()).arg(propertyHelperList().size()));
    }
}

void SetPropertyCommand::redo()
{
    update(setValue(m_newValue, true, m_subPropertyMask));
}


int SetPropertyCommand::id() const
{
    return 1976;
}

bool SetPropertyCommand::mergeWith(const QUndoCommand *other)
{
    if (id() != other->id() || !formWindow()->isDirty())
        return false;

    // Merging: When  for example when the user types ahead in an inplace-editor,
    // it makes sense to merge all the generated commands containing the one-character changes.
    // In the case of subproperties, if the user changes the font size from 10 to 30 via 20
    // and then changes to bold, it makes sense to merge the font size commands only.
    // This is why the m_subPropertyMask is checked.

    const SetPropertyCommand *cmd = static_cast<const SetPropertyCommand*>(other);
    if (!propertyDescription().equals(cmd->propertyDescription()) ||
        !canMergeLists(cmd->propertyHelperList()))
        return false;

    m_newValue = cmd->newValue();
    m_subPropertyMask |= cmd->m_subPropertyMask;
    if(debugPropertyCommands)
        qDebug() << "SetPropertyCommand::mergeWith() succeeded " << propertyName();

    return true;
}

// ---- ResetPropertyCommand ----
ResetPropertyCommand::ResetPropertyCommand(QDesignerFormWindowInterface *formWindow)
    : PropertyListCommand(formWindow)
{
}

bool ResetPropertyCommand::init(QObject *object, const QString &apropertyName)
{
    Q_ASSERT(object);

    propertyHelperList().clear();
    if (!add(object, apropertyName))
        return false;

    setDescription();
    return true;
}

bool ResetPropertyCommand::init(const ObjectList &list, const QString &apropertyName, QObject *referenceObject)
{
    if (!initList(list, apropertyName, referenceObject))
        return false;

    if(debugPropertyCommands)
        qDebug() << "ResetPropertyCommand::init()" << propertyHelperList().size() << '/' << list.size();

    setDescription();
    return true;
}

void ResetPropertyCommand::setDescription()
{
    if (propertyHelperList().size() == 1) {
        setText(QApplication::translate("Command", "reset '%1' of '%2'").arg(propertyName()).arg(propertyHelperList()[0].object()->objectName()));
    } else {
        setText(QApplication::translate("Command", "reset '%1' of %2 objects").arg(propertyName()).arg(propertyHelperList().size()));
    }
}

void ResetPropertyCommand::redo()
{
    update(restoreDefaultValue());
}

AddDynamicPropertyCommand::AddDynamicPropertyCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow),
      m_propertySheet(0)
{

}

void AddDynamicPropertyCommand::init(QObject *object, const QString &propertyName, const QVariant &value)
{
    Q_ASSERT(object);
    m_object = object;
    m_propertyName = propertyName;

    QDesignerFormEditorInterface *core = formWindow()->core();
    m_propertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object);
    Q_ASSERT(m_propertySheet);

    m_value = value;

    setText(QApplication::translate("Command", "add dynamic property '%1' to '%2'").arg(m_propertyName).arg(m_object->objectName()));
}

void AddDynamicPropertyCommand::redo()
{
    m_propertySheet->addDynamicProperty(m_propertyName, m_value);
    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == m_object)
            propertyEditor->setObject(m_object);
    }
}

void AddDynamicPropertyCommand::undo()
{
    m_propertySheet->removeDynamicProperty(m_propertyName);
    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == m_object)
            propertyEditor->setObject(m_object);
    }
}


RemoveDynamicPropertyCommand::RemoveDynamicPropertyCommand(QDesignerFormWindowInterface *formWindow)
    : QDesignerFormWindowCommand(QString(), formWindow),
      m_propertySheet(0)
{

}

void RemoveDynamicPropertyCommand::init(QObject *object, const QString &propertyName)
{
    Q_ASSERT(object);
    m_object = object;
    m_propertyName = propertyName;

    QDesignerFormEditorInterface *core = formWindow()->core();
    m_propertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), object);
    Q_ASSERT(m_propertySheet);

    int index = m_propertySheet->indexOf(m_propertyName);
    m_value = m_propertySheet->property(index);
    m_changed = m_propertySheet->isChanged(index);

    setText(QApplication::translate("Command", "remove dynamic property '%1' to '%2'").arg(m_propertyName).arg(m_object->objectName()));
}

void RemoveDynamicPropertyCommand::redo()
{
    m_propertySheet->removeDynamicProperty(m_propertyName);
    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == m_object)
            propertyEditor->setObject(m_object);
    }
}

void RemoveDynamicPropertyCommand::undo()
{
    m_propertySheet->addDynamicProperty(m_propertyName, m_value);
    m_propertySheet->setChanged(m_propertySheet->indexOf(m_propertyName), m_changed);
    if (QDesignerPropertyEditorInterface *propertyEditor = formWindow()->core()->propertyEditor()) {
        if (propertyEditor->object() == m_object)
            propertyEditor->setObject(m_object);
    }
}


} // namespace qdesigner_internal
