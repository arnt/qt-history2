/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbuttongroup.h#17 $
**
** Definition of QButtonGroup class
**
** Created : 950130
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBTTNGRP_H
#define QBTTNGRP_H

#include "qgrpbox.h"


class QButton;
class QButtonList;


class QButtonGroup : public QGroupBox
{
    Q_OBJECT
public:
    QButtonGroup( QWidget *parent=0, const char *name=0 );
    QButtonGroup( const char *title, QWidget *parent=0, const char *name=0 );
   ~QButtonGroup();

    bool	isExclusive() const;
    void	setExclusive( bool );

    int		insert( QButton *, int id=-1 );
    void	remove( QButton * );
    QButton    *find( int id ) const;

signals:
    void	pressed( int id );
    void	released( int id );
    void	clicked( int id );

protected slots:
    void	buttonPressed();
    void	buttonReleased();
    void	buttonClicked();

private:
    void	init();
    bool	excl_grp;
    QButtonList *buttons;

private:	// Disabled copy constructor and operator=
    QButtonGroup( const QButtonGroup & );
    QButtonGroup &operator=( const QButtonGroup & );
};


#endif // QBTTNGRP_H
