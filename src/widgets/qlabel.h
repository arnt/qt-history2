/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.h#58 $
**
** Definition of QLabel widget class
**
** Created : 941215
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QLABEL_H
#define QLABEL_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

#ifndef QT_NO_COMPLEXWIDGETS

class QSimpleRichText;
class QLabelPrivate;

class Q_EXPORT QLabel : public QFrame
{
    Q_OBJECT
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( TextFormat textFormat READ textFormat WRITE setTextFormat )
    Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap )
    Q_PROPERTY( bool scaledContents READ hasScaledContents WRITE setScaledContents )
    Q_PROPERTY( Alignment alignment READ alignment WRITE setAlignment )
    Q_PROPERTY( int indent READ indent WRITE setIndent )
	
public:
    QLabel( QWidget *parent, const char *name=0, WFlags f=0 );
    QLabel( const QString &text, QWidget *parent, const char *name=0,
	    WFlags f=0 );
    QLabel( QWidget * buddy, const QString &,
	    QWidget * parent, const char * name=0, WFlags f=0 );
   ~QLabel();

    QString	 text()		const	{ return ltext; }
    QPixmap     *pixmap()	const	{ return lpixmap; }
#ifndef QT_NO_MOVIE
    QMovie      *movie()		const;
#endif

    TextFormat textFormat() const;
    void 	 setTextFormat( TextFormat );

    int		 alignment() const	{ return align; }
    virtual void setAlignment( int );
    int		 indent() const		{ return extraMargin; }
    void 	 setIndent( int );

    bool 	 autoResize() const	{ return autoresize; }
    virtual void setAutoResize( bool );
    
    bool 	hasScaledContents() const;
    void 	setScaledContents( bool );

    QSize	 sizeHint() const;
    QSize	 minimumSizeHint() const;
    QSizePolicy  sizePolicy() const;

    virtual void setBuddy( QWidget * );
    QWidget     *buddy() const;

    void	 setAutoMask(bool);

    int		 heightForWidth(int) const;

public slots:
    virtual void setText( const QString &);
    virtual void setPixmap( const QPixmap & );
#ifndef QT_NO_MOVIE
    virtual void setMovie( const QMovie & );
#endif
    virtual void setNum( int );
    virtual void setNum( double );
    void	 clear();

protected:
    void	 drawContents( QPainter * );
    void	 drawContentsMask( QPainter * );
    void	 fontChange( const QFont & );
    void	 resizeEvent( QResizeEvent* );

private slots:
    void	 acceleratorSlot();
    void	 buddyDied();
#ifndef QT_NO_MOVIE
    void	 movieUpdated(const QRect&);
    void	 movieResized(const QSize&);
#endif

private:
    void	init();
    void	clearContents();
    void	updateLabel( QSize oldSizeHint );
    QSize	sizeForWidth( int w ) const;
    QString	ltext;
    QPixmap    *lpixmap;
#ifndef QT_NO_MOVIE
    QMovie *	lmovie;
#endif
    QWidget *	lbuddy;
    ushort	align;
    short	extraMargin;
    uint	autoresize:1;
    uint	scaledcontents :1;
    TextFormat textformat;
    QSimpleRichText* doc;
    QAccel *	accel;
    QLabelPrivate* d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLabel( const QLabel & );
    QLabel &operator=( const QLabel & );
#endif
};


#endif // QT_NO_COMPLEXWIDGETS

#endif // QLABEL_H
