/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcheckbox.h#19 $
**
** Definition of QCheckBox class
**
** Created : 940222
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QCHKBOX_H
#define QCHKBOX_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H


class QCheckBox : public QButton
{
    Q_OBJECT
public:
    QCheckBox( QWidget *parent=0, const char *name=0 );
    QCheckBox( const char *text, QWidget *parent, const char *name=0 );

    bool    isChecked() const;
    void    setChecked( bool check );

    QSize sizeHint() const;

protected:
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );

private:	// Disabled copy constructor and operator=
    QCheckBox( const QCheckBox & );
    QCheckBox &operator=( const QCheckBox & );
};


inline bool QCheckBox::isChecked() const
{ return isOn(); }

inline void QCheckBox::setChecked( bool check )
{ setOn( check ); }


#endif // QCHKBOX_H
