/**********************************************************************
**
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
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
