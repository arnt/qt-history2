/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.h#8 $
**
** Definition of QSpinBox widget class
**
** Created : 940206
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSPINBOX_H
#define QSPINBOX_H

#include "qframe.h"

class QPushButton;
class QLineEdit;
struct QSpinBoxData;


class QSpinBox : public QFrame
{
    Q_OBJECT
public:
    QSpinBox( QWidget * parent = 0, const char * name = 0 );
    ~QSpinBox();

    virtual void setRange( double bottom, double top, int decimals=0 );

    double current() const;

    virtual void setWrapping( bool w );
    bool wrapping() const { return wrap; }

    QSize sizeHint() const;

public slots:
    virtual void setCurrent( double value );

    virtual void next();
    virtual void prev();

signals:
    void selected( int );
    void selected( double );

protected:
    void keyPressEvent( QKeyEvent * );
    void resizeEvent( QResizeEvent * );

    void enableButtons();

private slots:
    void textChanged();

private:
    struct QSpinBoxData * d;
    bool wrap;
    QPushButton * up;
    QPushButton * down;
    QLineEdit * vi;
};


#endif
