/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.h#1 $
**
** Definition of QLabel class
**
** Author  : Eirik Eng
** Created : 941215
**
** Copyright (c) 1994 by Troll Tech AS.	 All rights reserved.
**
***********************************************************************/

#ifndef QLABEL_H
#define QLABEL_H

#include "qwidget.h"


class QLabel : public QWidget
{
    Q_OBJECT
public:
    QLabel( QWidget *parent=0, const char *name=0 );
   ~QLabel();

    void    setText( const char * );
    char   *text() const;

protected:
    void    paintEvent( QPaintEvent * );
    void    resizeEvent( QResizeEvent * );

private:
    QString t;
};


#endif // QLINED_H
