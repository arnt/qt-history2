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
class HierarchyList;
class HierarchyItem;
class QStatusBar;
class QListViewItemIterator;

/*
 * Application
 */

class DesignerApplicationInterfaceImpl : public QComponentInterface
{
public:
    DesignerApplicationInterfaceImpl();

    // QUnknownInterface
    QUnknownInterface *queryInterface( const QGuid & );
    unsigned long addRef();
    unsigned long release();
    // QComponentInterface
    QString name() const;
    QString description() const;
    QString version() const;
    QString author() const;

private:
    unsigned long ref;
};

class DesignerStatusBarInterfaceImpl : public DesignerStatusBarInterface
{
public:
    DesignerStatusBarInterfaceImpl( QUnknownInterface* );

    QUnknownInterface *queryInterface( const QGuid & );
    unsigned long addRef();
    unsigned long release();

    void setMessage( const QString &, int ms = 3000 );
    void clear();

public:
    QUnknownInterface* appIface;
    QStatusBar *statusBar;
    unsigned long ref;
};

class DesignerFormListInterfaceImpl : public DesignerFormListInterface
{
public:
    DesignerFormListInterfaceImpl( QUnknownInterface* );
    ~DesignerFormListInterfaceImpl();

    QUnknownInterface *queryInterface( const QGuid & );
    unsigned long addRef();
    unsigned long release();

    const QPixmap* pixmap( DesignerFormInterface*, int col ) const;
    void setPixmap( DesignerFormInterface*, int col, const QPixmap& );
    QString text( DesignerFormInterface*, int col ) const;
    void setText( DesignerFormInterface*, int col, const QString& );

    uint count() const;
    DesignerFormInterface* reset();
    DesignerFormInterface* current();
    DesignerFormInterface* next();
    DesignerFormInterface* prev();

    bool newForm();
    bool loadForm();
    bool saveAll() const;
    void closeAll() const;

    bool connect( const char *, QObject *, const char * );

private:
    QUnknownInterface* appIface;
    FormList *formList;
    QListViewItemIterator *listIterator;
    unsigned long ref;
};

class DesignerFormInterfaceImpl : public DesignerFormInterface
{
public:
    DesignerFormInterfaceImpl( FormListItem *fw, QUnknownInterface *ai );

    QUnknownInterface *queryInterface( const QGuid & );
    unsigned long addRef();
    unsigned long release();

    QVariant property( const QCString& );
    bool setProperty( const QCString&, const QVariant& );

    void save() const;
    void close() const;
    void undo() const;
    void redo() const;

    bool connect( const char *, QObject *, const char * );

private:
    FormListItem *item;
    QUnknownInterface *appIface;

    unsigned long ref;
};

class DesignerWidgetListInterfaceImpl : public DesignerWidgetListInterface
{
public:
    DesignerWidgetListInterfaceImpl( QUnknownInterface * );
    ~DesignerWidgetListInterfaceImpl();

    QUnknownInterface *queryInterface( const QGuid & );
    unsigned long addRef();
    unsigned long release();

    uint count() const;
    DesignerWidgetInterface *reset();
    DesignerWidgetInterface *current();
    DesignerWidgetInterface *next();
    DesignerWidgetInterface *prev();

    void selectAll() const;
    void removeAll() const;

private:
    QUnknownInterface *appIface;
    HierarchyList *hierarchy;
    QListViewItemIterator *listIterator;

    unsigned long ref;
};

class DesignerWidgetInterfaceImpl : public DesignerWidgetInterface
{
public:
    DesignerWidgetInterfaceImpl( HierarchyItem *, QUnknownInterface * );

    QUnknownInterface *queryInterface( const QGuid & );
    unsigned long addRef();
    unsigned long release();

    QVariant property( const QCString& );
    bool setProperty( const QCString&, const QVariant& );

    void setSelected( bool );
    bool selected() const;

    void remove();

private:
    HierarchyItem *item;
    QUnknownInterface *appIface;

    unsigned long ref;
};

#endif
