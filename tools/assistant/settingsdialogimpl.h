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

#include "settingsdialog.h"

#include <qstringlist.h>
#include <qlistview.h>

class SettingsDialog : public SettingsDialogBase
{
    Q_OBJECT

public:
    SettingsDialog( QWidget *parent, const char* name = 0 );

protected slots:
    void selectColor();
    void browseWebApp();
    void browsePDFApplication();
    void browseHomepage();
    void accept();
    void reject();

private:
    void init();
    void setFile( QLineEdit *le, const QString &caption );
};

#endif
