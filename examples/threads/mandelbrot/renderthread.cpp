#include <QtGui>

#include "renderthread.h"

static const int ColormapSize = 512;
static uint colormap[ColormapSize];

static uint rgbFromWaveLength(float wave)
{
    float r = 0.0;
    float g = 0.0;
    float b = 0.0;

    if (wave >= 380.0 && wave <= 440.0) {
        r = -1.0 * (wave - 440.0) / (440.0 - 380.0);
        b = 1.0;
    } else if (wave >= 440.0 && wave <= 490.0) {
        g = (wave - 440.0) / (490.0 - 440.0);
        b = 1.0;
    } else if (wave >= 490.0 && wave <= 510.0) {
        g = 1.0;
        b = -1.0 * (wave - 510.0) / (510.0 - 490.0);
    } else if (wave >= 510.0 && wave <= 580.0) {
        r = (wave - 510.0) / (580.0 - 510.0);
        g = 1.0;
    } else if (wave >= 580.0 && wave <= 645.0) {
        r = 1.0;
        g = -1.0 * (wave - 645.0) / (645.0 - 580.0);
    } else if (wave >= 645.0 && wave <= 780.0) {
        r = 1.0;
    }

    float s = 1.0;
    if (wave > 700.0)
        s = 0.3 + 0.7 * (780.0 - wave) / (780.0 - 700.0);
    else if (wave <  420.0)
        s = 0.3 + 0.7 * (wave - 380.0) / (420.0 - 380.0);

    r = pow(r * s, 0.8);
    g = pow(g * s, 0.8);
    b = pow(b * s, 0.8);
    return qRgb(int(r * 255), int(g * 255), int(b * 255));
}

RenderThread::RenderThread(QObject *parent)
    : QThread(parent)
{
    parameters = 0;
    abort = false;

    for (int i = 0; i < ColormapSize; ++i)
        colormap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / ColormapSize));
}

RenderThread::~RenderThread()
{
    abort = true;
    wait();
}

void RenderThread::run()
{
    forever {
        short limit = 4;
        int lp;
        float a1, a2, b1, b2;
        int x, y;
        float ax, ay;

        mutex.lock();
        QSize resultSize = parameters->resultSize;
        float scaleFactor = parameters->scaleFactor;
        float centerX = parameters->centerX;
        float centerY = parameters->centerY;
        delete parameters;
        parameters = 0;
        mutex.unlock();

        int halfWidth = resultSize.width() / 2;
        int halfHeight = resultSize.height() / 2;
        QImage image(resultSize, 32);

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
                    ax = centerX + (x * scaleFactor);
                    ay = centerY + (y * scaleFactor);
                    a1 = ax;
                    b1 = ay;
                    lp = 0;

                    do {
                        ++lp;
                        a2 = a1 * a1 - b1 * b1 + ax;
                        b2 = 2 * a1 * b1 + ay;
                        a1 = a2;
                        b1 = b2;
                    } while (lp <= maxPrecision
                             && (a1 * a1) + (b1 * b1) < limit);

                    if (lp > maxPrecision)
                        *scanLine++ = qRgb(0, 0, 0);
                    else
                        *scanLine++ = colormap[lp % ColormapSize];
                }
            }

            if (!parameters) {
                emit finishedRendering(image);
            } else {
                break;
            }
        }

        if (!parameters)
            break;
    }
}

void RenderThread::render(float centerX, float centerY, float scaleFactor,
                          QSize resultSize)
{
    QMutexLocker locker(&mutex);

    if (!parameters)
        parameters = new RenderParameters;

    parameters->centerX = centerX;
    parameters->centerY = centerY;
    parameters->scaleFactor = scaleFactor;
    parameters->resultSize = resultSize;

    if (!isRunning())
        start(LowPriority);
}
