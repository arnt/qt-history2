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

#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "propertyeditor_global.h"

#include <abstractformeditor.h>
#include <abstractpropertyeditor.h>

#include <qpropertyeditor.h>
#include <QPointer>

class DomProperty;
class IPropertySheet;

class QT_PROPERTYEDITOR_EXPORT PropertyEditor: public AbstractPropertyEditor
{
    Q_OBJECT
public:
    PropertyEditor(AbstractFormEditor *core, QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~PropertyEditor();

    virtual AbstractFormEditor *core() const;

    virtual bool isReadOnly() const;
    virtual void setReadOnly(bool readOnly);
    virtual void setPropertyValue(const QString &name, const QVariant &value, bool changed = true);
    virtual void setObject(QObject *object);

    virtual QObject *object() const
    { return m_object; }

private slots:
    void firePropertyChanged(IProperty *property);
    void resetProperty(const QString &prop_name);

private:
    IProperty *propertyByName(IProperty *p, const QString &name);
    void clearDirty(IProperty *p);

    void createPropertySheet(PropertyCollection *root, QObject *object);

    static IProperty *createSpecialProperty(const QVariant &value,
            const QString &name);

private:
    AbstractFormEditor *m_core;
    QPropertyEditor::View *m_editor;
    IPropertyGroup *m_properties;
    IPropertySheet *m_prop_sheet;
    QPointer<QObject> m_object;
};

#endif // PROPERTYEDITOR_H
