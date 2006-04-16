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

#include <qimage.h>
#include <qwsmemid_qws.h>
#include <private/qsharedmemory_p.h>

class QWSLock;

class QWSBackingStore
{
public:
    QWSBackingStore();
    ~QWSBackingStore();

    void create(QSize size, QImage::Format, int windowType);
    bool createIfNecessary(QWidget *tlw);
#ifndef QT_NO_QWS_MULTIPROCESS
    void attach(QWSMemId id, QSize size, QImage::Format, int windowType);
#endif
    void detach();
    void setMemory(QWSMemId id, const QSize &size, QImage::Format, int windowType);

    bool lock(int timeout = -1);
    void unlock();
    bool wait(int timeout = -1);
    void setLock(QWSLock *l);

    const QImage &image() const { return img; }
    QPaintDevice *paintDevice() { return &img; }

    void blit(const QRect &src, const QPoint &dest);

    QWSMemId memoryId() const;
    QSize size() const { return img.size(); }

    bool isNull() const { return img.isNull(); }

    enum WindowType {Transparent=0, Opaque=1, OnScreen=2, NoBS=3, YellowThing=4};

    int windowType() const { return _windowType; }

    QPoint tlwOffset() const { return offs; }
private:
    QImage img;
#ifndef QT_NO_QWS_MULTIPROCESS
    QSharedMemory shm;
#endif
    uchar *mem;

    QRegion clipRgn;
    QPoint offs;
    bool isSharedMemory : 1;
    bool ownsMemory : 1;

    int _windowType;

    QWSLock *memLock;
};

#endif // QWIDGET_QWS_P_H
