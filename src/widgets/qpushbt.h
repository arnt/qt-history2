/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbt.h#29 $
**
** Definition of QPushButton class
**
** Created : 940221
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPUSHBT_H
#define QPUSHBT_H

#include "qbutton.h"


class QPushButton : public QButton
{
friend class QDialog;
    Q_OBJECT
public:
    QPushButton( QWidget *parent=0, const char *name=0 );
    QPushButton( const char *text, QWidget *parent=0, const char *name=0 );

    void	setToggleButton( bool );

    bool	autoDefault()	const	{ return autoDefButton; }
    void	setAutoDefault( bool autoDef );
    bool	isDefault()	const	{ return defButton; }
    void	setDefault( bool def );

    QSize	sizeHint() const;

  // Reimplemented move,resize and setGeometry
    void	move( int x, int y );
    void	move( const QPoint &p );
    void	resize( int w, int h );
    void	resize( const QSize & );
    void	setGeometry( int x, int y, int w, int h );
    void	setGeometry( const QRect & );

public slots:
    void	setOn( bool );
    void	toggle();

protected:
    void	drawButton( QPainter * );
    void	drawButtonLabel( QPainter * );
    void	focusInEvent( QFocusEvent * );

private:
    void	init();

    uint	autoDefButton	: 1;
    uint	defButton	: 1;
    uint	lastDown	: 1;
    uint	lastDef		: 1;
    uint	lastEnabled	: 1;

private:	// Disabled copy constructor and operator=
    QPushButton( const QPushButton & ) {}
    QPushButton &operator=( const QPushButton & ) { return *this; }
};


#endif // QPUSHBT_H
