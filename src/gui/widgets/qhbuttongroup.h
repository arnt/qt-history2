/****************************************************************************
**
** Definition of QHButtonGroup class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QHBUTTONGROUP_H
#define QHBUTTONGROUP_H

#ifndef QT_H
#include "qbuttongroup.h"
#endif // QT_H

#ifndef QT_NO_HBUTTONGROUP

class Q_GUI_EXPORT QHButtonGroup : public QButtonGroup
{
    Q_OBJECT
public:
    QHButtonGroup( QWidget* parent=0, const char* name=0 );
    QHButtonGroup( const QString &title, QWidget* parent=0, const char* name=0 );
    ~QHButtonGroup();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QHButtonGroup( const QHButtonGroup & );
    QHButtonGroup &operator=( const QHButtonGroup & );
#endif
};


#endif // QT_NO_HBUTTONGROUP

#endif // QHBUTTONGROUP_H
