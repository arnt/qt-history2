/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
