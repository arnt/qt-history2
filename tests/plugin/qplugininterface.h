#ifndef QPLUGININTERFACE_H
#define QPLUGININTERFACE_H

#include <qstringlist.h>

class QClientInterface;

class QPlugInInterface
{
public:
    QPlugInInterface() {}
    virtual ~QPlugInInterface() {}

    virtual QString name() { return QString::null; }
    virtual QString description() { return QString::null; }
    virtual QString author() { return QString::null; }

    virtual QStringList featureList() { return QStringList(); }
    
    virtual QCString queryPlugInInterface() const = 0;
    virtual QClientInterface* requestClientInterface( const QCString& ) 
    {
	return 0;
    }
};

#endif
