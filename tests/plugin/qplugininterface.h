#ifndef QPLUGININTERFACE_H
#define QPLUGININTERFACE_H

#include <qstringlist.h>
#include <qstrlist.h>
#include "qapplicationinterfaces.h"

class QPlugInInterface
{
    friend class QPlugIn;
public:
    QPlugInInterface() { cIface = 0; }
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
    QClientInterface* clientInterface() const
    {
	return cIface;
    }

private:
    QClientInterface* requestClientInterface( const QCString& request ) 
    {
	if ( queryInterfaceList().contains( request ) )
	    return cIface ? cIface : ( cIface = new QClientInterface );
	else
	    return 0;
    }

    QClientInterface* cIface;
};

#endif
