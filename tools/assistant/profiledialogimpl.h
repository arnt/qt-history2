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

#ifndef PROFILEDIALOGIMPL_H
#define PROFILEDIALOGIMPL_H

#include "profiledialog.h"
#include "profile.h"

class ProfileDialog : public ProfileDialogBase
{
    Q_OBJECT

public:
    enum Mode { Add, Modify };
    ProfileDialog( QWidget *parent, QString pN = QString::null );
    ~ProfileDialog();
    bool profileChanged() const;

private slots:
    void okClicked();
    void cancelClicked();
    void chooseProfileIcon();
    void addDocFile();
    void removeDocFile();
    void saveProfileInFile();
    void setUrl();
    void setPath();
    void setHome();

private:
    void initDialog();
    void checkForChanges();
    QString iconAbsFilePath( const QString &docFileName,
			     const QString &iconName );
    void insertProfileData();
    void updatePaths();
    QString profName, profFile;
    Profile *oldProfile, *profile;
    bool changed;
    Mode mode;
    QStringList removedDocFiles;
};

#endif
