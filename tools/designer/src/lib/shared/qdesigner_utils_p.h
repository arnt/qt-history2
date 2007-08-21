
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

#ifndef QDESIGNER_UTILS_H
#define QDESIGNER_UTILS_H

#include "shared_global_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtGui/QMainWindow>

class QIcon;
class QPixmap;

namespace qdesigner_internal {
class ResourceMimeData;
class QDesignerFormWindowCommand;


QDESIGNER_SHARED_EXPORT void designerWarning(const QString &message);

/* Flag/Enumeration helpers for the property sheet: Enumeration or flag values are returned by the property sheet
 * as a pair of meta type and integer value.
 * The meta type carries all the information required for the property editor and serialization
 * by the form builders (names, etc).
 * Note that the property editor uses unqualified names ("Cancel") while the form builder serialization  (uic)
 * requires the whole string
 * ("QDialogButtonBox::Cancel" or "com.trolltech.qt.gui.QDialogButtonBox.StandardButton.Cancel").*/

/* --------- MetaEnum: Base class representing a QMetaEnum with lookup functions
 * in both ways. Template of int type since unsigned is more suitable for flags.
 * The keyToValue() is ignorant of scopes, it can handle fully qualified or unqualified names. */

template <class IntType>
class MetaEnum
{
public:
    typedef QMap<QString, IntType> KeyToValueMap;

    MetaEnum(const QString &name, const QString &scope, const QString &separator);
    MetaEnum() {}
    void addKey(IntType value, const QString &name);

    QString valueToKey(IntType value, bool *ok = 0) const;
    // Ignorant of scopes.
    IntType keyToValue(QString key, bool *ok = 0) const;

    const QString &name() const      { return m_name; }
    const QString &scope() const     { return m_scope; }
    const QString &separator() const { return m_separator; }

    const QStringList &keys() const { return m_keys; }
    const KeyToValueMap &keyToValueMap() const { return m_keyToValueMap; }

protected:
    void appendQualifiedName(const QString &key, QString &target) const;

private:
    QString m_name;
    QString m_scope;
    QString m_separator;
    KeyToValueMap m_keyToValueMap;
    QStringList m_keys;
};

template <class IntType>
MetaEnum<IntType>::MetaEnum(const QString &name, const QString &scope, const QString &separator) :
    m_name(name),
    m_scope(scope),
    m_separator(separator)
{
}

template <class IntType>
void MetaEnum<IntType>::addKey(IntType value, const QString &name)
{
    m_keyToValueMap.insert(name, value);
    m_keys.append(name);
}

template <class IntType>
QString MetaEnum<IntType>::valueToKey(IntType value, bool *ok) const
{
    const QString rc = m_keyToValueMap.key(value);
    if (ok)
        *ok = !rc.isEmpty();
    return rc;
}

template <class IntType>
IntType MetaEnum<IntType>::keyToValue(QString key, bool *ok) const
{
    if (!m_scope.isEmpty() && key.startsWith(m_scope))
        key.remove(0, m_scope.size() + m_separator.size());
    const Q_TYPENAME KeyToValueMap::const_iterator it = m_keyToValueMap.find(key);
    const bool found = it != m_keyToValueMap.constEnd();
    if (ok)
        *ok = found;
    return found ? it.value() : IntType(0);
}

template <class IntType>
void MetaEnum<IntType>::appendQualifiedName(const QString &key, QString &target) const
{
    if (!m_scope.isEmpty()) {
        target += m_scope;
        target += m_separator;
    }
    target += key;
}

// -------------- DesignerMetaEnum: Meta type for enumerations

class QDESIGNER_SHARED_EXPORT DesignerMetaEnum : public MetaEnum<int>
{
public:
    DesignerMetaEnum(const QString &name, const QString &scope, const QString &separator);
    DesignerMetaEnum() {}

    enum SerializationMode { FullyQualified, NameOnly };
    QString toString(int value, SerializationMode sm, bool *ok = 0) const;

    QString messageToStringFailed(int value) const;
    QString messageParseFailed(const QString &s) const;

