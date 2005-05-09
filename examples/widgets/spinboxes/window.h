/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QDateTimeEdit;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

public slots:
    void changePrecision(int decimals);
    void setFormatString(const QString &formatString);

private:
    void createSpinBoxes();
    void createDateTimeEdits();
    void createDoubleSpinBoxes();

    QDateTimeEdit *meetingEdit;
    QDoubleSpinBox *doubleSpinBox;
    QDoubleSpinBox *priceSpinBox;
    QDoubleSpinBox *scaleSpinBox;
    QGroupBox *spinBoxesGroup;
    QGroupBox *editsGroup;
    QGroupBox *doubleSpinBoxesGroup;
    QLabel *meetingLabel;
};

#endif
