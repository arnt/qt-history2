/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qgroupbox.h#13 $
**
** Definition of QGroupBox widget class
**
** Created : 950203
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QGROUPBOX_H
#define QGROUPBOX_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H


class QGroupBox : public QFrame
{
    Q_OBJECT
public:
    QGroupBox( QWidget *parent=0, const char *name=0 );
    QGroupBox( const char *title, QWidget *parent=0, const char *name=0 );

    const char *title()		const	{ return str; }

    void	setTitle( const char * );

    int		alignment()	const	{ return align; }
    void	setAlignment( int );

protected:
    void	paintEvent( QPaintEvent * );

private:
    void	init();
    QString	str;
    int		align;

private:	// Disabled copy constructor and operator=
    QGroupBox( const QGroupBox & );
    QGroupBox &operator=( const QGroupBox & );
};


#endif // QGROUPBOX_H
