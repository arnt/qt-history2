#ifndef QPLUGIN_H
#define QPLUGIN_H

#include "qplugininterface.h"
#include <qstringlist.h>
#include <qwindowdefs.h>

class Q_EXPORT QPlugIn : public QPlugInInterface
{
public:
    enum LibraryPolicy
    { 
	Default,
	OptimizeSpeed,
	Manual
    };

    QPlugIn( const QString& filename, LibraryPolicy = Default, const char* fn = 0 );
    ~QPlugIn();

    bool load();
    bool unload( bool force = FALSE );
    bool loaded() const;

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

#ifdef _WS_WIN_
    HINSTANCE pHnd;
#else
    void* pHnd;
#endif
    QString libfile;
    LibraryPolicy libPol;
    QCString function;
};

#endif // QPLUGIN_H

