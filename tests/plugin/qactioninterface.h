#ifndef QACTIONINTERFACE_H
#define QACTIONINTERFACE_H

#include <qplugininterface.h>

class QAction;
class QObject;

class ActionInterface : public QPlugInInterface
{
public:
    QString queryInterface() const { return "ActionInterface"; }

    virtual QAction* create( const QString&, QObject* parent = 0 ) = 0;
};

#endif // QACTIONINTERFACE_H
