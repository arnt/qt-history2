/****************************************************************************
** $Id: $
**
** Declaration of the QAxBindable class
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

#ifndef QAXBINDABLE_H
#define QAXBINDABLE_H

#include <qwidget.h>
#include <private/qcom_p.h>

struct IAxServerBase;
struct IUnknown;

class QAxAggregated
{
    friend class QAxServerBase;
public:
    virtual long queryInterface( const QUuid &iid, void **iface ) = 0;

protected:
    IUnknown *controllingUnknown() const
    { return controlling_unknown; }
    QWidget *widget() const
    { return the_widget; }

private:
    virtual ~QAxAggregated();

    IUnknown *controlling_unknown;
    QWidget *the_widget;
};

#define QAXAGG_IUNKNOWN \
    HRESULT WINAPI QueryInterface( REFIID iid, LPVOID *iface ) { \
    return controllingUnknown()->QueryInterface( iid, iface ); } \
    ULONG WINAPI AddRef() {return controllingUnknown()->AddRef(); } \
    ULONG WINAPI Release() {return controllingUnknown()->Release(); } \


class QAxBindable
{
    friend class QAxServerBase;
public:
    QAxBindable();
    virtual ~QAxBindable();
    
    virtual QAxAggregated *createAggregate();

protected:
    bool requestPropertyChange( const char *property );
    void propertyChanged( const char *property );

private:
    IAxServerBase *activex;
};

#endif // QAXBINDABLE_H
