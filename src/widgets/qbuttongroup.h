/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbuttongroup.h#40 $
**
** Definition of QButtonGroup class
**
** Created : 950130
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the widgets
** module and therefore may only be used if the widgets module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QBUTTONGROUP_H
#define QBUTTONGROUP_H

#ifndef QT_H
#include "qgroupbox.h"
#endif // QT_H

#ifndef QT_NO_COMPLEXWIDGETS


class QButton;
class QButtonList;


class Q_EXPORT QButtonGroup : public QGroupBox
{
    Q_OBJECT
    Q_PROPERTY( bool exclusive READ isExclusive WRITE setExclusive )
    Q_PROPERTY( bool radioButtonExclusive READ isRadioButtonExclusive WRITE setRadioButtonExclusive )
	
public:
    QButtonGroup( QWidget *parent=0, const char *name=0 );
    QButtonGroup( const QString &title,
		  QWidget *parent=0, const char* name=0 );
    QButtonGroup( int columns, Orientation o,
		  QWidget *parent=0, const char *name=0 );
    QButtonGroup( int columns, Orientation o, const QString &title,
		  QWidget *parent=0, const char* name=0 );
   ~QButtonGroup();

    bool	isExclusive() const;
    bool	isRadioButtonExclusive() const { return radio_excl; }
    virtual void setExclusive( bool );
    virtual void setRadioButtonExclusive( bool );

public:
    int		insert( QButton *, int id=-1 );
    void	remove( QButton * );
    QButton    *find( int id ) const;
    int		id( QButton * ) const;
    int		count() const;

    virtual void setButton( int id );

    virtual void moveFocus( int );

    QButton    *selected();

signals:
    void	pressed( int id );
    void	released( int id );
    void	clicked( int id );

protected slots:
    void	buttonPressed();
    void	buttonReleased();
    void	buttonClicked();
    void	buttonToggled( bool on );

private:
    void	init();
    bool	excl_grp;
    bool	radio_excl;
    QButtonList *buttons;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QButtonGroup( const QButtonGroup & );
    QButtonGroup &operator=( const QButtonGroup & );
#endif
};


#endif // QT_NO_COMPLEXWIDGETS

#endif // QBUTTONGROUP_H
