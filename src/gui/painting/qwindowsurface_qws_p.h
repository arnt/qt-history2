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

#include "qwindowsurface_p.h"
#include <qregion.h>
#include <qimage.h>
#include <private/qsharedmemory_p.h>

class QWSWindowSurfacePrivate;

class QWSWindowSurface : public QWindowSurface
{
public:
    QWSWindowSurface();
    QWSWindowSurface(QWidget *window);
    ~QWSWindowSurface();

    virtual void resize(const QSize &size);
    virtual void flush(QWidget *widget, const QRegion &region,
                       const QPoint &offset);
    virtual void release();
    virtual bool isValidFor(QWidget *widget) const = 0;

    virtual const QString key() const = 0;
    virtual const QByteArray data() const = 0;

    QWidget *window() const;
    const QRegion dirtyRegion() const;
    void setDirty(const QRegion &) const;
    const QRegion clipRegion() const;
    void setClipRegion(const QRegion &);

    virtual bool attach(const QByteArray &data) = 0;
    virtual void detach() = 0;
    virtual const QImage image() const = 0;

#if 0
    virtual bool create(QWidget *window) = 0;

    enum SurfaceFlag {
        Reserved = 0x0,
        Buffered = 0x1,
        Opaque = 0x2
    };
    Q_DECLARE_FLAGS(SurfaceFlags, SurfaceFlag);

    SurfaceFlags surfaceFlags() const;
    bool isBuffered() const { return surfaceFlags() & Buffered; }
    bool isReserved() const { return surfaceFlags() == Reserved; }

protected:
    void setSurfaceFlags(SurfaceFlags type);
#else
    virtual bool isBuffered() const { return !image().isNull(); }
    virtual bool isReserved() const { return false; }
#endif

private:
    QWSWindowSurfacePrivate *d_ptr;
};

class QWSWindowSurfaceFactory
{
public:
    static QWSWindowSurface* create(QWidget *widget);
    static QWSWindowSurface* create(const QString &key,
                                    const QByteArray &data);
};

class QWSLocalMemWindowSurface : public QWSWindowSurface
{
public:
    QWSLocalMemWindowSurface(QWidget *widget = 0);
    ~QWSLocalMemWindowSurface();

    QPaintDevice *paintDevice() { return &img; }

    void resize(const QSize &size);
    void release();
    bool isValidFor(QWidget *widget) const;

    void scroll(const QRegion &area, int dx, int dy);
    QSize size() const { return img.size(); };
    const QString key() const { return QLatin1String("mem"); }
    const QByteArray data() const;

    bool attach(const QByteArray &data);
    void detach();
    const QImage image() const { return img; };

protected:
    QImage::Format preferredImageFormat(const QWidget *widget) const;

    uchar *mem;
    int memsize;
    QImage img;
};

class QWSLock;

class QWSSharedMemWindowSurface : public QWSLocalMemWindowSurface
{
public:
    QWSSharedMemWindowSurface(QWidget *widget = 0);
    ~QWSSharedMemWindowSurface();

    QPaintDevice *paintDevice() { return &img; }
    void beginPaint(const QRegion &);
    void endPaint(const QRegion &);

    void resize(const QSize &size);
    void release();
    void scroll(const QRegion &area, int dx, int dy);
    QSize size() const { return img.size(); };
    const QString key() const { return QLatin1String("shm"); }
    const QByteArray data() const;

    bool attach(const QByteArray &data);
    void detach();

private:
    bool setMemory(int memId);
    bool setLock(int lockId);

    QSharedMemory mem;
    QWSLock *memlock;
};

class QWSYellowSurface : public QWSWindowSurface
{
public:
    QWSYellowSurface();
    QWSYellowSurface(QWidget *);
    ~QWSYellowSurface();

    void setDelay(int msec) { delay = msec; }

    void resize(const QSize &size);
    void scroll(const QRegion &, int, int) {}
    QSize size() const { return img.size(); };
    bool isValidFor(QWidget *) const { return true; }

    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);

    const QString key() const { return QLatin1String("YellowThing"); }
    const QByteArray data() const;

    bool attach(const QByteArray &data);
    void detach();
    void blit(const QRegion &, const QPoint &) {}

    QPaintDevice *paintDevice() { return &img; }
    const QImage image() const { return img; }

private:
    int winId;
    int delay;
    QSize surfaceSize; // client side
    QImage img; // server side
};

class QWSDirectPainterSurface : public QWSWindowSurface
{
public:
    QWSDirectPainterSurface();
    QWSDirectPainterSurface(QWidget *);
    ~QWSDirectPainterSurface();

    void resize(const QSize &size) { resize(QRect(QPoint(0, 0), size)); }
    void resize(const QRegion &region);
    QSize size() const { return clipRegion().boundingRect().size(); }

    void flush(QWidget*, const QRegion &, const QPoint &) {};
    void scroll(const QRegion &, int, int) {};

    bool isValidFor(QWidget*) const { return false; }

    const QString key() const { return QLatin1String("DirectPainter"); }
    const QByteArray data() const { return QByteArray(); }

    bool attach(const QByteArray &) { return true; }
    void detach() {}
    void release();

    const QImage image() const { return QImage(); }
    QPaintDevice *paintDevice() { return 0; }

    bool isBuffered() const { return false; }
    bool isReserved() const { return true; }

private:
    int winId;
};

#endif // QWINDOWSURFACE_QWS_P_H
