/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobutton.h#13 $
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

    bool    isChecked() const;
    void    setChecked( bool check );

    void    adjustSize();

protected:
    bool    hitButton( const QPoint & ) const;
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );

    void    mouseReleaseEvent( QMouseEvent * );

private:
    void    init();
    uint    noHit : 1;

private:	// Disabled copy constructor and operator=
    QRadioButton( const QRadioButton & ) {}
    QRadioButton &operator=( const QRadioButton & ) { return *this; }
};


inline bool QRadioButton::isChecked() const
{ return isOn(); }

inline void QRadioButton::setChecked( bool check )
{ setOn( check ); }


#endif // QRADIOBT_H

