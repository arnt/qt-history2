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

#ifndef DEFAULT_PROPERTYSHEET_H
#define DEFAULT_PROPERTYSHEET_H

#include "shared_global.h"
#include <propertysheet.h>
#include <default_extensionfactory.h>
#include <qpair.h>

class QT_SHARED_EXPORT QDesignerPropertySheet: public QObject, public IPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(IPropertySheet)
public:
    QDesignerPropertySheet(QObject *object, QObject *parent = 0);
    virtual ~QDesignerPropertySheet();

    virtual int indexOf(const QString &name) const;

    virtual int count() const;
    virtual QString propertyName(int index) const;

    virtual QString propertyGroup(int index) const;
    virtual void setPropertyGroup(int index, const QString &group);

    virtual bool hasReset(int index) const;
    virtual bool reset(int index);

    virtual bool isAttribute(int index) const;
    virtual void setAttribute(int index, bool b);

    virtual bool isVisible(int index) const;
    virtual void setVisible(int index, bool b);

    virtual QVariant property(int index) const;
    virtual void setProperty(int index, const QVariant &value);

    virtual bool isChanged(int index) const;
    virtual void setChanged(int index, bool changed);

    void createFakeProperty(const QString &propertyName, const QVariant &value = QVariant());

protected:
    bool isAdditionalProperty(int index) const;
    bool isFakeProperty(int index) const;
    QVariant resolvePropertyValue(const QVariant &value) const;
    QVariant metaProperty(int index) const;
    void setFakeProperty(int index, const QVariant &value);

protected:
    QObject *m_object;
    const QMetaObject *meta;

    class Info
    {
    public:
        QString group;
        uint changed: 1;
        uint visible: 1;
        uint attribute: 1;
        uint reset: 1;

        inline Info()
            : changed(0),
              visible(1),
              attribute(0),
              reset(1)
        {}
    };

    QHash<int, Info> m_info;
    QHash<int, QVariant> m_fakeProperties;
    QHash<int, QVariant> m_addProperties;
    QHash<QString, int> m_addIndex;
};

class QT_SHARED_EXPORT QDesignerPropertySheetFactory: public DefaultExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(ExtensionFactory)
public:
    QDesignerPropertySheetFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif // DEFAULT_PROPERTYSHEET_H
