/**********************************************************************
** Copyright (C) 2000-2003 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Assistant.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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
