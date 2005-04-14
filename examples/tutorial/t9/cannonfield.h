/****************************************************************
**
** Definition of CannonField class, Qt tutorial 9
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

public slots:
    void setAngle(int angle);

signals:
    void angleChanged(int newAngle);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int ang;
};

#endif // CANNON_H
