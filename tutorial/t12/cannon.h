/****************************************************************************
** Definition of CannonField class, Qt tutorial 12
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef CANNON_H
#define CANNON_H

#include <qscrbar.h>
#include <qlcdnum.h>


class CannonField : public QWidget
{
    Q_OBJECT
public:
    CannonField( QWidget *parent=0, const char *name=0 );

    int  angle() const { return ang; }
    int  force() const { return f; }
public slots:
    void  setAngle( int degrees );
    void  setForce( int newton );
    void  shoot();
signals:
    void hit();
    void missed();
protected:
    void  timerEvent( QTimerEvent * );
    void  paintEvent( QPaintEvent * );
private:
    void  stopShooting();
    void  paintCannon();
    void  paintShot();
    void  paintTarget();
    QRect shotRect() const;
    QRect targetRect() const;
    void  newTarget();

    int   ang;
    int   f;
    bool  shooting;
    int   timerCount;

    float shoot_ang;
    float shoot_f;

    QPoint target;
};

#endif // CANNON_H
