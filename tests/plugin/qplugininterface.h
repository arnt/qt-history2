#ifndef QPLUGININTERFACE_H
#define QPLUGININTERFACE_H

#include <qstringlist.h>
#include <qstrlist.h>
#include <qdict.h>
#include "qdualinterface.h"

class QPlugInInterface
{
    friend class QPlugIn;
public:
    QPlugInInterface(): cIfaces( 53 ) 
    {
	cIfaces.setAutoDelete( TRUE );
    }
    virtual ~QPlugInInterface() {}

    virtual QString name() { return QString::null; }
    virtual QString description() { return QString::null; }
    virtual QString author() { return QString::null; }

    virtual QStringList featureList() { return QStringList(); }
    
    virtual QCString queryPlugInInterface() const = 0;
    virtual QStrList queryInterfaceList() const
    {
	return QStrList();
    }

protected:
    QClientInterface* clientInterface( const QCString& request ) const
    {
	return cIfaces[request];
    }

    virtual void connectNotify( const QCString& iface )
    {
    }

private:
    QClientInterface* requestClientInterface( const QCString& request );

    QDict<QClientInterface> cIfaces;
};

#endif
