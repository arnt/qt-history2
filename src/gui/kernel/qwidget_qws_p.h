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

#include <qbytearray.h>

class QWSBackingStore
{
public:
    QWSBackingStore();
    ~QWSBackingStore();

    void create(QSize size);
    void attach(int shmid, QSize size);
    void detach();

    void lock(bool write=false);
    void unlock();

    QPixmap *pixmap();

    void blt(const QRect &src, const QPoint &dest);

    int memoryId() const { return shmid; }
    QSize size() const;

private:
    QPixmap *pix;
    int shmid;
    void *shmaddr;
};

#endif // QWIDGET_QWS_P_H
