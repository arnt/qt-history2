#ifndef ACTIONINTERFACE_H
#define ACTIONINTERFACE_H

#include <qplugininterface.h>

class QAction;
class QObject;

class ActionInterface : public QPlugInInterface
{
public:
    QCString queryPlugInInterface() const { return "ActionInterface"; }

    virtual QAction* create( const QString&, QObject* parent = 0 ) = 0;
    virtual QString group( const QString & ) = 0;
    
};

#endif
