#ifndef RESOURCE_H
#define RESOURCE_H

#include <qcstring.h>
#include <qstring.h>

class ResourceLoader
{
public:
    ResourceLoader( char *resourceName, int minimumSize=0 );
    ~ResourceLoader();

    bool isValid() const;
    QByteArray data();

private:
    bool valid;
    int arSize;
    char *arData;
    QByteArray ba;
};

#if defined(Q_OS_WIN32)
class ResourceSaver
{
public:
    ResourceSaver( const QString& appName );
    ~ResourceSaver();

    bool setData( char *resourceName, const QByteArray &data, QString *errorMessage=0 );

private:
    QString applicationName;
};
#endif

#endif // RESOURCE_H
