/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qchkbox.h#3 $
**
** Definition of QCheckBox class
**
** Author  : Haavard Nord
** Created : 940222
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
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
    QCheckBox( const char *label, QWidget *parent, const char *name=0 );

    void    setChecked( bool check );
    bool    isChecked() const { return isOn(); }

    void    resizeFitLabel();

protected:
    void    drawButton( QPainter * );
};


#endif // QCHKBOX_H
