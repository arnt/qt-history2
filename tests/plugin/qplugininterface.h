#ifndef QPLUGININTERFACE_H
#define QPLUGININTERFACE_H

#include <qstringlist.h>

class QApplicationInterface;

class QPlugInInterface
{
public:
    QPlugInInterface() {}
    virtual ~QPlugInInterface() {}

    virtual QString name() { return QString::null; }
    virtual QString description() { return QString::null; }
    virtual QString author() { return QString::null; }

    virtual QStringList featureList() { return QStringList(); }
    virtual QString queryPlugInInterface() = 0;
    virtual QApplicationInterface* requestApplicationInterface( const QCString& ) 
    { 
	return 0;
    }
};

#endif
