#ifndef GLOBALINFORMATION_H
#define GLOBALINFORMATION_H
#include <qstring.h>

class GlobalInformation
{
public:
    GlobalInformation();
    ~GlobalInformation();

    void setReconfig( bool );
    bool reconfig() const;
    void setQtVersionStr( const QString& );
    QString qtVersionStr() const;
#if defined(QSA)
    void setQsaVersionStr( const QString& );
    QString qsaVersionStr() const;
#endif

    enum SysId {
	MSVC	= 0,
	Borland	= 1,
	GCC	= 2,
	MACX	= 3,
	MSVCNET = 4,
	MinGW   = 5,
	Other   = 6
    };
    void setSysId( SysId );
    SysId sysId() const;

private:
    bool _reconfig;
    QString _qtVersionStr;
#if defined(QSA)
    QString _qsaVersionStr;
#endif
    SysId _sysId;
};

extern GlobalInformation globalInformation;

#endif // GLOBALINFORMATION_H
