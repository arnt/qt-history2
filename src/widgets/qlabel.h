/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.h#26 $
**
** Definition of QLabel widget class
**
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
    QLabel( QWidget *parent=0, const char *name=0, WFlags f=0 );
    QLabel( const char *text, QWidget *parent=0, const char *name=0,
	    WFlags f=0 );
   ~QLabel();

    const char *text()		const	{ return ltext; }
    QPixmap    *pixmap()	const	{ return lpixmap; }

    int		alignment()	const	{ return align; }
    void	setAlignment( int );
    int		margin()	const	{ return extraMargin; }
    void	setMargin( int );

    bool	autoResize()	const	{ return autoresize; }
    void	setAutoResize( bool );
    QSize	sizeHint() const;

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
    int		extraMargin;
    int		align;
    bool	autoresize;

private:	// Disabled copy constructor and operator=
    QLabel( const QLabel & ) {}
    QLabel &operator=( const QLabel & ) { return *this; }
};


#endif // QLABEL_H
