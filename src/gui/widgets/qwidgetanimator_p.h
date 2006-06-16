#ifndef QWIDGET_ANIMATOR_P_H
#define QWIDGET_ANIMATOR_P_H

#include <QObject>
#include <QRect>
#include <QMap>

class QWidget;
class QTimer;
class QTime;

class QWidgetAnimator : public QObject
{
    Q_OBJECT
public:
    QWidgetAnimator(QObject *parent = 0);
    ~QWidgetAnimator();
    void animate(QWidget *widget, const QRect &final_geometry, bool animate);
    bool animating() const;
    bool animating(QWidget *widget);

signals:
    void finished(QWidget *widget);
    void finishedAll();

private slots:
    void animationStep();

private:
    struct AnimationItem {
        AnimationItem(QWidget *_widget = 0, const QRect &_r1 = QRect(),
                        const QRect &_r2 = QRect())
            : widget(_widget), r1(_r1), r2(_r2), step(0) {}
        QWidget *widget;
        QRect r1, r2;
        int step;
    };
    typedef QMap<QWidget*, AnimationItem> AnimationMap;
    AnimationMap m_animation_map;
    QTimer *m_timer;
    QTime *m_time;
};

#endif // QWIDGET_ANIMATOR_P_H
