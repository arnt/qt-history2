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

#ifndef HYBRIDSURFACE_H
#define HYBRIDSURFACE_H

#include <private/qglwindowsurface_qws_p.h>
#include <private/qglpaintdevice_qws_p.h>
#include <GLES/egl.h>
#include <vanilla/eglVanilla.h>

class HybridPaintDevice;
class HybridSurfacePrivate;
class QWSLock;

class HybridSurface : public QWSGLWindowSurface
{
public:
    HybridSurface();
    HybridSurface(QWidget *w, EGLDisplay display);
    ~HybridSurface();

    void beginPaint(const QRegion &region);
    bool lock(int timeout);
    void unlock();

    bool isValid() const;
    void setGeometry(const QRect &rect, const QRegion &mask);
    QString key() const { return QLatin1String("hybrid"); }

    QByteArray permanentState() const;
    void setPermanentState(const QByteArray &state);

    QImage image() const;
    QPaintDevice *paintDevice();
    QPoint painterOffset() const;

private:
    QSharedMemory mem;
    QImage img;
    QWSLock *memlock;
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;
    QWSGLPaintDevice *pdevice;

    VanillaPixmap vanillaPix;
};

#endif // HYBRIDSURFACE_H
