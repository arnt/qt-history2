/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROJECTSETTINGSIMPL_H
#define PROJECTSETTINGSIMPL_H

#include "projectsettings.h"

class Project;
class QListViewItem;
class FormWindow;
class SourceFile;

class ProjectSettings : public ProjectSettingsBase
{
    Q_OBJECT

public:
    ProjectSettings( Project *pro, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ProjectSettings();

protected slots:
    void chooseDatabaseFile();
    void chooseProjectFile();
    void helpClicked();
    void okClicked();
    void languageChanged( const QString &lang );

private:
    Project *project;
};

#endif
