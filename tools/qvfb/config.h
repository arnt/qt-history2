/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include "ui_config.h"

class Config : public QDialog, public Ui::Config
{
    Q_OBJECT
public:
    Config(QWidget *parent = 0, Qt::WFlags flags = 0)
        : QDialog(parent, flags)
    {
        setupUi(this);
    }

private slots:
    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();
};

#endif // CONFIG_H
