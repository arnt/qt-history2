#ifndef MANDELBROTWIDGET_H
#define MANDELBROTWIDGET_H
#include "demowidget.h"
#include <qwidget.h>
#include <qpixmap.h>

#include "renderthread.h"

class MandelbrotWidget : public DemoWidget
{
    Q_OBJECT
public:
    MandelbrotWidget(QWidget *parent = 0);

    QSize sizeHint() const;

    enum Zoom { ZoomIn, ZoomOut };
    void zoom(Zoom z);

    void startAnimation() { resetState(); }
    void stopAnimation() {}

public slots:
    void drawRenderedImage(const QImage *image);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *event);

    double cx;
    double cy;
    double scale;
    double lastScale;

    QPoint moveAnchor;

    QPoint pixmapDrawPoint;

    QPixmap pixmap;

    RenderThread renderThread;
};


#endif
