/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qgrid.h#15 $
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

#ifndef QGRID_H
#define QGRID_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

class QGridLayout;

class Q_EXPORT QGrid : public QFrame
{
    Q_OBJECT
public:
    enum Direction { Horizontal, Vertical };
    QGrid( int n, QWidget *parent=0, const char *name=0, WFlags f=0 );
    QGrid( int n, Direction, QWidget *parent=0, const char *name=0,
	   WFlags f=0 );
    void setSpacing( int );
    QSize sizeHint() const;

protected:
    void frameChanged();

private:
    QGridLayout *lay;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGrid( const QGrid & );
    QGrid& operator=( const QGrid & );
#endif
};

#endif //QGRID_H
