/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.h#1 $
**
** Definition of QSpinBox widget class
**
** Created : 940206
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSPINBT_H
#define QSPINBT_H

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

    void setWrapping( bool wrap ) { w = wrap ? 1 : 0; }
    bool wrapping() const { return (bool)w; }

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

    void doResize( const QSize & );

private:
    int c;
    QStrList * l;
    struct QSpinBoxData * d;
    uint w : 1;
    QPushButton * up;
    QPushButton * down;
};


#endif
