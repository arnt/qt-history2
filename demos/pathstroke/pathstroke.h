#ifndef PATHSTROKE_H

#include <QtGui>
#include "arthurwidgets.h"

class PathStrokeRenderer : public ArthurFrame
{
    Q_OBJECT
public:
    enum PathMode { CurveMode, LineMode };

    PathStrokeRenderer(QWidget *parent);

    void paint(QPainter *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void timerEvent(QTimerEvent *e);

    QSize sizeHint() const { return QSize(500, 500); }

public slots:
    void setPenWidth(int penWidth) { m_penWidth = penWidth / 10.0; update(); }
    void setAnimation(bool animation);

    void setFlatCap() { m_capStyle = Qt::FlatCap; update(); }
    void setSquareCap() { m_capStyle = Qt::SquareCap; update(); }
    void setRoundCap() { m_capStyle = Qt::RoundCap; update(); }

    void setBevelJoin() { m_joinStyle = Qt::BevelJoin; update(); }
    void setMiterJoin() { m_joinStyle = Qt::MiterJoin; update(); }
    void setRoundJoin() { m_joinStyle = Qt::RoundJoin; update(); }

    void setCurveMode() { m_pathMode = CurveMode; update(); }
    void setLineMode() { m_pathMode = LineMode; update(); }

    void setSolidLine() { m_penStyle = Qt::SolidLine; update(); }
    void setDashLine() { m_penStyle = Qt::DashLine; update(); }
    void setDotLine() { m_penStyle = Qt::DotLine; update(); }
    void setDashDotLine() { m_penStyle = Qt::DashDotLine; update(); }
    void setDashDotDotLine() { m_penStyle = Qt::DashDotDotLine; update(); }
    void setCustomDashLine() { m_penStyle = Qt::NoPen; update(); }

private:
    void initializePoints();
    void updatePoints();

    QBasicTimer m_timer;

    PathMode m_pathMode;

    bool m_wasAnimated;

    double m_penWidth;
    int m_pointCount;
    int m_pointSize;
    int m_activePoint;
    QVector<QPointF> m_points;
    QVector<QPointF> m_vectors;

    Qt::PenJoinStyle m_joinStyle;
    Qt::PenCapStyle m_capStyle;

    Qt::PenStyle m_penStyle;
};

class PathStrokeWidget : public QWidget
{
    Q_OBJECT
public:
    PathStrokeWidget();

private:
    PathStrokeRenderer *m_renderer;
};

#endif // PATHSTROKE_H
