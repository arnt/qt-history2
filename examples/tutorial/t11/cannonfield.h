/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************
**
** Definition of CannonField class, Qt tutorial 11
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

private slots:
    void moveShot();

signals:
    void angleChanged(int newAngle);
    void forceChanged(int newForce);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void paintShot(QPainter &painter);
    void paintCannon(QPainter &painter);
    QRect cannonRect() const;
    QRect shotRect() const;

    int ang;
    int f;

    int timerCount;
    QTimer *autoShootTimer;
    float shoot_ang;
    float shoot_f;
};

#endif // CANNON_H
