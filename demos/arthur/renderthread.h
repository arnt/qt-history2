#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H
#include <qevent.h>
#include <qimage.h>
#include <qthread.h>
#include <qmutex.h>

class RenderThread : public QThread
{
    Q_OBJECT
public:
    RenderThread();
    ~RenderThread();

    void run();
    void startRendering(QObject *receiver, double cx, double cy, double scale, int width, int height);

signals:
    void renderingDone(const QImage &img);

private:
    QMutex mutex;
    struct RenderParameters {
        QObject *receiver;
        double cx;
        double cy;
        double scale;
        int width;
        int height;
        bool abort;
    };

    RenderParameters *parameters;
    bool abort;
};

#endif
