#include "globalinformation.h"

GlobalInformation::GlobalInformation() :
    _qtVersionStr( QT_VERSION_STR ),
    _reconfig( FALSE )
{
#if defined(Q_OS_WIN32)
    _sysId = MSVC;
#elif Q_OS_MACX
    _sysId = MACX;
#else
    _sysId = GCC;
#endif
}

GlobalInformation::~GlobalInformation()
{
}

void GlobalInformation::setReconfig( bool r )
{
    _reconfig = r;
}

bool GlobalInformation::reconfig() const
{
    return _reconfig;
}

void GlobalInformation::setQtVersionStr( const QString& qvs )
{
    _qtVersionStr = qvs;
}

QString GlobalInformation::qtVersionStr() const
{
    return _qtVersionStr;
}

void GlobalInformation::setSysId( SysId s )
{
    _sysId = s;
}

GlobalInformation::SysId GlobalInformation::sysId() const
{
    return _sysId;
}
