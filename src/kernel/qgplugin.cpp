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
    if ( _iface )
	return 0;
    QUnknownInterface *i;
    _iface->queryInterface( IID_QUnknown, &i );
    return i;
}

