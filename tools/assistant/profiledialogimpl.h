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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
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
    void setProfileIcon();
    void addDocFile();
    void removeDocFile();
    void saveProfileInFile();
    void setUrl();

private:
    void initDialog();
    void setIcon();
    void checkForChanges();
    QString profName, profFile;
    Profile *oldProfile, *profile;
    bool changed;
    Mode mode;
    QString iconName;
};

#endif
