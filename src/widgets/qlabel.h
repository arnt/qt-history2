/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.h#4 $
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
    QLabel( const char *text, QWidget *parent=0, const char *name=0 );

    void    setText( const char * );
    void    setText( int );
    void    setText( long );
    void    setText( float );
    void    setText( double );
    char   *text() const;

protected:
    void    paintEvent( QPaintEvent * );

private:
    QString t;
};

inline void QLabel::setText( int   i ) { setText( (long)  i ); }

inline void QLabel::setText( float f ) { setText( (double)f ); }


#endif // QLABEL_H
