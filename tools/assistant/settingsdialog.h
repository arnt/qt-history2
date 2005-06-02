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

#ifndef SETTINGSDIALOGIMPL_H
#define SETTINGSDIALOGIMPL_H

#include "ui_settingsdialog.h"

#include <qdialog.h>
#include <qstringlist.h>

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
    void on_buttonBrowse_clicked();
    void on_buttonHome_clicked();
    void on_buttonPDF_clicked();

    void init();
    void accept();
    void reject();

private:
    void setFile(QLineEdit *le, const QString &caption);

private:
    Ui::SettingsDialog ui;
};

#endif
