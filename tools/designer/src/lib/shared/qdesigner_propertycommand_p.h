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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_PROPERTYCOMMAND_H
#define QDESIGNER_PROPERTYCOMMAND_H

#include "qdesigner_formwindowcommand_p.h"

#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QPair>

class QDesignerFormWindowInterface;
class QDesignerPropertySheetExtension;

namespace qdesigner_internal {

enum SpecialProperty {
        SP_None, SP_ObjectName, SP_WindowTitle,
        SP_MinimumSize, SP_MaximumSize, SP_Geometry, SP_Icon,SP_CurrentTabName,
        SP_Alignment
};

//Determine special property
enum SpecialProperty getSpecialProperty(const QString& propertyName);


// A helper class for applying properties to objects.
// Can be used for Set commands (setValue(), restoreOldValue()) or
// Reset Commands     restoreDefaultValue(), restoreOldValue()).
class PropertyHelper {
public:
    // A pair of Value and changed flag
    typedef QPair<QVariant, bool> Value;

    enum ObjectType {OT_Object, OT_Action, OT_Widget};

    PropertyHelper(QObject* object,
                   SpecialProperty specialProperty,
                   QDesignerPropertySheetExtension *sheet,
                   int index);

    QObject *object() const { return m_object; }
    SpecialProperty specialProperty() const { return m_specialProperty; }
    // set a new value
    Value setValue(QDesignerFormWindowInterface *fw, const QVariant &value, bool changed, unsigned subPropertyMask);

    // restore old value
    Value restoreOldValue(QDesignerFormWindowInterface *fw);
    // set default value
    Value restoreDefaultValue(QDesignerFormWindowInterface *fw);

    inline QVariant oldValue() const
    { return m_oldValue.first; }

    inline void setOldValue(const QVariant &oldValue)
    { m_oldValue.first = oldValue; }

    // required updates for this property (bit mask)
    enum UpdateMask { UpdatePropertyEditor=1, UpdateObjectInspector=2 };
    unsigned updateMask() const;

    // can be merged into one command (that is, object and name match)
    bool canMerge(const PropertyHelper &other) const;
private:
    // Apply the value and update. Returns corrected value
    Value applyValue(QDesignerFormWindowInterface *fw, const QVariant &oldValue, Value newValue);

    static void checkApplyWidgetValue(QDesignerFormWindowInterface *fw, QWidget* w,
                                      SpecialProperty specialProperty, QVariant &v);

    void updateObject(QDesignerFormWindowInterface *fw, const QVariant &oldValue, const QVariant &newValue);
    QVariant findDefaultValue(QDesignerFormWindowInterface *fw) const;

    SpecialProperty m_specialProperty;

    QPointer<QObject> m_object;
    ObjectType m_objectType;
    QPointer<QWidget> m_parentWidget;

    QDesignerPropertySheetExtension *m_propertySheet;
    int m_index;

    Value m_oldValue;
};

// Base class for commands that can be applied to several widgets

class QDESIGNER_SHARED_EXPORT PropertyListCommand : public QDesignerFormWindowCommand {
public:
    PropertyListCommand(QDesignerFormWindowInterface *formWindow);

    QObject* object(int index = 0) const;

    QVariant oldValue(int index = 0) const;

    void setOldValue(const QVariant &oldValue, int index = 0);

    // Calls restoreDefaultValue() and update()
    virtual void undo();
protected:
    typedef QList<PropertyHelper> PropertyHelperList;

    // add an object
    bool add(QObject *object, const QString &propertyName);

    // set a new value, return update mask
    unsigned setValue(QVariant value, bool changed, unsigned subPropertyMask);

    // restore old value,  return update mask
    unsigned  restoreOldValue();
    // set default value,  return update mask
    unsigned  restoreDefaultValue();

    // update designer
    void update(unsigned updateMask);

    // check if lists are aequivalent for command merging (same widgets and props)
    bool canMergeLists(const PropertyHelperList& other) const;

    PropertyHelperList& propertyHelperList() { return m_propertyHelperList; }
    const PropertyHelperList& propertyHelperList() const { return m_propertyHelperList; }

    const QString propertyName() const { return m_propertyName; }
    QVariant::Type propertyType() const  { return m_propertyType; }
    SpecialProperty specialProperty() const;
private:
    QString m_propertyName;
    QVariant::Type m_propertyType;
    SpecialProperty m_specialProperty;
    PropertyHelperList m_propertyHelperList;
};

class QDESIGNER_SHARED_EXPORT SetPropertyCommand: public PropertyListCommand
{

public:
    typedef QList<QObject *> ObjectList;

    SetPropertyCommand(QDesignerFormWindowInterface *formWindow);

    bool init(QObject *object, const QString &propertyName, const QVariant &newValue);
    bool init(const ObjectList &list, const QString &propertyName, const QVariant &newValue,
              QObject *referenceObject = 0);


    inline QVariant newValue() const
    { return m_newValue; }

    inline void setNewValue(const QVariant &newValue)
    { m_newValue = newValue; }

    int id() const;
    bool mergeWith(const QUndoCommand *other);

    virtual void redo();
private:
    unsigned subPropertyMask(const QVariant &newValue, QObject *referenceObject);
    void setDescription();
    QVariant m_newValue;
    unsigned m_subPropertyMask;
};

class QDESIGNER_SHARED_EXPORT ResetPropertyCommand: public PropertyListCommand
{

public:
    typedef QList<QObject *> ObjectList;

    ResetPropertyCommand(QDesignerFormWindowInterface *formWindow);

    bool init(QObject *object, const QString &propertyName);
    bool init(const ObjectList &list, const QString &propertyName);

    virtual void redo();

protected:
    virtual bool mergeWith(const QUndoCommand *) { return false; }

private:
    void setDescription();
    QString m_propertyName;
};

} // namespace qdesigner_internal

#endif // QDESIGNER_PROPERTYCOMMAND_H
