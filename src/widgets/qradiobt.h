/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobt.h#4 $
**
** Definition of QRadioButton class
**
** Author  : Haavard Nord
** Created : 940222
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QRADIOBT_H
#define QRADIOBT_H

#include "qbutton.h"


class QRadioButton : public QButton
{
    Q_OBJECT
public:
    QRadioButton( QWidget *parent=0, const char *name=0 );
    QRadioButton( const char *label, QWidget *parent=0, const char *name=0 );

    void    setChecked( bool check );
    bool    isChecked() const { return isOn(); }

    void    resizeFitLabel();

protected:
    bool    hitButton( const QPoint & ) const;
    void    drawButton( QPainter * );

private:
    void    mouseReleaseEvent( QMouseEvent * );
    uint    noHit : 1;
};


#endif // QRADIOBT_H
