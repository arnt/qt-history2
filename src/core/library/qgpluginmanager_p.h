/****************************************************************************
**
** Definition of QGPluginManager class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGPLUGINMANAGER_P_H
#define QGPLUGINMANAGER_P_H

#ifndef QT_H
#include "qhash.h"
#include "qlibrary.h"
#include "quuid.h"
#include "qstringlist.h"
#include "qcom_p.h"
#endif // QT_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_NO_COMPONENT

class Q_CORE_EXPORT QGPluginManager
{
public:
    QGPluginManager(const QUuid& id, const QStringList& paths = QString(), const QString &suffix = QString(), bool cs = true);
    ~QGPluginManager();

    void addLibraryPath(const QString& path);
    const QLibrary* library(const QString& feature) const;
    QStringList featureList() const;

    bool autoUnload() const;
    void setAutoUnload(bool);

protected:
    bool enabled() const;
    bool addLibrary(QLibrary* plugin);

    QRESULT queryUnknownInterface(const QString& feature, QUnknownInterface** iface) const;

    QUuid interfaceId;
    QHash<QString, QLibrary *> plugDict;            // Hash to match feature with library
    QHash<QString, QLibrary *> libDict;            // Hash to match library file with library
    QStringList libList;

    uint casesens : 1;
    uint autounload : 1;
};

inline void QGPluginManager::setAutoUnload(bool unload)
{
    autounload = unload;
}

inline bool QGPluginManager::autoUnload() const
{
    return autounload;
}

#endif

#endif //QGPLUGINMANAGER_P_H
