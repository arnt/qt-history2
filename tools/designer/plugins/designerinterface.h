#ifndef DESIGNERINTERFACE_H
#define DESIGNERINTERFACE_H

/* 
  This include file declares all interfaces for the Qt Designer 
  that can be used in the plugins.
*/

#include <qcomponentinterface.h>
#include <qlist.h>
#include "../designer/actioniface.h"
#include "../shared/widgetinterface.h"
#include "../designer/filteriface.h"

/*
 * Interfaces for MainWindow access
 */
class DesignerMainWindowInterface : public QApplicationComponentInterface
{
public:
    DesignerMainWindowInterface( QObject *c, QUnknownInterface *parent = 0, const char *name = 0 )
	: QApplicationComponentInterface( c, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerMainWindowInterface" ); }
};

class DesignerStatusBarInterface : public QApplicationComponentInterface
{
public:
    DesignerStatusBarInterface( QObject *c, QUnknownInterface *parent = 0, const char *name = 0 )
	: QApplicationComponentInterface( c, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerStatusBarInterface" ); }
};

/*
 * Interfaces for Widget access
 */

class DesignerWidgetInterface : public QApplicationComponentInterface
{
public:
    DesignerWidgetInterface( QObject *w, QUnknownInterface *parent = 0, const char *name = 0 )
	: QApplicationComponentInterface( w, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerWidgetInterface" ); }

    virtual void setSelected( bool ) = 0;
    virtual bool selected() const = 0;

    virtual void remove() = 0;
};

class DesignerWidgetListInterface : public QApplicationComponentInterface
{
public:
    DesignerWidgetListInterface( QObject *fw, QUnknownInterface *parent = 0, const char *name = 0 )
	: QApplicationComponentInterface( fw, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerWidgetListInterface" ); }

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

class DesignerFormWindowInterface : public QApplicationComponentInterface
{
public:
    DesignerFormWindowInterface( QObject *fw, QUnknownInterface *parent, const char *name = 0 )
	: QApplicationComponentInterface( fw, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerFormWindowInterface" ); }

    virtual void save() const = 0;
    virtual void close() const = 0;
    virtual void undo() const = 0;
    virtual void redo() const = 0;
};

class DesignerFormListInterface : public QApplicationComponentInterface
{
public:
    DesignerFormListInterface( QObject *li, QUnknownInterface* parent, const char *name = 0 ) 
	: QApplicationComponentInterface( li, parent, name ) {}
    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerFormListInterface" ); }

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
};

/*
 * Interfaces for PropertyEditor access
 */

class DesignerPropertyEditorInterface : public QApplicationComponentInterface
{
public:
    DesignerPropertyEditorInterface( QObject *pe, QUnknownInterface *parent, const char *name = 0 )
	: QApplicationComponentInterface( pe, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerPropertyEditorInterface" ); }
};

/*
 * Interfaces for HierarchyView access
 */

class DesignerHierarchyViewInterface : public QApplicationComponentInterface
{
public:
    DesignerHierarchyViewInterface( QObject *pe, QUnknownInterface *parent, const char *name = 0 )
	: QApplicationComponentInterface( pe, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerHierarchyViewInterface" ); }
};

/*
 * Interfaces for Configuration access
 */

class DesignerConfigurationInterface : public QApplicationComponentInterface
{
public:
    DesignerConfigurationInterface( QObject *pe, QUnknownInterface *parent, const char *name = 0 )
	: QApplicationComponentInterface( pe, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerConfigurationInterface" ); }
};

#endif //DESIGNERINTERFACE_H
