/****************************************************************************
** Definition of CannonField class, Qt tutorial 10
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

    int angle() const { return ang; }
    int force() const { return f; }
public slots:
    void setAngle( int degrees );
    void setForce( int newton );
protected:
    void paintEvent( QPaintEvent * );
private:
    void paintCannon();

    int ang;
    int f;
};

#endif // CANNON_H
