#ifndef DESIGNERINTERFACE_H
#define DESIGNERINTERFACE_H

/* 
  This include file declares all interfaces for the Qt Designer 
  that can be used in the plugins.
*/

#include <qcomponentinterface.h>
#include <qlist.h>
#include "../designer/actioninterface.h"
#include "../shared/widgetinterface.h"
#include "../designer/filterinterface.h"

/*
 * Interfaces for MainWindow access
 */
class DesignerMainWindowInterface : public QUnknownInterface
{
public:
    DesignerMainWindowInterface( QUnknownInterface *parent = 0 )
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "DesignerMainWindowInterface" ); }
};

class DesignerStatusBarInterface : public QUnknownInterface
{
public:
    DesignerStatusBarInterface( QUnknownInterface *parent = 0 )
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "DesignerStatusBarInterface" ); }

    virtual void setMessage( const QString&, int ms = 3000 ) = 0;
};

/*
 * Interfaces for Widget access
 */

class DesignerWidgetInterface : public QUnknownInterface
{
public:
    DesignerWidgetInterface( QUnknownInterface *parent = 0 )
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "DesignerWidgetInterface" ); }

    virtual void setSelected( bool ) = 0;
    virtual bool selected() const = 0;

    virtual void remove() = 0;
};

class DesignerWidgetListInterface : public QUnknownInterface
{
public:
    DesignerWidgetListInterface( QUnknownInterface *parent = 0 )
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "DesignerWidgetListInterface" ); }

    virtual uint count() const = 0;
    virtual DesignerWidgetInterface *toFirst() = 0;
    virtual DesignerWidgetInterface *current() = 0;
    virtual DesignerWidgetInterface *next() = 0;

    virtual void selectAll() const = 0;
    virtual void removeAll() const = 0;
};

/*
 * Interfaces for FormWindow access
 */

class DesignerFormWindowInterface : public QUnknownInterface
{
public:
    DesignerFormWindowInterface( QUnknownInterface *parent )
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "DesignerFormWindowInterface" ); }

    virtual void save() const = 0;
    virtual void close() const = 0;
    virtual void undo() const = 0;
    virtual void redo() const = 0;

    virtual QString fileName() const = 0;
    virtual void setFileName( const QString& ) = 0;

    virtual QPixmap icon() const = 0;
    virtual void setIcon( const QPixmap& ) = 0;

    virtual bool connect( const char *, QObject *, const char * ) = 0;
};

class DesignerFormListInterface : public QUnknownInterface
{
public:
    DesignerFormListInterface( QUnknownInterface* parent ) 
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "DesignerFormListInterface" ); }

    virtual const QPixmap* pixmap( DesignerFormWindowInterface*, int col ) const = 0;
    virtual void setPixmap( DesignerFormWindowInterface*, int col, const QPixmap& ) = 0;
    virtual QString text( DesignerFormWindowInterface*, int col ) const = 0;
    virtual void setText( DesignerFormWindowInterface*, int col, const QString& ) = 0;

    virtual uint count() const = 0;
    virtual DesignerFormWindowInterface *current() = 0;
    virtual DesignerFormWindowInterface *next() = 0;
    virtual DesignerFormWindowInterface *prev() = 0;

    virtual bool newForm() = 0;
    virtual bool loadForm() = 0;
    virtual bool saveAll() const = 0;
    virtual void closeAll() const = 0;

    virtual bool connect( const char *, QObject *, const char * ) = 0;
};

/*
 * Interfaces for PropertyEditor access
 */

class DesignerPropertyEditorInterface : public QUnknownInterface
{
public:
    DesignerPropertyEditorInterface( QUnknownInterface *parent )
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "DesignerPropertyEditorInterface" ); }
};

/*
 * Interfaces for HierarchyView access
 */

class DesignerHierarchyViewInterface : public QUnknownInterface
{
public:
    DesignerHierarchyViewInterface( QUnknownInterface *parent )
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "DesignerHierarchyViewInterface" ); }
};

/*
 * Interfaces for Configuration access
 */

class DesignerConfigurationInterface : public QUnknownInterface
{
public:
    DesignerConfigurationInterface( QUnknownInterface *parent )
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "DesignerConfigurationInterface" ); }
};

#endif //DESIGNERINTERFACE_H
