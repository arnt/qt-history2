#ifndef ACTIONINTERFACE_H
#define ACTIONINTERFACE_H

#include <qcomponentinterface.h>

class QAction;
class QObject;

class ActionInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() = 0;

    virtual QAction* create( const QString&, QObject* parent = 0 ) = 0;
    virtual QString group( const QString & ) = 0;

    QString interfaceID() { return "ActionInterface_QtDesigner_Trolltech_05102000_0515"; }
};

#endif
