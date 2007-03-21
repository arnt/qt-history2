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

#include "qdesigner_utils_p.h"
#include "resourcemimedata_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerIconCacheInterface>

#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <QtCore/QDir>


namespace qdesigner_internal
{
    QDESIGNER_SHARED_EXPORT void designerWarning(const QString &message)
    {
        QString prefixedMessage = QLatin1String("Designer: ");
        prefixedMessage += message;
        qWarning(prefixedMessage.toUtf8().constData());
    }

    QString EnumType::id() const
    {
        const int v = value.toInt();
        const ItemMap::const_iterator cend = items.constEnd();
        for (ItemMap::const_iterator it = items.constBegin();it != cend; ++it )  {
            if (it.value().toInt() == v)
                return it.key();
        }
        return QString();
    }

    // ------- FlagType
    QStringList FlagType::flags() const
    {
        QStringList rc;
        const uint v = value.toUInt();
        const ItemMap::const_iterator cend = items.constEnd();
        for (ItemMap::const_iterator it = items.constBegin();it != cend; ++it )  {
            const uint itemValue = it.value().toUInt();
            // Check for equality first as flag values can be 0 or -1, too. Takes preference over a bitwise flag
            if (v == itemValue) {
                rc.clear();
                rc.push_back(it.key());
                return rc;
            }
            if (v & itemValue)
                rc.push_back(it.key());
        }
        return rc;
    }

    QString FlagType::flagString() const
    {
        const QStringList flagIds = flags();
        switch (flagIds.size()) {
        case 0:
            return QString();
        case 1:
            return flagIds.front();
        default:
            break;
        }
        static const QString delimiter = QString(QLatin1Char('|'));
        return flagIds.join(delimiter);
    }

    // Convenience to return an icon normalized to form directory
    QDESIGNER_SHARED_EXPORT QIcon resourceMimeDataToIcon(const ResourceMimeData &rmd, QDesignerFormWindowInterface *fw)
    {
        if (rmd.type() != ResourceMimeData::Image)
            return QIcon();

        const QString normalizedQrcPath = fw->absoluteDir().absoluteFilePath(rmd.qrcPath());
        const QIcon rc =  fw->core()->iconCache()->nameToIcon(rmd.filePath(), normalizedQrcPath);
        return rc;
    }
    // Convenience to return an icon normalized to form directory
    QDESIGNER_SHARED_EXPORT QPixmap resourceMimeDataToPixmap(const ResourceMimeData &rmd, QDesignerFormWindowInterface *fw)
    {
        if (rmd.type() != ResourceMimeData::Image)
            return QPixmap();

        const QString normalizedQrcPath = fw->absoluteDir().absoluteFilePath(rmd.qrcPath());
        const QPixmap rc =  fw->core()->iconCache()->nameToPixmap(rmd.filePath(), normalizedQrcPath);
        return rc;
    }

    QDESIGNER_SHARED_EXPORT QString qtify(const QString &name)
    {
        QString qname = name;

        Q_ASSERT(qname.isEmpty() == false);


        if (qname.count() > 1 && qname.at(1).isUpper()) {
            const QChar first = qname.at(0);
            if (first == QLatin1Char('Q') || first == QLatin1Char('K'))
                qname.remove(0, 1);
        }

        const int len = qname.count();
        for (int i = 0; i < len && qname.at(i).isUpper(); i++)
            qname[i] = qname.at(i).toLower();

        return qname;
    }
} // namespace qdesigner_internal
