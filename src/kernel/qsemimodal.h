/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsemimodal.h#2 $
**
** Definition of QSemiModal class
**
** Created : 970627
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSEMIMODAL_H
#define QSEMIMODAL_H

#include "qwidget.h"


class QSemiModal : public QWidget
{
    Q_OBJECT
public:
    QSemiModal( QWidget *parent=0, const char *name=0, bool modal=FALSE, WFlags f=0 );
   ~QSemiModal();

    void	show();

    void	move( int x, int y );
    void	move( const QPoint &p );
    void	resize( int w, int h );
    void	resize( const QSize & );
    void	setGeometry( int x, int y, int w, int h );
    void	setGeometry( const QRect & );

private:
    uint	did_move   : 1;
    uint	did_resize : 1;

private:	// Disabled copy constructor and operator=
    QSemiModal( const QSemiModal & );
    QSemiModal &operator=( const QSemiModal & );
};


#endif // QSEMIMODAL_H
