#ifndef DESIGNERAPPIFACE_H
#define DESIGNERAPPIFACE_H

#include <qapplicationinterface.h>
#include <qguardedptr.h>

class MainWindow;
class FormList;
class QStatusBar;

class DesignerStatusBarInterface : public QComponentInterface
{
public:
    DesignerStatusBarInterface( QStatusBar *sb );

    bool requestSetProperty( const QCString &p, const QVariant &v );
};

class DesignerMainWindowInterface : public QComponentInterface
{
public:
    DesignerMainWindowInterface( MainWindow *mw );

    QComponentInterface *queryInterface( const QString &request );

private:
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

class DesignerFormListInterface : public QComponentInterface
{
public:
    DesignerFormListInterface( FormList *fl );

    QComponentInterface *queryInterface( const QString &request );
    QVariant requestProperty( const QCString &p );

private:
    QGuardedPtr<DesignerFormWindowInterface> fwIface;
};

class DesignerApplicationInterface : public QApplicationInterface
{
public:
    DesignerApplicationInterface();

    QComponentInterface *queryInterface( const QString &request );

private:
    QGuardedPtr<DesignerMainWindowInterface> mwIface;
    QGuardedPtr<DesignerFormListInterface> flIface;
};

#endif
