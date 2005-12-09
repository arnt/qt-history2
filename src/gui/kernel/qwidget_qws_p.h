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

#ifndef QWIDGET_QWS_P_H
#define QWIDGET_QWS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qpixmap.h>
#include <private/qsharedmemory_p.h>

class QWSBackingStore
{
public:
    QWSBackingStore();
    ~QWSBackingStore();

    void create(QSize size);
    void attach(int shmid, QSize size);
    void detach();

    void lock();
    void unlock();

    QPixmap *pixmap() { return &pix; }

    void blit(const QRect &src, const QPoint &dest);

    int memoryId() const { return shm.id(); }
    QSize size() const { return pix.size(); }

    bool isNull() const { return pix.isNull(); }
private:
    QPixmap pix;
    QSharedMemory shm;
};

#endif // QWIDGET_QWS_P_H
