#ifndef QDESIGNER_PROPERTYEDITOR_H
#define QDESIGNER_PROPERTYEDITOR_H

#include "propertyeditor_global.h"

#include <abstractformeditor.h>
#include <abstractpropertyeditor.h>

#include <QPropertyEditor>
#include <QPointer>

class DomProperty;

class QT_PROPERTYEDITOR_EXPORT PropertyEditor: public AbstractPropertyEditor
{
    Q_OBJECT
public:
    PropertyEditor(AbstractFormEditor *core, QWidget *parent, Qt::WFlags flags = 0);
    virtual ~PropertyEditor();

    virtual AbstractFormEditor *core() const;

    virtual bool isReadOnly() const;
    virtual void setReadOnly(bool readOnly);
    virtual void setPropertyValue(const QString &name, const QVariant &value);
    virtual void setObject(QObject *object);

    virtual QObject *object() const
    { return m_object; }

private slots:
    void firePropertyChanged(I::Property *property);

private:
    I::Property *propertyByName(I::Property *p, const QString &name);
    void clearDirty(I::Property *p);
    
    static void createPropertySheet(PropertyCollection *root,
            QExtensionManager *m,
            QObject *object);
        
    static I::Property *createSpecialProperty(const QVariant &value, 
            const QString &name);

private:
    AbstractFormEditor *m_core;
    QPropertyEditor::View *m_editor;
    I::PropertyGroup *m_properties;
    QPointer<QObject> m_object;
};

#endif // QDESIGNER_PROPERTYEDITOR_H
