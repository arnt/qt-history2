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

#ifndef QCOREGLOBALDATA_P_H
#define QCOREGLOBALDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qmap.h"
#include "QtCore/qstringlist.h"
#include "QtCore/qreadwritelock.h"

struct QCoreGlobalData {
    QMap<QString, QStringList> dirSearchPaths;
    QReadWriteLock dirSearchPathsLock;

    static QCoreGlobalData *instance();
};

#endif // QCOREGLOBALDATA_P_H

