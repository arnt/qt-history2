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

class QDesignerLanguageExtension;

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtGui/QMainWindow>

class QIcon;
class QPixmap;

namespace qdesigner_internal {
class ResourceMimeData;

QDESIGNER_SHARED_EXPORT void designerWarning(const QString &message);

class QDESIGNER_SHARED_EXPORT EnumType
{
public:
    typedef QMap<QString, QVariant> ItemMap;

    QString id() const;
    QString id(const QDesignerLanguageExtension *lang) const;

    void remapKeys(const QDesignerLanguageExtension *lang);

    QVariant value;
    ItemMap items;
    QStringList names;
};


class QDESIGNER_SHARED_EXPORT FlagType
{
public:
    typedef QMap<QString, QVariant> ItemMap;

    QStringList flags(const QDesignerLanguageExtension *lang) const;
    QString flagString(const QDesignerLanguageExtension *lang) const;

    void remapKeys(const QDesignerLanguageExtension *lang);

    QVariant value;
    ItemMap items;
};

// Convenience to return a dropped icon, normalized to form directory
QDESIGNER_SHARED_EXPORT QIcon resourceMimeDataToIcon(const ResourceMimeData *rmd, QDesignerFormWindowInterface *fw);
// Convenience to return an dropped pixmap, normalized to form directory
QDESIGNER_SHARED_EXPORT QPixmap resourceMimeDataToPixmap(const ResourceMimeData *rmd, QDesignerFormWindowInterface *fw);
// Find a suitable variable name for a class.
QDESIGNER_SHARED_EXPORT QString qtify(const QString &name);
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
