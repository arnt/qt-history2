/****************************************************************************
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QAXAGGREGATED_H
#define QAXAGGREGATED_H

#include <qobject.h>

struct IUnknown;
struct QUuid;

class QAxAggregated
{
    friend class QAxServerBase;
    friend class QAxHostWindow;
public:
    virtual long queryInterface(const QUuid &iid, void **iface) = 0;

protected:
    virtual ~QAxAggregated()
    {}

    inline IUnknown *controllingUnknown() const
    { return controlling_unknown; }
    inline QWidget *widget() const 
    { 
        if (the_object && the_object->isWidgetType())
            return (QWidget*)the_object;
        return 0;
    }
    inline QObject *object() const { return the_object; }

private:
    IUnknown *controlling_unknown;
    QObject *the_object;
};

#define QAXAGG_IUNKNOWN \
    HRESULT WINAPI QueryInterface(REFIID iid, LPVOID *iface) { \
    return controllingUnknown()->QueryInterface(iid, iface); } \
    ULONG WINAPI AddRef() {return controllingUnknown()->AddRef(); } \
    ULONG WINAPI Release() {return controllingUnknown()->Release(); } \


#endif // QAXAGGREGATED_H
