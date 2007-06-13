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

#ifndef QDESIGNER_PROPERTYSHEET_H
#define QDESIGNER_PROPERTYSHEET_H

#include "shared_global_p.h"
#include "dynamicpropertysheet.h"
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/default_extensionfactory.h>
#include <QtCore/QVariant>
#include <QtCore/QPair>

#include <QPointer>

class QLayout;

class QDESIGNER_SHARED_EXPORT QDesignerPropertySheet: public QObject, public QDesignerPropertySheetExtension, public QDesignerDynamicPropertySheetExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension QDesignerDynamicPropertySheetExtension)
public:
    explicit QDesignerPropertySheet(QObject *object, QObject *parent = 0);
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

    virtual bool dynamicPropertiesAllowed() const;
    virtual int addDynamicProperty(const QString &propertyName, const QVariant &value);
    virtual bool removeDynamicProperty(int index);
    virtual bool isDynamicProperty(int index) const;
    virtual bool canAddDynamicProperty(const QString &propertyName) const;

    void createFakeProperty(const QString &propertyName, const QVariant &value = QVariant());

protected:
    bool isAdditionalProperty(int index) const;
    bool isFakeProperty(int index) const;
    QVariant resolvePropertyValue(const QVariant &value) const;
    QVariant metaProperty(int index) const;
    void setFakeProperty(int index, const QVariant &value);

    bool isFakeLayoutProperty(int index) const;
    bool isDynamic(int index) const;

public: // For MSVC 6
    enum PropertyType { PropertyNone,
                        PropertyLayoutLeftMargin,
                        PropertyLayoutTopMargin,
                        PropertyLayoutRightMargin,
                        PropertyLayoutBottomMargin,
                        PropertyLayoutSpacing,
                        PropertyLayoutHorizontalSpacing,
                        PropertyLayoutVerticalSpacing,
                        PropertySizeConstraint,
                        PropertyBuddy,
                        PropertyAccessibility,
                        PropertyGeometry,
                        PropertyCheckable};

protected:
    enum ObjectType { ObjectNone, ObjectLabel, ObjectLayout, ObjectLayoutWidget, ObjectQ3GroupBox };
    static ObjectType objectType(const QObject *o);
    static  PropertyType propertyTypeFromName(const QString &name);
    PropertyType propertyType(int index) const;

    QObject *object() const;
    const QMetaObject *m_meta;
    const ObjectType m_objectType;

    class Info
    {
    public:
        Info();

        QString group;
        QVariant defaultValue;
        uint changed: 1;
        uint visible: 1;
        uint attribute: 1;
        uint reset: 1;
        uint defaultDynamic: 1;
        PropertyType propertyType;
    };

    Info &ensureInfo(int index);

    typedef QHash<int, Info> InfoHash;
    InfoHash m_info;
    QHash<int, QVariant> m_fakeProperties;
    QHash<int, QVariant> m_addProperties;
    QHash<QString, int> m_addIndex;

private:
    QString transformLayoutPropertyName(int index) const;
    QLayout* layout(QDesignerPropertySheetExtension **layoutPropertySheet = 0) const;

    const bool m_canHaveLayoutAttributes;

    // Variables used for caching the layout, access via layout().
    QPointer<QObject> m_object;
    mutable QPointer<QLayout> m_lastLayout;
    mutable QDesignerPropertySheetExtension *m_lastLayoutPropertySheet;
    mutable bool m_LastLayoutByDesigner;
};

class QDESIGNER_SHARED_EXPORT QDesignerPropertySheetFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    explicit QDesignerPropertySheetFactory(QExtensionManager *parent = 0);

    QObject *extension(QObject *object, const QString &iid) const;

private slots:
    void objectDestroyed(QObject *object);

private:
    typedef QMap<QObject*, QObject*> ExtensionMap;
    mutable ExtensionMap m_extensions;
    typedef QHash<QObject*, bool> ExtendedSet;
    mutable ExtendedSet m_extended;
};

#endif // QDESIGNER_PROPERTYSHEET_H
