/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.h#6 $
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
struct QSpinBoxData;


class QSpinBox : public QFrame
{
    Q_OBJECT
public:
    QSpinBox( QWidget * parent = 0, const char * name = 0 );
    ~QSpinBox();

    virtual void append( const char * );
    virtual void append( const char ** );
    virtual void clear();

    virtual const char * text( int index ) const;
    int current() const { return c; }
    virtual int count() const;

    void setWrapping( bool w );
    bool wrapping() const { return wrap; }

    QSize sizeHint() const;

public slots:
    virtual void setCurrent( int );

    void next();
    void previous();

signals:
    void selected( const char * );

protected:
    void drawContents( QPainter * );
    void keyPressEvent( QKeyEvent * );
    void resizeEvent( QResizeEvent * );

    void enableButtons();

private:
    int c;
    QStrList * l;
    struct QSpinBoxData * d;
    bool wrap;
    QPushButton * up;
    QPushButton * down;
};


#endif
