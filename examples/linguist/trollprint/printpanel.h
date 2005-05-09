/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PRINTPANEL_H
#define PRINTPANEL_H

#include <QWidget>

class QGroupBox;
class QRadioButton;

class PrintPanel : public QWidget
{
    Q_OBJECT

public:
    PrintPanel(QWidget *parent = 0);

private:
    QGroupBox *twoSidedGroupBox;
    QGroupBox *colorsGroupBox;
    QRadioButton *twoSidedEnabledRadio;
    QRadioButton *twoSidedDisabledRadio;
    QRadioButton *colorsEnabledRadio;
    QRadioButton *colorsDisabledRadio;
};

#endif
