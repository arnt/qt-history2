#ifndef GLOBALINFORMATION_H
#define GLOBALINFORMATION_H
#include <qstring.h>

class GlobalInformation
{
public:
    GlobalInformation();
    ~GlobalInformation();

    void setReconfig( bool );
    bool reconfig();
    void setQtVersionStr( const QString& );
    QString qtVersionStr();

private:
    bool _reconfig;
    QString _qtVersionStr;
};

extern GlobalInformation globalInformation;

#endif // GLOBALINFORMATION_H
