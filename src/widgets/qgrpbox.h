/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qgrpbox.h#1 $
**
** Definition of QGroupBox widget class
**
** Author  : Haavard Nord
** Created : 950203
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
***********************************************************************/

#ifndef QGRPBOX_H
#define QGRPBOX_H

#include "qframe.h"


class QGroupBox : public QFrame
{
    Q_OBJECT
public:
    QGroupBox( QWidget *parent=0, const char *name=0 );
    QGroupBox( const char *title, QWidget *parent=0, const char *name=0 );

    const char *title() const	{ return (const char *)str; }

    void    setTitle( const char * );

    int	    alignment() const	{ return align; }
    void    setAlignment( int );

protected:
    void    paintEvent( QPaintEvent * );

private:
    void    updateLabel();
    QString str;
    int	    align;
};


#endif // QGRPBOX_H
