/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbutton.h#47 $
**
** Definition of QPushButton class
**
** Created : 940221
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

#ifndef QPUSHBUTTON_H
#define QPUSHBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H


class Q_EXPORT QPushButton : public QButton
{
    Q_OBJECT
public:
    QPushButton( QWidget *parent, const char *name=0 );
    QPushButton( const QString &text, QWidget *parent, const char* name=0 );

    virtual void	setToggleButton( bool );

    bool	autoDefault()	const	{ return autoDefButton; }
    virtual void	setAutoDefault( bool autoDef );
    bool	isDefault()	const	{ return defButton; }
    virtual void	setDefault( bool def );

    virtual void	setIsMenuButton( bool );
    bool	isMenuButton() const;

    QSize	sizeHint() const;
    QSizePolicy sizePolicy() const;

    void	move( int x, int y );
    void	move( const QPoint &p );
    void	resize( int w, int h );
    void	resize( const QSize & );
    virtual void	setGeometry( int x, int y, int w, int h );
    virtual void	setGeometry( const QRect & );

public slots:
    virtual void	setOn( bool );
    void	toggle();

protected:
    void	drawButton( QPainter * );
    void	drawButtonLabel( QPainter * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	resizeEvent( QResizeEvent * );
    void updateMask();

private:
    void	init();

    uint	autoDefButton	: 1;
    uint	defButton	: 1;
    uint	lastDown	: 1;
    uint	lastDef		: 1;
    uint	lastEnabled	: 1;
    uint	hasMenuArrow	: 1;

    friend class QDialog;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPushButton( const QPushButton & );
    QPushButton &operator=( const QPushButton & );
#endif
};


#endif // QPUSHBUTTON_H
