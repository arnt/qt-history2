/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.h#44 $
**
** Definition of QLabel widget class
**
** Created : 941215
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#ifndef QLABEL_H
#define QLABEL_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

class QLabelPrivate;

class Q_EXPORT QLabel : public QFrame
{
    Q_OBJECT
public:
    QLabel( QWidget *parent=0, const char *name=0, WFlags f=0 );
    QLabel( const QString &text, QWidget *parent=0, const char *name=0,
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
    virtual void setMargin( int );

    bool	autoResize()	const	{ return autoresize; }
    virtual void setAutoResize( bool );
    QSize	sizeHint() const;
    QSizePolicy sizePolicy() const;

    virtual void setBuddy( QWidget * );
    QWidget    *buddy() const;

    void setAutoMask(bool);

public slots:
    virtual void	setText( const QString &);
    virtual void	setPixmap( const QPixmap & );
    virtual void	setMovie( const QMovie & );
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
    QString	ltext;
    QPixmap    *lpixmap;
    QWidget *	lbuddy;
    QMovie *	lmovie;
    ushort	align;
    int		extraMargin:8;
    uint	autoresize:1;
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
