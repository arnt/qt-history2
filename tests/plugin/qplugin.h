#ifndef QPLUGIN_H
#define QPLUGIN_H

#include "qplugininterface.h"

class QPlugIn;

class QPlugIn : public QObject, public QPlugInInterface
{
    Q_OBJECT
    Q_ENUMS( LibraryPolicy )
    Q_PROPERTY( LibraryPolicy policy READ policy WRITE setPolicy )
    Q_PROPERTY( QString library READ library )

public:
    enum LibraryPolicy
    { 
	Default,
	OptimizeSpeed,
	OptimizeMemory,
	Manual
    };

    QPlugIn( const QString& filename, LibraryPolicy = Default );
    ~QPlugIn();

    bool load();
    bool unload( bool = FALSE );

    void setPolicy( LibraryPolicy pol );
    LibraryPolicy policy() const;

    QString library() const;

    QString name();
    QString description();
    QString author();

signals:
    void loaded();
    void unloaded();

protected slots:
    void unuse();
    bool use();

protected:
    bool loadInterface();
    QPlugInInterface* iface() { return ifc; }
    void guard( QObject* o );

private slots:
    bool deref();

private:
    QPlugInInterface* ifc;

    uint count;
    void ref() { count++; }

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

