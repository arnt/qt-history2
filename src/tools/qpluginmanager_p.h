/****************************************************************************
**
** Definition of QPluginManager class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPLUGINMANAGER_P_H
#define QPLUGINMANAGER_P_H

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

#ifndef QT_H
#include "qgpluginmanager_p.h"
#endif // QT_H

#ifndef QT_NO_COMPONENT

template<class Type>
class QPluginManager : public QGPluginManager
{
public:
    QPluginManager( const QUuid& id, const QStringList& paths = QString(), const QString &suffix = QString(), bool cs = TRUE )
	: QGPluginManager( id, paths, suffix, cs ) {}
    QRESULT queryInterface(const QString& feature, Type** iface) const
    {
	return queryUnknownInterface( feature, (QUnknownInterface**)iface );
    }
};

#endif //QT_NO_COMPONENT

#endif //QPLUGINMANAGER_P_H
