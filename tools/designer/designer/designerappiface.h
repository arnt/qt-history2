#ifndef DESIGNERAPPIFACE_H
#define DESIGNERAPPIFACE_H

#include <qguardedptr.h>
#include <qobjectlist.h>
#include <../plugins/designerinterface.h>

class MainWindow;
class FormList;
class FormWindow;
class QStatusBar;

class DesignerStatusBarInterfaceImpl : public QApplicationComponentInterface
{
public:
    DesignerStatusBarInterfaceImpl( QStatusBar *sb );

    bool requestSetProperty( const QCString &p, const QVariant &v );
};

class DesignerMainWindowInterfaceImpl : public QApplicationComponentInterface
{
public:
    DesignerMainWindowInterfaceImpl( MainWindow *mw );

    QUnknownInterface *queryInterface( const QString &request );

private:
    QGuardedPtr<DesignerStatusBarInterface> sbIface;
    MainWindow *mainWindow;

};

class DesignerActiveFormWindowInterfaceImpl : public QApplicationComponentInterface
{
    Q_OBJECT
    
public:
    DesignerActiveFormWindowInterfaceImpl ( FormList* );

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

    QUnknownInterface *queryInterface( const QString &request );

    const QPixmap* pixmap( DesignerFormWindowInterface*, int col ) const;
    void setPixmap( DesignerFormWindowInterface*, int col, const QPixmap& );
    QString text( DesignerFormWindowInterface*, int col ) const;
    void setText( DesignerFormWindowInterface*, int col, const QString& );

    QList<DesignerFormWindowInterface>* queryFormInterfaceList();

private:
    QGuardedPtr<DesignerActiveFormWindowInterfaceImpl> fwIface;
};

class DesignerApplicationInterface : public QApplicationInterface
{
public:
    DesignerApplicationInterface();

    QUnknownInterface *queryInterface( const QString &request );

private:
    QGuardedPtr<DesignerMainWindowInterface> mwIface;
    QGuardedPtr<DesignerFormListInterface> flIface;
};

#endif
