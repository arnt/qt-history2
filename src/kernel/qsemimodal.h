/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsemimodal.h#9 $
**
** Definition of QSemiModal class
**
** Created : 970627
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

#ifndef QSEMIMODAL_H
#define QSEMIMODAL_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_SEMIMODAL
class Q_EXPORT QSemiModal : public QWidget
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
    virtual void	setGeometry( int x, int y, int w, int h );
    virtual void	setGeometry( const QRect & );

private:
    uint	did_move   : 1;
    uint	did_resize : 1;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSemiModal( const QSemiModal & );
    QSemiModal &operator=( const QSemiModal & );
#endif
};
#endif

#endif // QSEMIMODAL_H
