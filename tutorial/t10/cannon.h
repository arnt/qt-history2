/****************************************************************
**
** Definition of CannonField class, Qt tutorial 10
**
****************************************************************/

#ifndef CANNON_H
#define CANNON_H

#include <QWidget>

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

signals:
    void angleChanged(int newAngle);
    void forceChanged(int newForce);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QRect cannonRect() const;

    int ang;
    int f;
};

#endif // CANNON_H
