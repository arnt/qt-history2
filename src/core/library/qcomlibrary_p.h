/****************************************************************************
**
** Definition of QComLibrary class.
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

#ifndef QCOMLIBRARY_P_H
#define QCOMLIBRARY_P_H

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
#include "qcom_p.h"
#include "qlibrary.h"
#endif // QT_H

#ifndef QT_NO_COMPONENT

class Q_CORE_EXPORT QComLibrary : public QLibrary
{
public:
    QComLibrary( const QString &filename );
    ~QComLibrary();

    bool unload();
    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    uint qtVersion();

private:
    void createInstanceInternal();

    QUnknownInterface *entry;
    QLibraryInterface *libiface;
    uint qt_version;

};

#endif //QT_NO_COMPONENT

#endif
