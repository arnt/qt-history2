#ifndef DESIGNERAPPIFACE_H
#define DESIGNERAPPIFACE_H

#include <qapplicationinterface.h>
#include <qmap.h>
#include <qguardedptr.h>

class MainWindow;
class DesignerFormWindowInterface;
class DesignerMainWindowInterface;

class DesignerApplicationInterface : public QApplicationInterface
{
public:
    DesignerApplicationInterface();

    QComponentInterface *requestInterface( const QCString &request );

private:
    QGuardedPtr<DesignerMainWindowInterface> mwIface;

};

class DesignerFormWindowInterface : public QComponentInterface
{
public:
    DesignerFormWindowInterface( MainWindow *mw );

    QVariant requestProperty( const QCString& p );
    bool requestSetProperty( const QCString& p, const QVariant& v );
    bool requestConnect( const char* signal, QObject* target, const char* slot );
    bool requestEvents( QObject* o );
    
private:
    MainWindow *mainWindow;
    
};
    

class DesignerMainWindowInterface : public QComponentInterface
{
public:
    DesignerMainWindowInterface( MainWindow *mw );

    QComponentInterface *requestInterface( const QCString &request );

private:
    QGuardedPtr<DesignerFormWindowInterface> fwIface;
    MainWindow *mainWindow;
    
};

#endif
