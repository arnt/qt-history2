#ifndef QACTIONINTERFACE_H
#define QACTIONINTERFACE_H

#include "qplugininterface.h"
#include <qstringlist.h>
#include <qobject.h>

class QAction;
class QActionInterface;

class QApplicationInterface : public QObject
{
    Q_OBJECT
signals:
    void openFile();
};

class QActionInterface : public QPlugInInterface
{
public:
    QString queryInterface() { return "QActionInterface"; }

    virtual QAction* create( const QString&, QObject* parent = 0 ) = 0;
    virtual QApplicationInterface* appInterface() = 0;
};

#endif // QACTIONINTERFACE_H
