/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGRID_H
#define QGRID_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

#ifndef QT_NO_GRID

class QGridLayout;

class Q_GUI_EXPORT QGrid : public QFrame
{
    Q_OBJECT
public:
    QGrid( int n, QWidget* parent=0, const char* name=0, WFlags f = 0 );
    QGrid( int n, Orientation orient, QWidget* parent=0, const char* name=0,
	   WFlags f = 0 );

    void setSpacing( int );

#ifndef QT_NO_COMPAT
    typedef Orientation Direction;
#endif

protected:
    void frameChanged();
    void childEvent(QChildEvent*);
private:
    QGridLayout *lay;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGrid( const QGrid & );
    QGrid& operator=( const QGrid & );
#endif
};

#endif // QT_NO_GRID

#endif // QGRID_H
