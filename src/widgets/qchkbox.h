/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qchkbox.h#6 $
**
** Definition of QCheckBox class
**
** Author  : Haavard Nord
** Created : 940222
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QCHKBOX_H
#define QCHKBOX_H

#include "qbutton.h"


class QCheckBox : public QButton
{
    Q_OBJECT
public:
    QCheckBox( QWidget *parent=0, const char *name=0 );
    QCheckBox( const char *text, QWidget *parent, const char *name=0 );

    bool    isChecked() const { return isOn(); }
    void    setChecked( bool check );

    void    adjustSize();

protected:
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );
};


#endif // QCHKBOX_H
