/****************************************************************************
** $Id: //depot/qt/main/tests/moc-bug/spin.h#1 $
**
** Definition of 
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
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
