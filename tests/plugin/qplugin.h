#ifndef QPLUGIN_H
#define QPLUGIN_H

#include "qplugininterface.h"
#include <qstringlist.h>
#include <qwindowdefs.h>

class QPlugIn : virtual public QPlugInInterface
{
public:
    enum LibraryPolicy
    { 
	Default,
	OptimizeSpeed,
	Manual
    };

    QPlugIn( const QString& filename, LibraryPolicy = Default );
    ~QPlugIn();

    bool load();
    bool unload( bool force = FALSE );

    void setPolicy( LibraryPolicy pol );
    LibraryPolicy policy() const;

    QString library() const;

    QString name();
    QString description();
    QString author();

    QStringList featureList();

protected:
    bool use();
    QPlugInInterface* plugInterface() { return ifc; }

private:
    bool loadInterface();
    QPlugInInterface* ifc;

    typedef QPlugInInterface* (*LoadInterfaceProc)();
    typedef bool (*ConnectProc)( QApplication* );

#ifdef _WS_WIN_
    HINSTANCE pHnd;
#else
    void* pHnd;
#endif
    QString libfile;
    LibraryPolicy libPol;
};

#endif // QPLUGIN_H

