/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef SETTINGSDIALOGIMPL_H
#define SETTINGSDIALOGIMPL_H

#include <qstringlist.h>
#include <qptrlist.h>
#include <qlistview.h>
#include "settingsdialog.h"

class SettingsDialog : public SettingsDialogBase
{
    Q_OBJECT

public:
    SettingsDialog( QWidget *parent, const char* name = 0 );

protected slots:
    void selectColor();
    void addPath();
    void deletePath();
    void addCategory();
    void deleteCategory();
    void browseWebApp();
    void accept();
    void reject();
signals:
    void changedPath();
    void changedCategory();
private:
    void init();
    bool newFilesExist( QString dir );
    bool changed;
    QStringList pathlist, catlistavail, catlistsel;
    QPtrList<QCheckListItem> catitems;
};

#endif
