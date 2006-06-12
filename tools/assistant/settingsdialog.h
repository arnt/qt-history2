/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "ui_settingsdialog.h"

#include <QDialog>
#include <QStringList>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent);

    inline QTabWidget *settingsTab() const
    { return ui.settingsTab; }


private slots:
    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();
    void on_buttonHome_clicked();

    void init();

private:
    void setFile(QLineEdit *le, const QString &caption);

private:
    Ui::SettingsDialog ui;
};

#endif // SETTINGSDIALOG_H
