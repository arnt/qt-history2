#ifndef DESIGNERAPPIFACE_H
#define DESIGNERAPPIFACE_H

#include <qapplicationinterface.h>
#include <qguardedptr.h>
#include <qobjectlist.h>
#include <../plugins/designerinterface.h>

class MainWindow;
class FormList;
class FormWindow;
class QStatusBar;

class DesignerStatusBarInterface : public QApplicationComponentInterface
{
public:
    DesignerStatusBarInterface( QStatusBar *sb );

    bool requestSetProperty( const QCString &p, const QVariant &v );
};

class DesignerMainWindowInterface : public QApplicationComponentInterface
{
public:
    DesignerMainWindowInterface( MainWindow *mw );

    QApplicationComponentInterface *queryInterface( const QString &request );

private:
    QGuardedPtr<DesignerStatusBarInterface> sbIface;
    MainWindow *mainWindow;

};

class DesignerActiveFormWindowInterface : public QApplicationComponentInterface
{
    Q_OBJECT
    
public:
    DesignerActiveFormWindowInterface ( FormList* );

    QVariant requestProperty( const QCString& p );
    bool requestSetProperty( const QCString& p, const QVariant& v );
    bool requestConnect( const char* signal, QObject* target, const char* slot );
    bool requestConnect( QObject *sender, const char* signal, const char* slot );
    bool requestEvents( QObject* o );

private slots:
    void reconnect();
    
private:
    FormWindow *formWindow;
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

    QObjectList filterObjects;
};

class DesignerFormListInterfaceImpl : public DesignerFormListInterface
{
public:
    DesignerFormListInterfaceImpl( FormList *fl );

    QApplicationComponentInterface *queryInterface( const QString &request );

    const QPixmap* pixmap( QApplicationComponentInterface*, int col ) const;
    void setPixmap( QApplicationComponentInterface*, int col, const QPixmap& );
    QString text( QApplicationComponentInterface*, int col ) const;
    void setText( QApplicationComponentInterface*, int col, const QString& );

    QList<QApplicationComponentInterface>* queryFormInterfaceList();

private:
    QGuardedPtr<DesignerActiveFormWindowInterface> fwIface;
};

class DesignerApplicationInterface : public QApplicationInterface
{
public:
    DesignerApplicationInterface();

    QApplicationComponentInterface *queryInterface( const QString &request );

private:
    QGuardedPtr<DesignerMainWindowInterface> mwIface;
    QGuardedPtr<DesignerFormListInterface> flIface;
};

#endif
