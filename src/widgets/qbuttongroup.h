/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbuttongroup.h#34 $
**
** Definition of QButtonGroup class
**
** Created : 950130
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

#ifndef QBUTTONGROUP_H
#define QBUTTONGROUP_H

#ifndef QT_H
#include "qgroupbox.h"
#endif // QT_H


class QButton;
class QButtonList;


class Q_EXPORT QButtonGroup : public QGroupBox
{
    Q_OBJECT
public:
    QButtonGroup( QWidget *parent=0, const char *name=0 );
    QButtonGroup( const QString &title, QWidget *parent=0, const char* name=0 );
   ~QButtonGroup();

    bool	isExclusive() const;
    bool	isRadioButtonExclusive() const { return radio_excl; }
    virtual void setExclusive( bool );
    virtual void setRadioButtonExclusive( bool );

    int		insert( QButton *, int id=-1 );
    void	remove( QButton * );
    QButton    *find( int id ) const;
    int		count() const;

    virtual void setButton( int id );

    virtual void moveFocus( int );

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


#endif // QBUTTONGROUP_H
