/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qchkbox.h#1 $
**
** Definition of QCheckBox class
**
** Author  : Haavard Nord
** Created : 940222
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QCHKBOX_H
#define QCHKBOX_H

#include "qbutton.h"


class QCheckBox : public QButton
{
    Q_OBJECT
public:
    QCheckBox( QView *parent=0, const char *label = 0 );
    QCheckBox( QView *parent, const QRect &r, const char *label = 0 );

    void    setChecked( bool check );
    bool    isChecked() const { return isOn(); }

protected:
    void    drawButton( QPainter * );
};


#endif // QCHKBOX_H
