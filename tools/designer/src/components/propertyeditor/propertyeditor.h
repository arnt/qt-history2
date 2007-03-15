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

#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "propertyeditor_global.h"
#include "qpropertyeditor.h"
#include <qdesigner_propertyeditor_p.h>

#include <QtCore/QPointer>

class DomProperty;
class QDesignerMetaDataBaseItemInterface;
class QDesignerPropertySheetExtension;

namespace qdesigner_internal {
class StringProperty;

class QT_PROPERTYEDITOR_EXPORT PropertyEditor: public QDesignerPropertyEditor
{
    Q_OBJECT
public:
    explicit PropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~PropertyEditor();

    virtual QDesignerFormEditorInterface *core() const;

    virtual bool isReadOnly() const;
    virtual void setReadOnly(bool readOnly);
    virtual void setPropertyValue(const QString &name, const QVariant &value, bool changed = true);
    virtual void setPropertyComment(const QString &name, const QString &value);
    virtual void updatePropertySheet();

    virtual void setObject(QObject *object);

    virtual QObject *object() const
    { return m_object; }

    virtual QString currentPropertyName() const;
    
private slots:
    void slotFirePropertyChanged(IProperty *property);
    void slotResetProperty(const QString &prop_name);
    void slotCustomContextMenuRequested(const QPoint &pos);

private:
    IProperty *propertyByName(IProperty *p, const QString &name);
    void clearDirty(IProperty *p);
    void createPropertySheet(PropertyCollection *root, QObject *object);
    static IProperty *createSpecialProperty(const QVariant &value, const QString &name);

private:
    QDesignerMetaDataBaseItemInterface *metaDataBaseItem() const;
    StringProperty* createStringProperty(QObject *object, const QString &pname, const QVariant &value, bool isMainContainer) const;
  
    QDesignerFormEditorInterface *m_core;
    QPropertyEditor *m_editor;
    IPropertyGroup *m_properties;
    QDesignerPropertySheetExtension *m_prop_sheet;
    QPointer<QObject> m_object;
    QMap<int, IProperty *> m_indexToProperty;
};

}  // namespace qdesigner_internal

#endif // PROPERTYEDITOR_H
