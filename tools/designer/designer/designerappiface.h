#ifndef DESIGNERAPPIFACE_H
#define DESIGNERAPPIFACE_H

#include <qguardedptr.h>
#include <qobjectlist.h>
#include <qwidget.h> // for GCC 2.7.* compatibility
#include <qptrdict.h>
#include <../plugins/designerinterface.h>

class MainWindow;
class FormList;
class FormListItem;
class FormWindow;
class PropertyEditor;
class QStatusBar;
class QListViewItemIterator;

class DesignerStatusBarInterfaceImpl : public DesignerStatusBarInterface
{
public:
    DesignerStatusBarInterfaceImpl( QStatusBar *sb, QUnknownInterface* parent );

    void setMessage( const QString &, int ms = 3000 );

private:
    QStatusBar *statusBar;
};

class DesignerMainWindowInterfaceImpl : public DesignerMainWindowInterface
{
public:
    DesignerMainWindowInterfaceImpl( MainWindow *mw, QUnknownInterface* parent );

private:
    MainWindow *mainWindow;
};

class DesignerFormWindowInterfaceImpl : public DesignerFormWindowInterface
{
public:
    DesignerFormWindowInterfaceImpl( FormListItem *fw, QUnknownInterface *parent );

    void save() const;
    void close() const;
    void undo() const;
    void redo() const;

    QString fileName() const;
    void setFileName( const QString & );

    QPixmap icon() const;
    void setIcon( const QPixmap & );

    bool connect( const char *, QObject *, const char * );

protected:
    QObject *component() const;
    void setFormListItem( FormListItem * );
    FormListItem *formListItem() const;

private:
    FormListItem *item;
};

class DesignerActiveFormWindowInterfaceImpl : public DesignerFormWindowInterfaceImpl
{
public:
    DesignerActiveFormWindowInterfaceImpl( FormList *fl, QUnknownInterface *parent );
    QString interfaceId() const { return createId( DesignerFormWindowInterface::interfaceId(), "DesignerActiveFormWindowInterface" ); }

    unsigned long addRef();

private:
    QGuardedPtr<FormList> formList;
};

class DesignerFormListInterfaceImpl : public DesignerFormListInterface
{
public:
    DesignerFormListInterfaceImpl( FormList *fl, QUnknownInterface* parent  );

    unsigned long addRef();
    unsigned long release();

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

    bool connect( const char *, QObject *, const char * );

private:
    FormList *formList;
    QListViewItemIterator *listIterator;
};

class DesignerWidgetListInterfaceImpl : public DesignerWidgetListInterface
{
    friend class DesignerActiveFormWindowInterfaceImpl;
public:
    DesignerWidgetListInterfaceImpl( FormWindow *fw, QUnknownInterface *parent );

    unsigned long release();

    uint count() const;
    DesignerWidgetInterface* toFirst();
    DesignerWidgetInterface* current();
    DesignerWidgetInterface* next();

    void selectAll() const;
    void removeAll() const;

/*    FormWindow *formWindow() const;
    void setFormWindow( FormWindow* );
*/
private:
    QPtrDictIterator<QWidget> *dictIterator;
    FormWindow *formWindow;
};

class DesignerWidgetInterfaceImpl : public DesignerWidgetInterface
{
public:
    DesignerWidgetInterfaceImpl( QWidget *w, QUnknownInterface *parent );

    void setSelected( bool );
    bool selected() const;

    void remove();

private:
    QWidget *widget;
};

class DesignerActiveWidgetInterfaceImpl : public DesignerWidgetInterfaceImpl
{
    friend class DesignerWidgetListInterfaceImpl;
public:
    DesignerActiveWidgetInterfaceImpl( PropertyEditor *pe, QUnknownInterface *parent );

    QString interfaceId() const { return createId( DesignerWidgetInterfaceImpl::interfaceId(), "DesignerActiveWidgetInterface" ); }

    unsigned long addRef();

private:
    QGuardedPtr<PropertyEditor> propertyEditor;
};

/*
 * Application
 */

class DesignerApplicationInterface : public QComponentInterface
{
public:
    DesignerApplicationInterface();

    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "DesignerApplicationInterface" ); }

    QString name() const { return "Qt Designer"; }
    QString description() const { return "GUI Editor for the Qt Toolkit"; }
    QString version() const { return "1.1"; }
    QString author() const { return "Trolltech"; }

};

#endif
