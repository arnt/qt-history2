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

#ifndef QWSMEMID_QWS_H
#define QWSMEMID_QWS_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

union QWSMemId
{
    QWSMemId() {}
    QWSMemId(int id) { shmid = id; }
    QWSMemId(uchar *addr) { address = addr; }
    operator uchar* () { return address; }
    operator int () { return shmid; }

    uchar *address;
    int shmid;
};

QT_END_HEADER

#endif // QWSMEMID_QWS_H
