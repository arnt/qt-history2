/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWSUTILS_QWS_H
#define QWSUTILS_QWS_H

#ifndef QT_H
#include "qwssocket_qws.h"
#endif // QT_H

/********************************************************************
 *
 * Convenient socket functions
 *
 ********************************************************************/
#ifndef QT_NO_QWS_MULTIPROCESS
inline int qws_read_uint(QWSSocket *socket)
{
    if (!socket || socket->size() < sizeof(int))
        return -1;

    int i;
    socket->readBlock((char*)&i, sizeof(i));

    return i;
}

inline void qws_write_uint(QWSSocket *socket, int i)
{
    if (!socket)
        return;

    socket->writeBlock((char*)&i, sizeof(i));
}

#endif // QT_NO_QWS_MULTIPROCESS

#endif // QWSUTILS_QWS_H
