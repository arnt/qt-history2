/****************************************************************
**
** Definition of Cannonfield class, Qt tutorial 8
**
****************************************************************/

#ifndef CANNON_H
#define CANNON_H

#include <qwidget.h>


class CannonField : public QWidget
{
    Q_OBJECT
public:
    CannonField( QWidget *parent=0, const char *name=0 );

    int angle() const { return ang; }
public slots:
    void setAngle( int degrees );
protected:
    void paintEvent( QPaintEvent * );
private:
    int ang;
};

#endif // CANNON_H
