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

#ifndef PREFERENCEINTERFACE_H
#define PREFERENCEINTERFACE_H

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

// {5c168ee7-4bee-469f-9995-6afdb04ce5a2}
#ifndef IID_Preference
#define IID_Preference QUuid( 0x5c168ee7, 0x4bee, 0x469f, 0x99, 0x95, 0x6a, 0xfd, 0xb0, 0x4c, 0xe5, 0xa2 )
#endif

struct PreferenceInterface : public QUnknownInterface
{
    struct Preference
    {
	QWidget *tab;
	QString title;
	QObject *receiver;
	const char *init_slot;
	const char *accept_slot;
    };

    virtual Preference *preference() = 0;
    virtual void connectTo( QUnknownInterface *appInterface ) = 0;
    virtual void deletePreferenceObject( Preference * ) = 0;
};

#endif
