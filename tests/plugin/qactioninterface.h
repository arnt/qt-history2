#ifndef QACTIONINTERFACE_H
#define QACTIONINTERFACE_H

#include "qplugininterface.h"
#include <qstringlist.h>

class QAction;
class QObject;

class QActionInterface : public QPlugInInterface
{
public:
    virtual QStringList actions() = 0;
    virtual QAction* create( const QString&, bool& self, QObject* parent = 0 ) = 0;
};

#endif //QACTIONINTERFACE_H