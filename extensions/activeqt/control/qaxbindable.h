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
    virtual ~QAxAggregated();

    IUnknown *controllingUnknown() const
    { return controlling_unknown; }
    QWidget *widget() const 
    { 
	if ( the_object && the_object->isWidgetType() )
	    return (QWidget*)the_object;
	return 0;
    }
    QObject *object() const { return the_object; }

private:
    IUnknown *controlling_unknown;
    QObject *the_object;
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
    static void reportError( int code, const QString &src, const QString &desc, const QString &help = QString::null );

protected:
    bool requestPropertyChange( const char *property );
    void propertyChanged( const char *property );

    IUnknown *clientSite() const;

private:
    IAxServerBase *activex;
};

#endif // QAXBINDABLE_H
