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

class DesignerFormListInterface : public QComponentInterface
{
public:
    DesignerFormListInterface( QObject *li ) : QComponentInterface( li ) {}

    virtual const QPixmap* pixmap( QComponentInterface*, int col ) const = 0;
    virtual void setPixmap( QComponentInterface*, int col, const QPixmap& ) = 0;
    virtual QString text( QComponentInterface*, int col ) const = 0;
    virtual void setText( QComponentInterface*, int col, const QString& ) = 0;

    virtual QList<QComponentInterface> queryFormInterfaceList() = 0;
};

#endif //DESIGNERINTERFACE_H
