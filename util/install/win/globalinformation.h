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
	MSVCNET = 0,
	MSVC	= 1,
	Borland	= 2,
	MinGW   = 3,
	Other   = 4,
	Watcom  = 5,
	Intel	= 6,
	GCC	= 7,
	MACX	= 8
    };
    void setSysId( SysId );
    SysId sysId() const;

    enum Text {
	MakeTool,
	IDE,
	Mkspec
    };

    QString text(Text t) const;

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
