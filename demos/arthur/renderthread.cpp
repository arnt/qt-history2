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

#include "renderthread.h"

#include <qapplication.h>
#include <qimage.h>
#include <math.h>
#include <qcolor.h>
#include <qdebug.h>

RenderThread::RenderThread()
{
    parameters = 0;
    abort = false;
}

RenderThread::~RenderThread()
{
    abort = true;
    wait();
}

#define COLORMAP_SIZE 512
uint color_map[COLORMAP_SIZE];

uint wavelength_to_rgb(float wl)
{
    float r = 0.0, g = 0.0, b = 0.0;

    if (wl >= 380.0 && wl <= 440.0) {
        r = -1.0 * (wl - 440.0) / (440.0 - 380.0);
        b = 1.0;
    } else if (wl >= 440.0 && wl <= 490.0) {
        g = (wl - 440.0) / (490.0 - 440.0);
        b = 1.0;
    } else if (wl >= 490.0 && wl <= 510.0) {
        g = 1.0;
        b = -1.0 * (wl - 510.0) / (510.0 - 490.0);
    } else if (wl >= 510.0 && wl <= 580.0) {
        r = (wl - 510.0) / (580.0 - 510.0);
        g = 1.0;
    } else if (wl >= 580.0 && wl <= 645.0) {
        r = 1.0;
        g = -1.0 * (wl - 645.0) / (645.0 - 580.0);
    } else if (wl >= 645.0 && wl <= 780.0) {
        r = 1.0;
    }

    float s = 1.0;
    if (wl > 700.0)
        s = 0.3 + 0.7 * (780.0 - wl) / (780.0 - 700.0);
    else if (wl <  420.0)
        s = 0.3 + 0.7 * (wl - 380.0) / (420.0 - 380.0);

    r = pow(r * s, 0.8);
    g = pow(g * s, 0.8);
    b = pow(b * s, 0.8);
    return (uchar(r*255.0) << 16) | (uchar(g*255) << 8) | uchar(b*255);
}

void RenderThread::run()
{
    static bool init_cmap = true;
    if (init_cmap) {
        init_cmap = false;
        for (int i=0; i<COLORMAP_SIZE; ++i)
            color_map[i] = wavelength_to_rgb(380.0+400.0*i/COLORMAP_SIZE);
    }

    for (;;) {
        short limit = 4;
        int lp;
        float a1, a2, b1, b2;
        int x, y;
        float ax, ay;

        mutex.lock();
        RenderParameters *p = parameters;
        parameters = 0;

        int pheight = p->height;
        int pwidth = p->width;
        float pscale = p->scale;
        float pcx = p->cx;
        float pcy = p->cy;
        //QObject *preceiver = p->receiver;

        delete p;

        mutex.unlock();

        int halfWidth = pwidth / 2;
        int halfHeight = pheight / 2;
        QImage image(pwidth, pheight, 32);

        int passCount = 32;
        for (int pass = 0; pass < passCount; ++pass) {
            const int maxPrecision = (pass + 3) * 30;

            for (y = -halfHeight; y < halfHeight; ++y) {
                if (parameters)
                    break;
                if (abort)
                    return;

                QRgb *scanLine = (QRgb *) image.scanLine(y + halfHeight);
                for (x = -halfWidth; x < halfWidth; ++x) {

                    ax = pcx + x * pscale;
                    ay = pcy + y * pscale;
                    a1 = ax;
                    b1 = ay;
                    lp = 0;

                    do {
                        ++lp;
                        a2 = a1 * a1 - b1 * b1 + ax;
                        b2 = 2 * a1 * b1 + ay;
                        a1 = a2;
                        b1 = b2;
                    } while (!(lp > maxPrecision || ((a1*a1) + (b1*b1) > limit)));

                    if (lp > maxPrecision)
                        *scanLine++ = 0xff000000;
                    else
                        *scanLine++ = color_map[lp%COLORMAP_SIZE];
                }
            }

            if (!parameters) {
                emit renderingDone(image);
            } else {
                break;
            }
        }

        if (!parameters)
            break;
    }
}

void RenderThread::startRendering(QObject *receiver, double cx, double cy,
                                  double scale, int width, int height)
{
    QMutexLocker locker(&mutex);

    if (!parameters)
        parameters = new RenderParameters;

    parameters->receiver = receiver;
    parameters->cx = cx;
    parameters->cy = cy;
    parameters->scale = scale;
    parameters->width = width;
    parameters->height = height;

    if (!isRunning())
        start(LowPriority);
}
