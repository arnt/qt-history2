/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.h#10 $
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
class QValidator;
struct QSpinBoxData;


class QSpinBox: public QFrame
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
    void selected( double );

protected:
    bool eventFilter( QObject *, QEvent * );
    void resizeEvent( QResizeEvent * );

    void setValidator( QValidator * );
    QPushButton * upButton();
    QPushButton * downButton();
    QLineEdit * editor();

protected slots:
    virtual void textChanged();

private:
    void enableButtons();

    struct QSpinBoxData * d;
    QPushButton * up;
    QPushButton * down;
    QLineEdit * vi;
    uint wrap: 1;
};


#endif
