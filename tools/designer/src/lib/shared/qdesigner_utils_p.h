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

#include <QtCore/QVariant>
#include <QtCore/QMap>

namespace qdesigner_internal {

class EnumType
{
public:
    QVariant value;
    QMap<QString, QVariant> items;
};


class FlagType
{
public:
    QVariant value;
    QMap<QString, QVariant> items;
};

} // namespace qdesigner_internal

Q_DECLARE_METATYPE(qdesigner_internal::EnumType)
Q_DECLARE_METATYPE(qdesigner_internal::FlagType)

namespace qdesigner_internal { namespace Utils {

inline int valueOf(const QVariant &value, bool *ok = 0)
{
    if (qVariantCanConvert<EnumType>(value))
        return qVariantValue<EnumType>(value).value.toInt(ok);
    else if (qVariantCanConvert<FlagType>(value))
        return qVariantValue<FlagType>(value).value.toInt(ok);

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

} // namespace Utils

} // namespace qdesigner_internal

#endif // QDESIGNER_UTILS_H
