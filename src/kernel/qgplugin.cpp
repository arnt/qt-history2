#include "qgplugin.h"
#include <private/qcom_p.h>

QGPlugin::QGPlugin()
    : _iface( 0 )
{
}

QGPlugin::QGPlugin( QUnknownInterface *i )
    : _iface( i )
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

