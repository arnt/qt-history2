#ifndef DESIGNERINTERFACE_H
#define DESIGNERINTERFACE_H

/* 
  This include file declares all interfaces for the Qt Designer 
  that can be used in the plugins.
*/

#include <qapplicationinterface.h>
#include "../designer/actioniface.h"
#include "../shared/widgetinterface.h"
#include "../designer/filteriface.h"

class DesignerFormListInterface : public QApplicationComponentInterface
{
public:
    DesignerFormListInterface( QObject *li ) : QApplicationComponentInterface( li ) {}

    virtual const QPixmap* pixmap( QApplicationComponentInterface*, int col ) const = 0;
    virtual void setPixmap( QApplicationComponentInterface*, int col, const QPixmap& ) = 0;
    virtual QString text( QApplicationComponentInterface*, int col ) const = 0;
    virtual void setText( QApplicationComponentInterface*, int col, const QString& ) = 0;

    virtual QList<QApplicationComponentInterface>* queryFormInterfaceList() = 0;
};

#endif //DESIGNERINTERFACE_H
