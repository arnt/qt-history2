/****************************************************************************
** $Id: $
**
** Declaration of the QAxServerBase interface
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QAXSERVERBASE_H
#define QAXSERVERBASE_H

#ifdef __ATLBASE_H__
#include "qaxfactory.h"

class CExeModule : public CComModule
{
public:
    LONG Unlock();
    DWORD dwThreadID;
    HANDLE hEventShutdown;
    void MonitorShutdown();
    bool StartMonitor();
    bool bActivity;

    static QAxFactoryInterface *factory();
    static QInterfacePtr<QAxFactoryInterface> _factory;
};
extern CExeModule _Module;
#endif

template<class Type> class QIntDict;
class QMetaProperty;
class QObject;
class QWidget;

struct IAxServerBase : public IUnknown
{
    virtual QObject *qObject() = 0;
    virtual QWidget *widget() = 0;
    virtual void emitPropertyChanged( long dispId ) = 0;
    virtual bool emitRequestPropertyChange( long dispId ) = 0;
    virtual QIntDict<QMetaProperty> *propertyList() = 0;
};

#endif //QAXSERVERBASE_H
