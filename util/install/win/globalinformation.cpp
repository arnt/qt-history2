#include "globalinformation.h"

GlobalInformation::GlobalInformation() :
    _qtVersionStr( QT_VERSION_STR ),
    _reconfig( FALSE )
{
#if defined(Q_OS_WIN32)
    _sysId = Other;
#elif defined(Q_OS_MACX)
    _sysId = MACX;
#else
    _sysId = MingW32;
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

#if defined(QSA)
void GlobalInformation::setQsaVersionStr( const QString& qvs )
{
    _qsaVersionStr = qvs;
}

QString GlobalInformation::qsaVersionStr() const
{
    return _qsaVersionStr;
}
#endif

void GlobalInformation::setSysId( SysId s )
{
    _sysId = s;
}

GlobalInformation::SysId GlobalInformation::sysId() const
{
    return _sysId;
}

QString GlobalInformation::text(Text t) const
{
    QString str;

    switch (_sysId) {
    case MSVC:
	if (t ==  IDE)
	    str = "Microsoft Visual Studio 6.0";
	else if (t == Mkspec)
	    str = "win32-msvc";
	else if (t == MakeTool)
	    str = "nmake.exe";
	break;
    case MSVCNET:
	if (t ==  IDE)
	    str = "Microsoft Visual Studio .NET";
	else if (t == Mkspec)
	    str = "win32-msvc.net";
	else if (t == MakeTool)
	    str = "nmake.exe";
	break;
    case Watcom:
	if (t == Mkspec)
	    str = "win32-watcom";
	else if (t == MakeTool)
	    str = "wmake.exe";
	break;
    case Intel:
	if (t == Mkspec)
	    str = "win32-icc";
	else if (t == MakeTool)
	    str = "nmake.exe";
	break;
    case GCC:
	if (t == Mkspec)
	    str = "win32-g++";
	else if (t == MakeTool)
	    str = "gmake.exe";
	break;
    case MACX:
	if (t == Mkspec)
	    str = "mac-g++";
	else if (t == MakeTool)
	    str = "make";
	break;
    case MinGW:
	if (t == Mkspec)
	    str = "win32-g++";
	else if (t == MakeTool)
	    str = "mingw32-make.exe";
	break;
    case Borland:
	if (t == Mkspec)
	    str = "win32-borland";
	else if (t == MakeTool)
	    str = "make.exe";
	break;
    default:
	if (t == Mkspec)
	    str = "Custom";
	else if (t == MakeTool)
	    str = "make.exe";
	break;
    }

    return str;
}
