/****************************************************************************
**
** Definition of the QComponentFactory class.
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

#ifndef QCOMPONENTFACTORY_P_H
#define QCOMPONENTFACTORY_P_H

#ifndef QT_H
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

class Q_EXPORT QComponentFactory
{
public:
    static QRESULT createInstance( const QString &cid, const QUuid &iid, QUnknownInterface** instance, QUnknownInterface *outer = 0 );
    static QRESULT registerServer( const QString &filename );
    static QRESULT unregisterServer( const QString &filename );

    static bool registerComponent( const QUuid &cid, const QString &filename, const QString &name = QString::null, 
				   int version = 0, const QString &description = QString::null );
    static bool unregisterComponent( const QUuid &cid );
};

#endif // QT_NO_COMPONENT

#endif // QCOMPONENTFACTORY_P_H
