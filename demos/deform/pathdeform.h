#ifndef PATHDEFORM_H
#define PATHDEFORM_H

#include "arthurwidgets.h"

#include <qevent.h>
#include <qpainterpath.h>
#include <qbasictimer.h>
#include <qdatetime.h>

class PathDeformRenderer : public ArthurFrame
{
    Q_OBJECT
public:
    PathDeformRenderer(QWidget *widget);

    void paint(QPainter *painter);

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void timerEvent(QTimerEvent *e);

    QSize sizeHint() const { return QSize(600, 500); }

public slots:
    void setRadius(int radius);
    void setFontSize(int fontSize) { m_fontSize = fontSize; setText(m_text); }
    void setText(const QString &text);
    void setIntensity(int intensity);

    void setAnimated(bool animated);

// signals:
//     void frameRate(double fps);

private:
    void generateLensPixmap();
    QPainterPath lensDeform(const QPainterPath &source, const QPointF &offset);

    QBasicTimer m_repaintTimer;
//     QBasicTimer m_fpsTimer;
//     int m_fpsCounter;
    QTime m_repaintTracker;

    QVector<QPainterPath> m_paths;
    QVector<QPointF> m_advances;
    QRectF m_pathBounds;
    QString m_text;

    QPixmap m_pixmap;

    int m_fontSize;
    bool m_animated;

    double m_intensity;
    double m_radius;
    QPointF m_pos;
    QPointF m_offset;
    QPointF m_direction;
};

class PathDeformWidget : public QWidget
{
public:
    PathDeformWidget(QWidget *parent);

private:
    PathDeformRenderer *m_renderer;
};

#endif // PATHDEFORM_H
