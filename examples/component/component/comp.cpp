#include "comp.h"

QRESULT Component::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_Print )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}
