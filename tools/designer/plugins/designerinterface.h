#ifndef DESIGNERINTERFACE_H
#define DESIGNERINTERFACE_H

/* 
  This include file declares all interfaces for the Qt Designer 
  that can be used in the plugins.
*/

#include <qcomponentinterface.h>
#include "../designer/actioniface.h"
#include "../shared/widgetinterface.h"
#include "../designer/filteriface.h"

typedef QApplicationComponentInterface DesignerFormWindowInterface;
typedef QApplicationComponentInterface DesignerActiveFormWindowInterface;
typedef QApplicationComponentInterface DesignerMainWindowInterface;
typedef QApplicationComponentInterface DesignerStatusBarInterface;

class DesignerFormListInterface : public QApplicationComponentInterface
{
public:
    DesignerFormListInterface( QObject *li, QUnknownInterface* parent = 0 ) : QApplicationComponentInterface( li, parent ) {}
    QString interfaceID() const { return "DesignerFormListInterface"; }

    virtual const QPixmap* pixmap( DesignerFormWindowInterface*, int col ) const = 0;
    virtual void setPixmap( DesignerFormWindowInterface*, int col, const QPixmap& ) = 0;
    virtual QString text( DesignerFormWindowInterface*, int col ) const = 0;
    virtual void setText( DesignerFormWindowInterface*, int col, const QString& ) = 0;

    virtual QList<DesignerFormWindowInterface>* queryFormInterfaceList() = 0;
};

#endif //DESIGNERINTERFACE_H
