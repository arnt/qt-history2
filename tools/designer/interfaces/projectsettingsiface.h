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

#ifndef PROJECTSETTINGSIFACE_H
#define PROJECTSETTINGSIFACE_H

//
//  W A R N I N G  --  PRIVATE INTERFACES
//  --------------------------------------
//
// This file and the interfaces declared in the file are not
// public. It exists for internal purpose. This header file and
// interfaces may change from version to version (even binary
// incompatible) without notice, or even be removed.
//
// We mean it.
//
//

#include <private/qcom_p.h>
#include <qstring.h>
class QWidget;
class QObject;

// {d332785d-17fb-4894-84fe-50dbd0ad9512}
#ifndef IID_ProjectSettings
#define IID_ProjectSettings QUuid( 0xd332785d, 0x17fb, 0x4894, 0x84, 0xfe, 0x50, 0xdb, 0xd0, 0xad, 0x95, 0x12 )
#endif

struct ProjectSettingsInterface : public QUnknownInterface
{
    struct ProjectSettings
    {
	QWidget *tab;
	QString title;
	QObject *receiver;
	const char *init_slot;
	const char *accept_slot;
    };

    virtual ProjectSettings *projectSetting() = 0;
    virtual QStringList projectSettings() const = 0;
    virtual void connectTo( QUnknownInterface *appInterface ) = 0;
    virtual void deleteProjectSettingsObject( ProjectSettings * ) = 0;
};

#endif
