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

#ifndef LINEDITS_H
#define LINEDITS_H

#include <qgroupbox.h>

class QLineEdit;
class QComboBox;

class LineEdits : public QGroupBox
{
    Q_OBJECT

public:
    LineEdits( QWidget *parent = 0, const char *name = 0 );

protected:
    QLineEdit *lined1, *lined2, *lined3, *lined4, *lined5;
    QComboBox *combo1, *combo2, *combo3, *combo4, *combo5;

protected slots:
    void slotEchoChanged( int );
    void slotValidatorChanged( int );
    void slotAlignmentChanged( int );
    void slotInputMaskChanged( int );
    void slotReadOnlyChanged( int );
};

#endif
