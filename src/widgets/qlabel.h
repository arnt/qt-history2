/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.h#17 $
**
** Definition of QLabel widget class
**
** Author  : Eirik Eng
** Created : 941215
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
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
   ~QLabel();

    const char *text()		const	{ return ltext; }
    QPixmap    *pixmap()	const	{ return lpixmap; }

    int		alignment()	const	{ return align; }
    void	setAlignment( int );

    bool	autoResize()	const	{ return autoresize; }
    void	setAutoResize( bool );
    void	adjustSize();

public slots:
    void	setText( const char * );
    void	setPixmap( const QPixmap & );
    void	setNum( int );
    void	setNum( double );

protected:
    void	drawContents( QPainter * );

private:
    void	updateLabel();
    QString	ltext;
    QPixmap    *lpixmap;
    int		align;
    bool	autoresize;
};


#endif // QLABEL_H
