/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Assistant.
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

#include "settingsdialog.h"

#include <qstringlist.h>
#include <qptrlist.h>
#include <qlistview.h>

class ProfileCheckItem : public QCheckListItem
{
public:
    ProfileCheckItem( QListView *parent, const QString &name );
    ProfileCheckItem( ProfileCheckItem *parent, const QString& pN );
    int rtti() const;
    static int RTTI;
    void activate();
    QString profileName() const;
private:
    QString profName;
};


class SettingsDialog : public SettingsDialogBase
{
    Q_OBJECT

public:
    SettingsDialog( QWidget *parent, const char* name = 0 );
    void setCurrentProfile();

protected slots:
    void selectColor();
    void addProfile();
    void removeProfile();
    void modifyProfile();
    void browseWebApp();
    void browsePDFApplication();
    void browseHomepage();
    void accept();
    void reject();

signals:
    void profileChanged();

private:
    void init();
    void setFile( QLineEdit *le, const QString &caption );
    ProfileCheckItem* currentCheckedProfile();
    void setupProfiles();
    bool profileAttributesChanged;
    QStringList deleteProfilesList;
    QString oldProfile, newProfile;
};

#endif
