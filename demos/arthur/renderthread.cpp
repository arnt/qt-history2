#include <qapplication.h>
#include <qimage.h>
#include <math.h>
#include <qcolor.h>
#include <qdebug.h>
#include "renderthread.h"

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

void RenderThread::run()
{
    for (;;) {
        short limit = 4;
        int lp;
        double a1, a2, b1, b2;
        int x, y;
        double ax, ay;

        mutex.lock();
        RenderParameters *p = parameters;
        parameters = 0;

        int pheight = p->height;
        int pwidth = p->width;
        double pscale = p->scale;
        double pcx = p->cx;
        double pcy = p->cy;
        //QObject *preceiver = p->receiver;

        delete p;

        mutex.unlock();

        int halfWidth = pwidth / 2;
        int halfHeight = pheight / 2;
        QImage image(pwidth, pheight, 32);

        int passCount = 8;
        for (int pass = 0; pass < passCount; ++pass) {
            const int maxPrecision = (pass + 3) * 100;

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

                    QColor c;
                    if (lp > maxPrecision) {
                        c = Qt::black;
                    } else {
                        int h = (lp / 360) & 1 ? 359 - (lp % 360) : lp % 360;
                        int v = ((lp / 256) & 1) ? 255 - (lp & 255) : (lp & 255);
                        int s = 255 - v;
                        c.setHsv(h, s, v);
                    }

                    *scanLine++ = c.rgb();
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

