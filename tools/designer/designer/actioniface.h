#ifndef ACTIONINTERFACE_H
#define ACTIONINTERFACE_H

#include <qcomponentinterface.h>

class QAction;
class QObject;

class ActionInterface : public QUnknownInterface
{
public:
    ActionInterface( QUnknownInterface *parent = 0, const char *name = 0 ) 
	: QUnknownInterface( parent, name ) {}

    virtual QStringList featureList() const = 0;

    virtual QAction* create( const QString&, QObject* parent = 0 ) = 0;
    virtual QString group( const QString & ) = 0;

    QString interfaceID() const { return createID( QUnknownInterface::interfaceID(), "ActionInterface" ); }
};

#endif
