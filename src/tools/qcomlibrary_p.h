/****************************************************************************
** $Id$
**
** Definition of QComLibrary class
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QUCOMLIBRARY_H
#define QUCOMLIBRARY_H

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
class Q_EXPORT QComLibrary : public QLibrary
{
public:
    QComLibrary( const QString &filename );
    ~QComLibrary();

    bool unload();
    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );

private:
    void createInstanceInternal();

    QUnknownInterface *entry;
    QLibraryInterface *libiface;

};
#endif //QT_NO_COMPONENT
#endif // QUCOMLIBRARY_H
