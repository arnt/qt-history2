#ifndef QACTIONINTERFACE_H
#define QACTIONINTERFACE_H

#include "qplugininterface.h"
#include <qstringlist.h>

class QAction;
class QObject;

class QActionInterface : public QPlugInInterface
{
public:
    QString queryInterface() { return "QActionInterface"; }

    virtual QStringList actions() = 0;
    virtual QAction* create( const QString&, QObject* parent = 0 ) = 0;
};

#endif // QACTIONINTERFACE_H
