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

#ifndef PROPERTYSHEET_H
#define PROPERTYSHEET_H

#include <extension.h>
#include <QStringList>
#include <QVariant>
#include <QMap>

class QVariant;

class IPropertySheet
{
public:
    virtual ~IPropertySheet() {}

    virtual int count() const = 0;

    virtual int indexOf(const QString &name) const = 0;

    virtual QString propertyName(int index) const = 0;
    virtual QString propertyGroup(int index) const = 0;
    virtual void setPropertyGroup(int index, const QString &group) = 0;

    virtual bool hasReset(int index) const = 0;
    virtual bool reset(int index) = 0;

    virtual bool isVisible(int index) const = 0;
    virtual void setVisible(int index, bool b) = 0;

    virtual bool isAttribute(int index) const = 0;
    virtual void setAttribute(int index, bool b) = 0;

    virtual QVariant property(int index) const = 0;
    virtual void setProperty(int index, const QVariant &value) = 0;

    virtual bool isChanged(int index) const = 0;
    virtual void setChanged(int index, bool changed) = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(IPropertySheet, "http://trolltech.com/Qt/IDE/PropertySheet")

class EnumType
{
public:
    QVariant value;
    QMap<QString, QVariant> items;
};

Q_DECLARE_METATYPE(EnumType)

class FlagType
{
public:
    QVariant value;
    QMap<QString, QVariant> items;
};

Q_DECLARE_METATYPE(FlagType)

namespace Utils
{

inline int valueOf(const QVariant &value, bool *ok = 0)
{
    if (qVariantCanConvert<EnumType>(value))
        return qVariantValue<EnumType>(value).value.toInt(ok);
    else if (qVariantCanConvert<FlagType>(value))
        return qVariantValue<FlagType>(value).value.toInt(ok);

    return value.toInt(ok);
}

} // namespace Utils

#endif // PROPERTYSHEET_H
