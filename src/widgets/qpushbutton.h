/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbutton.h#54 $
**
** Definition of QPushButton class
**
** Created : 940221
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPUSHBUTTON_H
#define QPUSHBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#include "qiconset.h"
#endif // QT_H

class Q_EXPORT QPushButton : public QButton
{
    Q_OBJECT
    
    Q_PROPERTY( bool autoDefault READ autoDefault WRITE setAutoDefault )
    Q_PROPERTY( bool default READ isDefault WRITE setDefault )
    Q_PROPERTY( bool menuButton READ isMenuButton WRITE setIsMenuButton )
    Q_PROPERTY( QIconSet iconSet READ iconSet WRITE setIconSet )
    
    Q_OVERRIDE( bool toggleButton WRITE setToggleButton )
    Q_OVERRIDE( bool on WRITE setOn )

public:
    QPushButton( QWidget *parent, const char *name=0 );
    QPushButton( const QString &text, QWidget *parent, const char* name=0 );
    QPushButton( const QIconSet& icon, const QString &text, QWidget *parent, const char* name=0 );
    ~QPushButton();

    QSize	sizeHint() const;
    QSizePolicy sizePolicy() const;

    void	move( int x, int y );
    void	move( const QPoint &p );
    void	resize( int w, int h );
    void	resize( const QSize & );
    virtual void setGeometry( int x, int y, int w, int h );

    virtual void setGeometry( const QRect & );

    virtual void setToggleButton( bool ); //### fjern virtual 3.0

    bool	autoDefault()	const	{ return autoDefButton; }
    virtual void setAutoDefault( bool autoDef );
    bool	isDefault()	const	{ return defButton; }
    virtual void setDefault( bool def );

    virtual void setIsMenuButton( bool );
    bool	isMenuButton() const;

    void setPopup( QPopupMenu* popup );
    QPopupMenu* popup() const;

    void setIconSet( const QIconSet& );
    QIconSet* iconSet() const;

public slots:
    virtual void setOn( bool );

    void	toggle();

protected:
    void	drawButton( QPainter * );
    void	drawButtonLabel( QPainter * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	resizeEvent( QResizeEvent * );
    void	updateMask();

private slots:
    void popupPressed();

private:
    void	init();

    uint	autoDefButton	: 1;
    uint	defButton	: 1;
    uint	lastDown		: 1;
    uint	reserved		: 1;
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
