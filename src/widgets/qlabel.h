/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.h#50 $
**
** Definition of QLabel widget class
**
** Created : 941215
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QLABEL_H
#define QLABEL_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

class QMLSimpleDocument;
class QLabelPrivate;

class Q_EXPORT QLabel : public QFrame
{
    Q_OBJECT
public:
    QLabel( QWidget *parent, const char *name=0, WFlags f=0 );
    QLabel( const QString &text, QWidget *parent, const char *name=0,
	    WFlags f=0 );
    QLabel( QWidget * buddy, const QString &,
	    QWidget * parent, const char * name=0, WFlags f=0 );
   ~QLabel();

    QString	text()		const	{ return ltext; }
    QPixmap    *pixmap()	const	{ return lpixmap; }
    QMovie     *movie()		const;

    int		alignment()	const	{ return align; }
    virtual void setAlignment( int );
    int		margin()	const	{ return extraMargin; }
    void setMargin( int );

    bool	autoResize()	const	{ return autoresize; }
    virtual void setAutoResize( bool );
    QSize	sizeHint() const;
    QSizePolicy sizePolicy() const;

    virtual void setBuddy( QWidget * );
    QWidget    *buddy() const;

    void setAutoMask(bool);

    int heightForWidth(int) const;

public slots:
    virtual void	setText( const QString &);
    virtual void	setPixmap( const QPixmap & );
    virtual void	setMovie( const QMovie & );
    virtual void	setQML( const QString & );
    virtual void	setNum( int );
    virtual void	setNum( double );
    void	clear();

protected:
    void	drawContents( QPainter * );
    void	drawContentsMask( QPainter * );

private slots:
    void	acceleratorSlot();
    void	buddyDied();
    void	movieUpdated(const QRect&);
    void	movieResized(const QSize&);

private:
    void init();
    void	updateLabel();
    QSize	sizeForWidth( int w ) const;
    QString	ltext;
    QPixmap    *lpixmap;
    QMovie *	lmovie;
    QWidget *	lbuddy;
    ushort	align;
    int		extraMargin:8;
    uint	autoresize:1;
    uint	isqml:1;
    QMLSimpleDocument* qmlDoc;
    QAccel *	accel;  // NON NULL
    QLabelPrivate* d;

    void	unsetMovie();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLabel( const QLabel & );
    QLabel &operator=( const QLabel & );
#endif
};


#endif // QLABEL_H
