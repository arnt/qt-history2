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

#ifndef BUTTONS_GROUPS_H
#define BUTTONS_GROUPS_H

#include <qwidget.h>

class QCheckBox;
class QRadioButton;

class ButtonsGroups : public QWidget
{
    Q_OBJECT

public:
    ButtonsGroups( QWidget *parent = 0, const char *name = 0 );

protected:
    QCheckBox *state;
    QRadioButton *rb21, *rb22, *rb23;

protected slots:    
    void slotChangeGrp3State();

};

#endif
