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
};

class DesignerWidgetListInterface : public QApplicationComponentInterface
{
public:
    DesignerWidgetListInterface( QObject *fw, QUnknownInterface *parent = 0, const char *name = 0 )
	: QApplicationComponentInterface( fw, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerWidgetListInterface" ); }

    virtual DesignerWidgetInterface *toFirst() = 0;
    virtual DesignerWidgetInterface *current() = 0;
    virtual DesignerWidgetInterface *next() = 0;
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

    virtual DesignerFormWindowInterface *current() = 0;
    virtual DesignerFormWindowInterface *next() = 0;
    virtual DesignerFormWindowInterface *prev() = 0;
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
    DesignerHierarchyViewInterface( QObject *pe, QUnknownInterface *parent, const char *name = 0 )
	: QApplicationComponentInterface( pe, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerHierarchyViewInterface" ); }
};

/*
 * Interfaces for Configuration access
 */

class DesignerConfigurationInterface : public QApplicationComponentInterface
{
    DesignerConfigurationInterface( QObject *pe, QUnknownInterface *parent, const char *name = 0 )
	: QApplicationComponentInterface( pe, parent, name ) {}

    QString interfaceID() const { return createID( QApplicationComponentInterface::interfaceID(), "DesignerConfigurationInterface" ); }
};

#endif //DESIGNERINTERFACE_H
