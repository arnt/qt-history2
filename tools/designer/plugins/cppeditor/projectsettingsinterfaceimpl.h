/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROJECTSETTINGSINTERFACEIMPL_H
#define PROJECTSETTINGSINTERFACEIMPL_H

#include <projectsettingsiface.h>

class CppProjectSettings;

class ProjectSettingsInterfaceImpl : public ProjectSettingsInterface
{
public:
    ProjectSettingsInterfaceImpl( QUnknownInterface *outer = 0 );

    ulong addRef();
    ulong release();

    ProjectSettings *projectSetting();
    QStringList projectSettings() const;
    void connectTo( QUnknownInterface *appInterface );
    void deleteProjectSettingsObject( ProjectSettings * );
    QRESULT queryInterface( const QUuid &uuid, QUnknownInterface **iface );

private:
    QUnknownInterface *parent;
    int ref;
    CppProjectSettings *settingsTab;
};

#endif
