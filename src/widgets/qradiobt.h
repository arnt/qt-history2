/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobt.h#9 $
**
** Definition of QRadioButton class
**
** Author  : Haavard Nord
** Created : 940222
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
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
    QRadioButton( const char *text, QWidget *parent=0, const char *name=0 );

    bool    isChecked() const { return isOn(); }
    void    setChecked( bool check );

    void    adjustSize();

protected:
    bool    hitButton( const QPoint & ) const;
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );

    void    mouseReleaseEvent( QMouseEvent * );

private:
    uint    noHit : 1;
};


#endif // QRADIOBT_H