    // parse a string (ignorant of scopes)
    int parseEnum(const QString &s, bool *ok = 0) const { return keyToValue(s, ok); }
};

// -------------- DesignerMetaFlags: Meta type for flags.
// Note that while the handling of flags is done using unsigned integers, the actual values returned
// by the property system  are integers.

class QDESIGNER_SHARED_EXPORT DesignerMetaFlags : public MetaEnum<uint>
{
public:
    DesignerMetaFlags(const QString &name, const QString &scope, const QString &separator);
    DesignerMetaFlags() {}

    enum SerializationMode { FullyQualified, NameOnly };
    QString toString(int value, SerializationMode sm) const;
    QStringList flags(int value) const;

    QString messageParseFailed(const QString &s) const;
    // parse a string (ignorant of scopes)
    int parseFlags(const QString &s, bool *ok = 0) const;
};

// -------------- EnumValue: Returned by the property sheet for enumerations

struct QDESIGNER_SHARED_EXPORT PropertySheetEnumValue
{
    PropertySheetEnumValue(int v, const DesignerMetaEnum &me);
    PropertySheetEnumValue();

    int value;
    DesignerMetaEnum metaEnum;
};

// -------------- FlagValue: Returned by the property sheet for flags

struct QDESIGNER_SHARED_EXPORT PropertySheetFlagValue
{
    PropertySheetFlagValue(int v, const DesignerMetaFlags &mf);
    PropertySheetFlagValue();

    int value;
    DesignerMetaFlags metaFlags;
};

// Convenience to return a dropped icon, normalized to form directory
QDESIGNER_SHARED_EXPORT QIcon resourceMimeDataToIcon(const ResourceMimeData &rmd, QDesignerFormWindowInterface *fw);
// Convenience to return an dropped pixmap, normalized to form directory
QDESIGNER_SHARED_EXPORT QPixmap resourceMimeDataToPixmap(const ResourceMimeData &rmd, QDesignerFormWindowInterface *fw);
// Create a command to change a text property (that is, create a reset property command if the text is empty)
QDESIGNER_SHARED_EXPORT QDesignerFormWindowCommand *createTextPropertyCommand(const QString &propertyName, const QString &text, QObject *object, QDesignerFormWindowInterface *fw);

// Convenience to run UIC
enum UIC_Mode { UIC_GenerateCode, UIC_ConvertV3 };
QDESIGNER_SHARED_EXPORT bool runUIC(const QString &fileName, UIC_Mode mode, QByteArray& ba, QString &errorMessage);

// Find a suitable variable name for a class.
QDESIGNER_SHARED_EXPORT QString qtify(const QString &name);
} // namespace qdesigner_internal

Q_DECLARE_METATYPE(qdesigner_internal::PropertySheetEnumValue)
Q_DECLARE_METATYPE(qdesigner_internal::PropertySheetFlagValue)

namespace qdesigner_internal { namespace Utils {

inline int valueOf(const QVariant &value, bool *ok = 0)
{
    if (qVariantCanConvert<PropertySheetEnumValue>(value)) {
        if (ok)
            *ok = true;
        return qVariantValue<PropertySheetEnumValue>(value).value;
    }
    else if (qVariantCanConvert<PropertySheetFlagValue>(value)) {
        if (ok)
            *ok = true;
        return qVariantValue<PropertySheetFlagValue>(value).value;
    }
    return value.toInt(ok);
}

inline bool isObjectAncestorOf(QObject *ancestor, QObject *child)
{
    QObject *obj = child;
    while (obj != 0) {
        if (obj == ancestor)
            return true;
        obj = obj->parent();
    }
    return false;
}

inline bool isCentralWidget(QDesignerFormWindowInterface *fw, QWidget *widget)
{
    if (! fw || ! widget)
        return false;

    if (widget == fw->mainContainer())
        return true;

    // ### generalize for other containers
    if (QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer())) {
        return mw->centralWidget() == widget;
    }

    return false;
}

} // namespace Utils

} // namespace qdesigner_internal

#endif // QDESIGNER_UTILS_H
