/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CHECKLISTS_H
#define CHECKLISTS_H

#include <qwidget.h>

class Q3ListView;
class QLabel;

class CheckLists : public QWidget
{
    Q_OBJECT

public:
    CheckLists( QWidget *parent = 0, const char *name = 0 );

protected:
    Q3ListView *lv1, *lv2;
    QLabel *label;

protected slots:
    void copy1to2();
    void copy2to3();

};

#endif
