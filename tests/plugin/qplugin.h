#ifndef QPLUGIN_H
#define QPLUGIN_H

#include "qplugininterface.h"
#include <qstringlist.h>
#include <qwindowdefs.h>

class QPlugIn : public QPlugInInterface
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
    QApplicationInterface* requestApplicationInterface( const QCString& );

protected:
    bool loadInterface();
    bool use();
    QPlugInInterface* plugInterface() { return ifc; }
private:
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

