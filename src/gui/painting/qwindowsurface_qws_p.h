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
    ~QWSWindowSurface();

    virtual bool create(QWidget *window);
    virtual void release();

    virtual bool isValidFor(QWidget *widget) const = 0;

    virtual void resize(const QSize &size);
    virtual void flush(QWidget *widget, const QRegion &region,
                       const QPoint &offset);

    virtual const QString key() const = 0;
    virtual const QByteArray data() const = 0;

    virtual bool attach(const QByteArray &data) = 0;
    virtual void detach() = 0;

    virtual const QImage image() const = 0;

    QWidget *window() const;

    const QRegion dirtyRegion() const;
    void setDirty(const QRegion &) const;

    const QRegion clipRegion() const;
    void setClipRegion(const QRegion &);

    enum SurfaceFlag {
        Reserved = 0x0,
        Buffered = 0x1,
        Opaque = 0x2
    };
    Q_DECLARE_FLAGS(SurfaceFlags, SurfaceFlag)

    SurfaceFlags surfaceFlags() const;

    inline bool isReserved() const { return surfaceFlags() == Reserved; }
    inline bool isBuffered() const { return surfaceFlags() & Buffered; }
    inline bool isOpaque() const { return !isBuffered() || surfaceFlags() & Opaque; }

protected:
    void setSurfaceFlags(SurfaceFlags type);

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
    QWSLocalMemWindowSurface();
    ~QWSLocalMemWindowSurface();

    bool create(QWidget *widget);
    void release();

    void resize(const QSize &size);

    const QString key() const { return QLatin1String("mem"); }
    const QByteArray data() const;

    bool isValidFor(QWidget *widget) const;

    QPaintDevice *paintDevice() { return &img; }

    void scroll(const QRegion &area, int dx, int dy);
    QSize size() const { return img.size(); };

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
    QWSSharedMemWindowSurface();
    ~QWSSharedMemWindowSurface();

    bool create(QWidget *widget);
    void release();

    QPaintDevice *paintDevice() { return &img; }
    void beginPaint(const QRegion &);
    void endPaint(const QRegion &);

    void resize(const QSize &size);
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
    ~QWSYellowSurface();

    bool create(QWidget *widget);

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
    ~QWSDirectPainterSurface();

    bool create(QWidget *);
    void release();

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

    const QImage image() const { return QImage(); }
    QPaintDevice *paintDevice() { return 0; }

private:
    int winId;
};

#endif // QWINDOWSURFACE_QWS_P_H
