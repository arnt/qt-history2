/****************************************************************************
**
** Definition of QVButtonGroup class.
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

#ifndef QVBUTTONGROUP_H
#define QVBUTTONGROUP_H

#ifndef QT_H
#include "qbuttongroup.h"
#endif // QT_H

#ifndef QT_NO_VBUTTONGROUP

class Q_GUI_EXPORT QVButtonGroup : public QButtonGroup
{
    Q_OBJECT
public:
    QVButtonGroup( QWidget* parent=0, const char* name=0 );
    QVButtonGroup( const QString &title, QWidget* parent=0, const char* name=0 );

    ~QVButtonGroup();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QVButtonGroup( const QVButtonGroup & );
    QVButtonGroup &operator=( const QVButtonGroup & );
#endif
};


#endif // QT_NO_VBUTTONGROUP

#endif // QVBUTTONGROUP_H
