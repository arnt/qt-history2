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
    DesignerStatusBarInterfaceImpl( QStatusBar *sb, QUnknownInterface* parent );

    QString interfaceID() const { return "DesignerStatusBarInterface"; }

    bool requestSetProperty( const QCString &p, const QVariant &v );
};

class DesignerMainWindowInterfaceImpl : public QApplicationComponentInterface
{
public:
    DesignerMainWindowInterfaceImpl( MainWindow *mw, QUnknownInterface* parent  );

    QString interfaceID() const { return "DesignerMainWindowInterface"; }
};

class DesignerActiveFormWindowInterfaceImpl : public QObject, public QApplicationComponentInterface
{
    Q_OBJECT
    
public:
    DesignerActiveFormWindowInterfaceImpl ( FormList*, QUnknownInterface * );

    QString interfaceID() const { return "DesignerActiveFormWindowInterface"; }

    QVariant requestProperty( const QCString& p );
    bool requestSetProperty( const QCString& p, const QVariant& v );
    bool requestConnect( const char* signal, QObject* target, const char* slot );
    bool requestConnect( QObject *sender, const char* signal, const char* slot );
    bool requestEvents( QObject* o );

private slots:
    void reconnect();
    
private:
    QGuardedPtr<FormWindow> formWindow;
    struct ConnectSignal 
    {
	QCString signal, slot;
	QGuardedPtr<QObject> target;
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
	bool operator==( const ConnectSignal& ) const { return FALSE; }
#endif
    };
    
    struct ConnectSlot
    {
	QGuardedPtr<QObject> sender;
	QCString signal, slot;
#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
	bool operator==( const ConnectSlot& ) const { return FALSE; }
#endif
    };
    
    QValueList<ConnectSignal> signalList;
    QValueList<ConnectSlot> slotList;

    QObjectList filterObjects;
};

class DesignerFormListInterfaceImpl : public DesignerFormListInterface
{
public:
    DesignerFormListInterfaceImpl( FormList *fl, QUnknownInterface* parent  );

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

    QString interfaceID() const { return "DesignerApplicationInterface"; }

    QString name() const { return "Qt Designer"; }
    QString description() const { return "GUI Editor for the Qt Toolkit"; }
    QString version() const { return "1.1"; }
    QString author() const { return "Trolltech"; }
};

#endif
