/****************************************************************************
**
** Definition of QHGroupBox widget class.
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

#ifndef QHGROUPBOX_H
#define QHGROUPBOX_H

#ifndef QT_H
#include "qgroupbox.h"
#endif // QT_H

#ifndef QT_NO_HGROUPBOX

class Q_GUI_EXPORT QHGroupBox : public QGroupBox
{
    Q_OBJECT
public:
    QHGroupBox( QWidget* parent=0, const char* name=0 );
    QHGroupBox( const QString &title, QWidget* parent=0, const char* name=0 );
    ~QHGroupBox();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QHGroupBox( const QHGroupBox & );
    QHGroupBox &operator=( const QHGroupBox & );
#endif
};

#endif // QT_NO_HGROUPBOX

#endif // QHGROUPBOX_H
