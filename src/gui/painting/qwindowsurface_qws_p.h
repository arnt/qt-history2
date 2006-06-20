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
#include <qsize.h>
#include <qimage.h>

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

    virtual bool attach(const QByteArray &data) = 0;
    virtual void detach() = 0;
    virtual const QImage image() const = 0;

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

#endif // QWINDOWSURFACE_QWS_P_H
