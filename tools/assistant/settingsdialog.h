/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
#include <qlistview.h>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent, const char* name = 0);

    inline QComboBox *fontCombo() const
    { return gui.fontCombo; }

    inline QComboBox *fixedFontCombo() const
    { return gui.fixedfontCombo; }

    inline QCheckBox *linkUnderlineCB() const
    { return gui.linkUnderlineCB; }

    inline QToolButton *colorButton() const
    { return gui.colorButton; }

    inline QTabWidget *settingsTab() const
    { return gui.settingsTab; }

protected slots:
    void init();
    void selectColor();
    void browseWebApp();
    void browsePDFApplication();
    void browseHomepage();
    void accept();
    void reject();

private:
    void setFile(QLineEdit *le, const QString &caption);

private:
    Ui::SettingsDialog gui;
};

#endif
