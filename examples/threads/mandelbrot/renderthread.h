#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>

class QImage;

class RenderThread : public QThread
{
    Q_OBJECT

public:
    RenderThread(QObject *parent = 0);
    ~RenderThread();

    void render(float centerX, float centerY, float scaleFactor,
                QSize resultSize);

signals:
    void finishedRendering(const QImage &image);

protected:
    void run();

private:
    QMutex mutex;
    QWaitCondition condition;
    float centerX;
    float centerY;
    float scaleFactor;
    QSize resultSize;
    bool restart;
    bool abort;
};

#endif
