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

    enum SysId {
	MSVC	= 0,
	Borland	= 1,
	GCC	= 2,
	MACX	= 3
    };
    void setSysId( SysId );
    SysId sysId() const;

private:
    bool _reconfig;
    QString _qtVersionStr;
    SysId _sysId;
};

extern GlobalInformation globalInformation;

#endif // GLOBALINFORMATION_H
