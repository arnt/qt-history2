 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
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

#ifndef EVENTINTERFACED_H
#define EVENTINTERFACED_H

#include <qcomponentinterface.h>
#include <qobject.h>

// {9958cfbc-64f9-44ce-a65e-2c6c11969a7b}
#ifndef IID_EventInterface
#define IID_EventInterface QUuid( 0x9958cfbc, 0x64f9, 0x44ce, 0xa6, 0x5e, 0x2c, 0x6c, 0x11, 0x96, 0x9a, 0x7b )
#endif

class EventInterface : public QUnknownInterface
{
public:
    virtual QStringList events( QObject *obj ) const = 0;
    virtual void setEventHandler( QObject *obj, QObject *form, const QString &event, const QString &function ) = 0;

};

#endif
