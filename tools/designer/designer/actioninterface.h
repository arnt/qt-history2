#ifndef ACTIONINTERFACE_H
#define ACTIONINTERFACE_H

#include <qcomponentinterface.h>

class QAction;
class QObject;

class ActionInterface : public QUnknownInterface
{
public:
    ActionInterface( QUnknownInterface *parent = 0  ) 
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "ActionInterface" ); }

    virtual QStringList featureList() const = 0;

    virtual QAction* create( const QString&, QObject* parent = 0 ) = 0;
    virtual QString group( const QString & ) const = 0;
    virtual void connectTo( QUnknownInterface *appInterface ) = 0;
};

#endif
