#include "qgplugin.h"
#include <private/qcom_p.h>

QGPlugin::QGPlugin()
{
}

QGPlugin::~QGPlugin()
{
}

QUnknownInterface* QGPlugin::iface()
{
    Q_ASSERT( _iface );
    QUnknownInterface *i;
    _iface->queryInterface( IID_QUnknown, &i );
    return i;
}

