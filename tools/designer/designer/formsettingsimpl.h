/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef FORMSETTINGSIMPL_H
#define FORMSETTINGSIMPL_H

#include "formsettings.h"

class FormWindow;

class FormSettings : public FormSettingsBase
{
    Q_OBJECT

public:
    FormSettings( QWidget *parent, FormWindow *fw );

protected slots:
    void okClicked();

private:
    FormWindow *formwindow;

};

#endif
