/****************************************************************************
** $Id: //depot/qt/main/tests/slider/slider.h#1 $
**
** Definition of 
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef SLIDER_H
#define SLIDER_H

#include "qwidget.h"

class MainParent : public QWidget {
    Q_OBJECT
public:
    MainParent( QWidget* parent = 0, const char* name = 0, int f = 0 );
};

#endif
