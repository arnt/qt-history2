/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qdial.h#2 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QDIAL_H
#define QDIAL_H

#ifndef QT_H
#include "qwidget.h"
#include "qrangecontrol.h"
#endif // QT_H

//class QTimer;
class QDialPrivate;

class Q_EXPORT QDial: public QWidget, public QRangeControl
{
    Q_OBJECT
public:
    QDial( QWidget *parent=0, const char *name=0 );
    QDial( int minValue, int maxValue, int pageStep, int value,
	   QWidget *parent=0, const char *name=0 );

    virtual void setTracking( bool enable );
    bool tracking() const;

    QSizePolicy sizePolicy() const;

    virtual void setWrapping( bool on );
    bool wrapping() const;

    int notchSize() const;
    virtual void setNotchTarget( double );
    double notchTarget() const;

public slots:
    virtual void setValue( int );
    void addStep();
    void subtractStep();
    void addPage();
    void subtractPage();

signals:
    void valueChanged( int value );
    void dialPressed();
    void dialMoved( int value );
    void dialReleased();

protected:
    void resizeEvent( QResizeEvent * );
    void paintEvent( QPaintEvent * );

    void keyPressEvent( QKeyEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void wheelEvent( QWheelEvent * );

    void valueChange();
    void rangeChange();

private:
    QDialPrivate * d;

    int valueFromPoint( const QPoint & ) const;
    
private: // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDial( const QDial & );
    QDial &operator=( const QDial & );
#endif

};

#endif
