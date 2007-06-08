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

#ifndef QMEMROTATE_P_H
#define QMEMROTATE_P_H

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

#include "private/qdrawhelper_p.h"

#define QT_ROTATION_CACHEDREAD 1
#define QT_ROTATION_CACHEDWRITE 2
#define QT_ROTATION_PACKING 3
#define QT_ROTATION_TILED 4

#ifndef QT_ROTATION_ALGORITHM
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
#define QT_ROTATION_ALGORITHM QT_ROTATION_TILED
#else
#define QT_ROTATION_ALGORITHM QT_ROTATION_CACHEDREAD
#endif
#endif

#define QT_DECL_MEMROTATE(srctype, desttype)                             \
    void qt_memrotate90(const srctype*, int, int, int, desttype*, int);  \
    void qt_memrotate180(const srctype*, int, int, int, desttype*, int); \
    void qt_memrotate270(const srctype*, int, int, int, desttype*, int)

QT_DECL_MEMROTATE(quint32, quint32);
QT_DECL_MEMROTATE(quint32, quint16);
QT_DECL_MEMROTATE(quint16, quint32);
QT_DECL_MEMROTATE(quint16, quint16);
#ifdef QT_QWS_DEPTH_24
QT_DECL_MEMROTATE(quint32, quint24);
#endif
#ifdef QT_QWS_DEPTH_18
QT_DECL_MEMROTATE(quint32, quint18);
#endif
QT_DECL_MEMROTATE(quint32, quint8);
QT_DECL_MEMROTATE(quint16, quint8);
QT_DECL_MEMROTATE(quint8, quint8);

#undef QT_DECL_MEMROTATE

#endif // QMEMROTATE_P_H
