/****************************************************************************
**
** Definition of .
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SPIN_H
#define SPIN_H

#include <qwidget.h>

class MainParent : public QWidget {
    Q_OBJECT
public:
    MainParent( QWidget* parent = 0, const char* name = 0, int f = 0 );
};


#endif
