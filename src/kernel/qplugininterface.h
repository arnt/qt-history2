#ifndef QPLUGININTERFACE_H
#define QPLUGININTERFACE_H

#include <qstringlist.h>
#include <qstrlist.h>
#include <qdict.h>

class QApplication;

class QPlugInInterface
{
public:
    QPlugInInterface() {}
    virtual ~QPlugInInterface() {}

    virtual bool connectNotify( QApplication* ) { return TRUE; }
    virtual bool disconnectNotify( QApplication* ) { return TRUE; }

    virtual QString name() { return QString::null; }
    virtual QString description() { return QString::null; }
    virtual QString author() { return QString::null; }

    virtual QStringList featureList() { return QStringList(); }

    virtual QCString queryPlugInInterface() const = 0;
};

#endif
