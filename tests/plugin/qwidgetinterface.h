#ifndef QWIDGETINTERFACE_H
#define QWIDGETINTERFACE_H

#include "qplugininterface.h"
#include <qstringlist.h>

class QWidget;

class QWidgetInterface : public QPlugInInterface
{
public:
    QString queryInterface() { return "QWidgetInterface"; }

    virtual QWidget* create( const QString&, QWidget* parent = 0, const char* name = 0 ) = 0;
};

#endif
