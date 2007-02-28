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

#ifndef QWINDOWSURFACE_QWS_P_H
#define QWINDOWSURFACE_QWS_P_H

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

#include "qwindowsurface_p.h"
#include <qregion.h>
#include <qimage.h>
#include <private/qsharedmemory_p.h>

class QScreen;
class QWSWindowSurfacePrivate;

// TODO: add windowId(), protected createWindowId()

class Q_GUI_EXPORT QWSWindowSurface : public QWindowSurface
{
public:
    QWSWindowSurface();
    QWSWindowSurface(QWidget *widget);
    ~QWSWindowSurface();

    virtual bool isValid() const = 0;

    virtual void setGeometry(const QRect &rect);
    virtual void flush(QWidget *widget, const QRegion &region,
                       const QPoint &offset);

    virtual QPoint painterOffset() const; // remove!!!

    virtual void beginPaint(const QRegion &);
    virtual void endPaint(const QRegion &);

    virtual bool lock(int timeout = -1);
    virtual void unlock();

    virtual QString key() const = 0;

    // XXX: not good enough
    virtual QByteArray transientState() const;
    virtual QByteArray permanentState() const;
    virtual void setTransientState(const QByteArray &state);
    virtual void setPermanentState(const QByteArray &state);

    virtual QImage image() const = 0;
    virtual QPaintDevice *paintDevice() = 0;


    const QRegion dirtyRegion() const;
    void setDirty(const QRegion &) const;

    const QRegion clipRegion() const;
    void setClipRegion(const QRegion &);

    enum SurfaceFlag {
        RegionReserved = 0x1,
        Buffered = 0x2,
        Opaque = 0x4
    };
    Q_DECLARE_FLAGS(SurfaceFlags, SurfaceFlag)

    SurfaceFlags surfaceFlags() const;

    inline bool isRegionReserved() const {
        return surfaceFlags() & RegionReserved;
    }
    inline bool isBuffered() const { return surfaceFlags() & Buffered; }
    inline bool isOpaque() const { return surfaceFlags() & Opaque; }

protected:
    void setSurfaceFlags(SurfaceFlags type);

private:
    QWSWindowSurfacePrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWSWindowSurface::SurfaceFlags)

class QWSLock;

class Q_GUI_EXPORT QWSMemorySurface : public QWSWindowSurface
{
public:
    QWSMemorySurface();
    QWSMemorySurface(QWidget *widget);
    ~QWSMemorySurface();

    bool isValid() const;

    QPaintDevice *paintDevice() { return &img; }
    bool scroll(const QRegion &area, int dx, int dy);

    QImage image() const { return img; };
    QPoint painterOffset() const;

    bool lock(int timeout = -1);
    void unlock();

protected:
    QImage::Format preferredImageFormat(const QWidget *widget) const;
    void setLock(int lockId);

    QWSLock *memlock;
    QImage img;
};

class Q_GUI_EXPORT QWSLocalMemSurface : public QWSMemorySurface
{
public:
    QWSLocalMemSurface();
    QWSLocalMemSurface(QWidget *widget);
    ~QWSLocalMemSurface();

    void setGeometry(const QRect &rect);

    QString key() const { return QLatin1String("mem"); }
    QByteArray permanentState() const;

    void setPermanentState(const QByteArray &data);

protected:
    uchar *mem;
    int memsize;
};

#ifndef QT_NO_QWS_MULTIPROCESS
class Q_GUI_EXPORT QWSSharedMemSurface : public QWSMemorySurface
{
public:
    QWSSharedMemSurface();
    QWSSharedMemSurface(QWidget *widget);
    ~QWSSharedMemSurface();

    void setGeometry(const QRect &rect);

    QString key() const { return QLatin1String("shm"); }
    QByteArray permanentState() const;

    void setPermanentState(const QByteArray &data);

private:
    bool setMemory(int memId);

    QSharedMemory mem;
};
#endif // QT_NO_QWS_MULTIPROCESS

#ifndef QT_NO_PAINTONSCREEN
class Q_GUI_EXPORT QWSOnScreenSurface : public QWSMemorySurface
{
public:
    QWSOnScreenSurface();
    QWSOnScreenSurface(QWidget *widget);
    ~QWSOnScreenSurface();

    bool isValid() const;
    QPoint painterOffset() const;

    void setGeometry(const QRect &rect);

    QString key() const { return QLatin1String("OnScreen"); }
    QByteArray permanentState() const;

    void setPermanentState(const QByteArray &data);

private:
    void attachToScreen(const QScreen *screen);

    mutable QRect brect;
    const QScreen *screen;
};
#endif // QT_NO_PAINTONSCREEN

#ifndef QT_NO_PAINT_DEBUG
class Q_GUI_EXPORT QWSYellowSurface : public QWSWindowSurface
{
public:
    QWSYellowSurface(bool isClient = false);
    ~QWSYellowSurface();

    void setDelay(int msec) { delay = msec; }

    bool isValid() const { return true; }

    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);

    QString key() const { return QLatin1String("Yellow"); }
    QByteArray permanentState() const;

    void setPermanentState(const QByteArray &data);

    QPaintDevice *paintDevice() { return &img; }
    QImage image() const { return img; }

private:
    int winId;
    int delay;
    QSize surfaceSize; // client side
    QImage img; // server side
};
#endif // QT_NO_PAINT_DEBUG

#ifndef QT_NO_DIRECTPAINTER

class QScreen;

class Q_GUI_EXPORT QWSDirectPainterSurface : public QWSWindowSurface
{
public:
    QWSDirectPainterSurface(bool isClient = false);
    ~QWSDirectPainterSurface();

    void setReserved() { setSurfaceFlags(RegionReserved); }

    void setGeometry(const QRect &rect) { setRegion(rect); }

    void setRegion(const QRegion &region);
    QRegion region() const { return clipRegion(); }

    void flush(QWidget*, const QRegion &, const QPoint &) {};

    bool isValid() const { return false; }

    QString key() const { return QLatin1String("DirectPainter"); }
    QByteArray permanentState() const;

    void setPermanentState(const QByteArray &);

    QImage image() const { return QImage(); }
    QPaintDevice *paintDevice() { return 0; }

    WId windowId() const { return static_cast<WId>(winId); }

    QScreen *screen() const { return _screen; }
private:
    int winId;
    QScreen *_screen;
};

#endif // QT_NO_DIRECTPAINTER

#endif // QWINDOWSURFACE_QWS_P_H
