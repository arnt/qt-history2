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
    restart = false;
    abort = false;

    for (int i = 0; i < ColormapSize; ++i)
        colormap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / ColormapSize));
}

RenderThread::~RenderThread()
{
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void RenderThread::run()
{
    forever {
        mutex.lock();
        QSize resultSize = this->resultSize;
        float scaleFactor = this->scaleFactor;
        float centerX = this->centerX;
        float centerY = this->centerY;
        mutex.unlock();

        int halfWidth = resultSize.width() / 2;
        int halfHeight = resultSize.height() / 2;
        QImage image(resultSize, 32);

        const int MaxPrecision = 256;
        int limit = 4;

        for (int y = -halfHeight; y < halfHeight; ++y) {
            if (restart)
                break;
            if (abort)
                return;

            QRgb *scanLine =
                   reinterpret_cast<QRgb *>(image.scanLine(y + halfHeight));
            for (int x = -halfWidth; x < halfWidth; ++x) {
                float ax = centerX + (x * scaleFactor);
                float ay = centerY + (y * scaleFactor);
                float a1 = ax;
                float b1 = ay;
                int numPasses = 0;

                do {
                    ++numPasses;
                    float a2 = a1 * a1 - b1 * b1 + ax;
                    float b2 = 2 * a1 * b1 + ay;
                    a1 = a2;
                    b1 = b2;
                } while (numPasses <= MaxPrecision
                         && (a1 * a1) + (b1 * b1) <= limit);

                if (numPasses > MaxPrecision)
                    *scanLine++ = qRgb(0, 0, 0);
                else
                    *scanLine++ = colormap[numPasses % ColormapSize];
            }
            emit finishedRendering(image);
        }

        mutex.lock();
        if (!restart)
            condition.wait(&mutex);
        mutex.unlock();
    }
}

void RenderThread::render(float centerX, float centerY, float scaleFactor,
                          QSize resultSize)
{
    QMutexLocker locker(&mutex);

    this->centerX = centerX;
    this->centerY = centerY;
    this->scaleFactor = scaleFactor;
    this->resultSize = resultSize;

    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeOne();
    }
}
