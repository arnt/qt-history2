/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.h#10 $
**
** Definition of QLabel widget class
**
** Author  : Eirik Eng
** Created : 941215
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
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
    QLabel( const char *text, QWidget *parent=0, const char *name=0 );

    const char *text()		const	{ return (const char *)str; }
    void	setText( const char * );
    void	setNum( int );
    void	setNum( long );
    void	setNum( float );
    void	setNum( double );

    int		alignment()	const	{ return align; }
    void	setAlignment( int );

    void	setAutoResizing( bool );
    bool	autoResizing()	const	{ return autoResize; }
    void	adjustSize();

protected:
    void	drawContents( QPainter * );

private:
    void	updateLabel();
    QString	str;
    int		align;
    bool	autoResize;
};


inline void QLabel::setNum( int i )
{
    setNum( (long)i );
}

inline void QLabel::setNum( float f )
{
    setNum( (double)f );
}


#endif // QLABEL_H
