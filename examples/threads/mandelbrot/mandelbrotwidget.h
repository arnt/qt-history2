#ifndef MANDELBROTWIDGET_H
#define MANDELBROTWIDGET_H

#include <QPixmap>
#include <QWidget>

#include "renderthread.h"

class MandelbrotWidget : public QWidget
{
    Q_OBJECT

public:
    MandelbrotWidget(QWidget *parent = 0);

    QSize sizeHint() const;

public slots:
    void drawRenderedImage(const QImage &image);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    void zoom(double zoomFactor);
    void scroll(int deltaX, int deltaY);

    double centerX;
    double centerY;
    double prevScale;
    double curScale;

    QPoint lastDragPos;
    QPoint pixmapDrawPoint;
    QPixmap pixmap;
    RenderThread thread;
};

#endif
