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

#ifndef ACTIONINTERFACE_H
#define ACTIONINTERFACE_H

#include <qcomponentinterface.h>

class QAction;
class QObject;

// {BB206E09-84E5-4777-9FCE-706BABFAB931}
#ifndef IID_ActionInterface
#define IID_ActionInterface QUuid( 0xbb206e09, 0x84e5, 0x4777, 0x9f, 0xce, 0x70, 0x6b, 0xab, 0xfa, 0xb9, 0x31 )
#endif

class ActionInterface : public QFeatureListInterface
{
public:
    virtual QAction* create( const QString&, QObject* parent = 0 ) = 0;
    virtual QString group( const QString & ) const = 0;
    virtual void connectTo( QUnknownInterface *appInterface ) = 0;
};

#endif
