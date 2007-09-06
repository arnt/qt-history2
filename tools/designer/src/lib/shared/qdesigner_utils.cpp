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
#include "qdesigner_propertycommand_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerIconCacheInterface>

#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <QtCore/QDir>

#include <QtGui/QApplication>
#include <QtCore/QProcess>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDebug>

namespace qdesigner_internal
{
    QDESIGNER_SHARED_EXPORT void designerWarning(const QString &message)
    {
        qWarning("Designer: %s", qPrintable(message));
    }

    // ------------- DesignerMetaEnum
    DesignerMetaEnum::DesignerMetaEnum(const QString &name, const QString &scope, const QString &separator) :
        MetaEnum<int>(name, scope, separator)
    {
    }


    QString DesignerMetaEnum::toString(int value, SerializationMode sm, bool *ok) const
    {
        // find value
        bool valueOk;
        const QString item = valueToKey(value, &valueOk);
        if (ok)
            *ok = valueOk;

        if (!valueOk || sm == NameOnly)
            return item;

        QString qualifiedItem;
        appendQualifiedName(item,  qualifiedItem);
        return qualifiedItem;
    }

    QString DesignerMetaEnum::messageToStringFailed(int value) const
    {
        return QObject::tr("%1 is not a valid enumeration value of '%2'.").arg(value).arg(name());
    }

    QString DesignerMetaEnum::messageParseFailed(const QString &s) const
    {
        return QObject::tr("'%1' could not be converted to an enumeration value of type '%2'.").arg(s).arg(name());
    }
    // -------------- DesignerMetaFlags
    DesignerMetaFlags::DesignerMetaFlags(const QString &name, const QString &scope, const QString &separator) :
       MetaEnum<uint>(name, scope, separator)
    {
    }

    QStringList DesignerMetaFlags::flags(int ivalue) const
    {
        typedef MetaEnum<uint>::KeyToValueMap::const_iterator KeyToValueMapIterator;
        QStringList rc;
        const uint v = static_cast<uint>(ivalue);
        const KeyToValueMapIterator cend = keyToValueMap().constEnd();
        for (KeyToValueMapIterator it = keyToValueMap().constBegin();it != cend; ++it )  {
            const uint itemValue = it.value();
            // Check for equality first as flag values can be 0 or -1, too. Takes preference over a bitwise flag
            if (v == itemValue) {
                rc.clear();
                rc.push_back(it.key());
                return rc;
            }
            // Do not add 0-flags (None-flags)
            if (itemValue)
                if ((v & itemValue) == itemValue)
                    rc.push_back(it.key());
        }
        return rc;
    }


    QString DesignerMetaFlags::toString(int value, SerializationMode sm) const
    {
        const QStringList flagIds = flags(value);
        if (flagIds.empty())
            return QString();

        const QChar delimiter = QLatin1Char('|');
        QString rc;
        const QStringList::const_iterator cend = flagIds.constEnd();
        for (QStringList::const_iterator it = flagIds.constBegin(); it != cend; ++it) {
            if (!rc.isEmpty())
                rc += delimiter ;
            if (sm == FullyQualified)
                appendQualifiedName(*it, rc);
            else
                rc += *it;
        }
        return rc;
    }


    int DesignerMetaFlags::parseFlags(const QString &s, bool *ok) const
    {
        if (s.isEmpty()) {
            if (ok)
                *ok = true;
            return 0;
        }
        uint flags = 0;
        bool valueOk = true;
        QStringList keys = s.split(QString(QLatin1Char('|')));
        const QStringList::iterator cend = keys.end();
        for (QStringList::iterator it = keys.begin(); it != cend; ++it) {
            const uint flagValue = keyToValue(*it, &valueOk);
            if (!valueOk) {
                flags = 0;
                break;
            }
            flags |= flagValue;
        }
        if (ok)
            *ok = valueOk;
        return static_cast<int>(flags);
    }

    QString DesignerMetaFlags::messageParseFailed(const QString &s) const
    {
        return QObject::tr("'%1' could not be converted to a flag value of type '%2'.").arg(s).arg(name());
    }

    // ---------- PropertySheetEnumValue

    PropertySheetEnumValue::PropertySheetEnumValue(int v, const DesignerMetaEnum &me) :
       value(v),
       metaEnum(me)
    {
    }
    PropertySheetEnumValue::PropertySheetEnumValue() :
       value(0)
    {
    }

    // ---------------- PropertySheetFlagValue
    PropertySheetFlagValue::PropertySheetFlagValue(int v, const DesignerMetaFlags &mf) :
        value(v),
        metaFlags(mf)
    {
    }

    PropertySheetFlagValue::PropertySheetFlagValue() :
        value(0)
    {
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

    QDESIGNER_SHARED_EXPORT QDesignerFormWindowCommand *createTextPropertyCommand(const QString &propertyName, const QString &text, QObject *object, QDesignerFormWindowInterface *fw)
    {
        if (text.isEmpty()) {
            ResetPropertyCommand *cmd = new ResetPropertyCommand(fw);
            cmd->init(object, propertyName);
            return cmd;
        }
        SetPropertyCommand *cmd = new SetPropertyCommand(fw);
        cmd->init(object, propertyName, text);
        return cmd;
    }

    QDESIGNER_SHARED_EXPORT bool runUIC(const QString &fileName, UIC_Mode mode, QByteArray& ba, QString &errorMessage)
    {
        QStringList argv;
        QString binary = QLibraryInfo::location(QLibraryInfo::BinariesPath);
        binary += QDir::separator();
        switch (mode) {
        case UIC_GenerateCode:
            binary += QLatin1String("uic");
            break;
        case UIC_ConvertV3:
            binary += QLatin1String("uic3");
            argv += QLatin1String("-convert");
            break;
        }
        argv += fileName;
        QProcess uic;
        uic.start(binary, argv);
        if (!uic.waitForStarted()) {
            errorMessage = QApplication::translate("Designer", "Unable to launch %1.").arg(binary);
            return false;
        }
        if (!uic.waitForFinished()) {
            errorMessage = QApplication::translate("Designer", "%1 timed out.").arg(binary);
            return false;
        }
        if (uic.exitCode()) {
            errorMessage =  uic.readAllStandardError();
            return false;
        }
        ba = uic.readAllStandardOutput();
        return true;
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
