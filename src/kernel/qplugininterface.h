#ifndef QPLUGININTERFACE_H
#define QPLUGININTERFACE_H

#ifndef QT_NO_PLUGIN

#ifndef QT_H
#include "qstringlist.h"
#include "qstrlist.h"
#include "qdict.h"
#endif // QT_H

class QApplication;

class Q_EXPORT QPlugInInterface
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

#ifdef _WS_WIN_
#undef QTPLUGINEXPORT
#define QTPLUGINEXPORT __declspec(dllexport)
#else
#define QTPLUGINEXPORT
#endif

#define QtExportInterface(INTERFACE, IMPLEMENTATION) \
    extern "C" QTPLUGINEXPORT INTERFACE *loadInterface() { return new IMPLEMENTATION(); }

#endif

#endif //QPLUGININTERFACE_H

