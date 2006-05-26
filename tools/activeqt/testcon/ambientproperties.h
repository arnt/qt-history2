/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef AMBIENTPROPERTIES_H
#define AMBIENTPROPERTIES_H

#include "ui_ambientproperties.h"

class AmbientProperties : public QDialog, Ui::AmbientProperties
{
    Q_OBJECT
public:
    AmbientProperties(QWidget *parent);

    void setControl(QWidget *widget);

public slots:
    void on_buttonBackground_clicked();
    void on_buttonForeground_clicked();
    void on_buttonFont_clicked();
    void on_buttonEnabled_toggled(bool on);

private:
    QWidget *container;
};

#endif // AMBIENTPROPERTIES_H
