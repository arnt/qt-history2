/****************************************************************
**
** Definition of CannonField class, Qt tutorial 12
**
****************************************************************/

#ifndef CANNON_H
#define CANNON_H

#include <QWidget>

class QTimer;

class CannonField : public QWidget
{
    Q_OBJECT

public:
    CannonField(QWidget *parent = 0);

    int angle() const { return ang; }
    int force() const { return f; }

public slots:
    void setAngle(int angle);
    void setForce(int force);
    void shoot();
    void newTarget();

private slots:
    void moveShot();

signals:
    void hit();
    void missed();
    void angleChanged(int newAngle);
    void forceChanged(int newForce);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void paintShot(QPainter &painter);
    void paintTarget(QPainter &painter);
    void paintCannon(QPainter &painter);
    QRect cannonRect() const;
    QRect shotRect() const;
    QRect targetRect() const;

    int ang;
    int f;

    int timerCount;
    QTimer *autoShootTimer;
    float shoot_ang;
    float shoot_f;

    QPoint target;
};

#endif // CANNON_H
