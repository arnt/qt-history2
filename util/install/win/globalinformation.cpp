#include "globalinformation.h"

GlobalInformation::GlobalInformation() :
    _qtVersionStr( QT_VERSION_STR ),
    _reconfig( FALSE )
{
#if defined(Q_OS_WIN32)
    _sysId = MSVC;
#elif defined(Q_OS_MAC)
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
	else
	    str = "nmake.exe";
	break;
    case MSVCNET:
	if (t ==  IDE)
	    str = "Microsoft Visual Studio .NET";
	else if (t == Mkspec)
	    str = "win32-msvc.net";
	else
	    str = "nmake.exe";
	break;
    case Watcom:
	if (t ==  IDE)
	    str = "Watcom compiler";
	else if (t == Mkspec)
	    str = "win32-watcom";
	else
	    str = "nmake.exe"; //?
	break;
    case Intel:
	if (t ==  IDE)
	    str = "Intel compiler";
	else if (t == Mkspec)
	    str = "win32-icc";
	else
	    str = "nmake.exe";
	break;
    case GCC:
	if (t ==  IDE)
	    str = "GNU C++";
	else if (t == Mkspec)
	    str = "win32-g++";
	else
	    str = "gmake.exe";
	break;
    case MACX:
	if (t ==  IDE)
	    str = "MAC X buildtool";
	if (t == Mkspec)
	    str = "mac-g++";
	else
	    str = "make";
	break;
    case MinGW:
	if (t ==  IDE)
	    str = "MinGW C++";
	else if (t == Mkspec)
	    str = "win32-g++";
	else
	    str = "mingw32-make.exe";
	break;
    case Borland:
	if (t == IDE)
	    str = "Borland C++ Builder";
	else if (t == Mkspec)
	    str = "win32-borland";
	else
	    str = "make.exe";
	break;
    default:
	if (t ==  IDE)
	    str = "IDE";
	else if (t == Mkspec)
	    str = "Custom";
	else
	    str = "make.exe";
	break;
    }

    return str;
}
