/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.h#7 $
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

#include "qframe.h"


class QLabel : public QFrame
{
    Q_OBJECT
public:
    QLabel( QWidget *parent=0, const char *name=0 );
    QLabel( const char *label, QWidget *parent=0, const char *name=0 );

    const char *label() const	{ return (const char *)str; }

    void    setLabel( const char * );
    void    setLabel( int );
    void    setLabel( long );
    void    setLabel( float );
    void    setLabel( double );

    int	    alignment() const { return align; }
    void    setAlignment( int );

protected:
    void    drawContents( QPainter * );

private:
    void    updateLabel();
    QString str;
    int	    align;
};

inline void QLabel::setLabel( int   i ) { setLabel( (long)  i ); }

inline void QLabel::setLabel( float f ) { setLabel( (double)f ); }


#endif // QLABEL_H
