#ifndef QACTIONINTERFACE_H
#define QACTIONINTERFACE_H

#include "qplugininterface.h"
#include <qstringlist.h>

class QAction;
class QObject;

class QActionInterface : public QPlugInInterface
{
public:
    QString queryPlugInInterface() { return "QActionInterface"; }

    virtual QAction* create( const QString&, QObject* parent = 0 ) = 0;
};

#endif // QACTIONINTERFACE_H
