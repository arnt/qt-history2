#ifndef DESIGNERAPPIFACE_H
#define DESIGNERAPPIFACE_H

#include <qapplicationinterface.h>
#include <qguardedptr.h>

class MainWindow;
class QStatusBar;
class DesignerMainWindowInterface;
class DesignerFormWindowInterface;
class DesignerStatusBarInterface;

class DesignerApplicationInterface : public QApplicationInterface
{
public:
    DesignerApplicationInterface();

    QComponentInterface *queryInterface( const QCString &request );

private:
    QGuardedPtr<DesignerMainWindowInterface> mwIface;

};

class DesignerMainWindowInterface : public QComponentInterface
{
public:
    DesignerMainWindowInterface( MainWindow *mw );

    QComponentInterface *queryInterface( const QCString &request );

private:
    QGuardedPtr<DesignerFormWindowInterface> fwIface;
    QGuardedPtr<DesignerStatusBarInterface> sbIface;
    MainWindow *mainWindow;

};

class DesignerFormWindowInterface : public QComponentInterface
{
    Q_OBJECT
    
public:
    DesignerFormWindowInterface( MainWindow *mw );

    QVariant requestProperty( const QCString& p );
    bool requestSetProperty( const QCString& p, const QVariant& v );
    bool requestConnect( const char* signal, QObject* target, const char* slot );
    bool requestConnect( QObject *sender, const char* signal, const char* slot );
    bool requestEvents( QObject* o );

private slots:
    void reconnect();
    
private:
    MainWindow *mainWindow;
    struct Connect1 
    {
	QCString signal, slot;
	QGuardedPtr<QObject> target;
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
	bool operator==( const Connect1& ) const { return FALSE; }
#endif
    };
    
    struct Connect2
    {
	QGuardedPtr<QObject> sender;
	QCString signal, slot;
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
	bool operator==( const Connect2& ) const { return FALSE; }
#endif
    };
    
    QValueList<Connect1> connects1;
    QValueList<Connect2> connects2;
};

class DesignerStatusBarInterface : public QComponentInterface
{
public:
    DesignerStatusBarInterface( QStatusBar *sb );

private:
    QStatusBar *statusBar;

};

#endif
