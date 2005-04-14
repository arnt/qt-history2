/****************************************************************
**
** Definition of CannonField class, Qt tutorial 14
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
    bool gameOver() const { return gameEnded; }
    bool isShooting() const;
    QSize sizeHint() const;

public slots:
    void setAngle(int angle);
    void setForce(int force);
    void shoot();
    void newTarget();
    void setGameOver();
    void restartGame();

private slots:
    void moveShot();

signals:
    void hit();
    void missed();
    void angleChanged(int newAngle);
    void forceChanged(int newForce);
    void canShoot(bool can);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void paintShot(QPainter &painter);
    void paintTarget(QPainter &painter);
    void paintBarrier(QPainter &painter);
    void paintCannon(QPainter &painter);
    QRect cannonRect() const;
    QRect shotRect() const;
    QRect targetRect() const;
    QRect barrierRect() const;
    bool barrelHit(const QPoint &pos) const;

    int ang;
    int f;

    int timerCount;
    QTimer *autoShootTimer;
    float shoot_ang;
    float shoot_f;

    QPoint target;

    bool gameEnded;
    bool barrelPressed;
};

#endif // CANNON_H
