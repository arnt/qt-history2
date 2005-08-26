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

#ifndef QAXAGGREGATED_H
#define QAXAGGREGATED_H

#include <QtCore/qobject.h>

struct IUnknown;
struct QUuid;

class QAxAggregated
{
    friend class QAxServerBase;
    friend class QAxClientSite;
public:
    virtual long queryInterface(const QUuid &iid, void **iface) = 0;

protected:
    virtual ~QAxAggregated()
    {}

    inline IUnknown *controllingUnknown() const
    { return controlling_unknown; }
    inline QWidget *widget() const 
    {
        return qobject_cast<QWidget*>(the_object);
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
