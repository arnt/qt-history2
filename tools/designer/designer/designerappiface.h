#ifndef DESIGNERAPPIFACE_H
#define DESIGNERAPPIFACE_H

#include <qguardedptr.h>
#include <qobjectlist.h>
#include <qwidget.h> // for GCC 2.7.* compatibility
#include <qptrdict.h>
#include <../plugins/designerinterface.h>

class MainWindow;
class FormList;
class FormWindow;
class PropertyEditor;
class QStatusBar;
class QListViewItemIterator;

class DesignerStatusBarInterfaceImpl : public DesignerStatusBarInterface
{
public:
    DesignerStatusBarInterfaceImpl( QStatusBar *sb, QUnknownInterface* parent );

    bool requestSetProperty( const QCString &p, const QVariant &v );
};

class DesignerMainWindowInterfaceImpl : public DesignerMainWindowInterface
{
public:
    DesignerMainWindowInterfaceImpl( MainWindow *mw, QUnknownInterface* parent );
};

class DesignerFormWindowInterfaceImpl : public DesignerFormWindowInterface
{
public:
    DesignerFormWindowInterfaceImpl( FormWindow *fw, QUnknownInterface *parent, const char *name = 0 );

    bool initialize();

    void save() const;
    void close() const;
    void undo() const;
    void redo() const;
};

class DesignerActiveFormWindowInterfaceImpl : public DesignerFormWindowInterfaceImpl
{
public:
    DesignerActiveFormWindowInterfaceImpl( FormList *fl, QUnknownInterface *parent, const char *name = 0 );

    QString interfaceId() const { return createId( DesignerFormWindowInterface::interfaceId(), "DesignerActiveFormWindowInterface" ); }

    bool initialize();

    bool requestConnect( const char* signal, QObject* target, const char* slot );
    bool requestConnect( QObject *sender, const char* signal, const char* slot );
    bool requestEvents( QObject* o );

private:
    void reconnect();

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
    
    QGuardedPtr<FormList> formList;
    QGuardedPtr<FormWindow> lastForm;
};

class DesignerFormListInterfaceImpl : public DesignerFormListInterface
{
public:
    DesignerFormListInterfaceImpl( FormList *fl, QUnknownInterface* parent  );

    bool initialize();
    bool cleanup();

    const QPixmap* pixmap( DesignerFormWindowInterface*, int col ) const;
    void setPixmap( DesignerFormWindowInterface*, int col, const QPixmap& );
    QString text( DesignerFormWindowInterface*, int col ) const;
    void setText( DesignerFormWindowInterface*, int col, const QString& );

    uint count() const;
    DesignerFormWindowInterface* current();
    DesignerFormWindowInterface* next();
    DesignerFormWindowInterface* prev();

    bool newForm();
    bool loadForm();
    bool saveAll() const;
    void closeAll() const;

private:
    QListViewItemIterator *listIterator;
};

class DesignerWidgetListInterfaceImpl : public DesignerWidgetListInterface
{
    friend class DesignerActiveFormWindowInterfaceImpl;
public:
    DesignerWidgetListInterfaceImpl( FormWindow *fw, QUnknownInterface *parent );

    bool initialize();
    bool cleanup();

    uint count() const;
    DesignerWidgetInterface* toFirst();
    DesignerWidgetInterface* current();
    DesignerWidgetInterface* next();

    void selectAll() const;
    void removeAll() const;

    FormWindow *formWindow() const;

private:
    QPtrDictIterator<QWidget> *dictIterator;
};

class DesignerWidgetInterfaceImpl : public DesignerWidgetInterface
{
public:
    DesignerWidgetInterfaceImpl( QWidget *w, QUnknownInterface *parent );

    void setSelected( bool );
    bool selected() const;

    void remove();
};

class DesignerActiveWidgetInterfaceImpl : public DesignerWidgetInterfaceImpl
{
    friend class DesignerWidgetListInterfaceImpl;
public:
    DesignerActiveWidgetInterfaceImpl( PropertyEditor *pe, QUnknownInterface *parent );

    QString interfaceId() const { return createId( DesignerWidgetInterfaceImpl::interfaceId(), "DesignerActiveWidgetInterface" ); }

    bool initialize();

private:
    QGuardedPtr<PropertyEditor> propertyEditor;
};

/*
 * Application
 */

class DesignerApplicationInterface : public QApplicationInterface
{
public:
    DesignerApplicationInterface();

    QString interfaceId() const;

    QString name() const { return "Qt Designer"; }
    QString description() const { return "GUI Editor for the Qt Toolkit"; }
    QString version() const { return "1.1"; }
    QString author() const { return "Trolltech"; }
};

#endif
