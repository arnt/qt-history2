#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "../../scripts/mac-binary/package/InstallerPane/keydec.h"

#include <QString>
#include <QStringList>
#include <QMap>


class Tools
{
public:
    static QString specialRot13(const QString &original);
    static bool checkInternalLicense(QMap<QString,QString> &map);
    static void checkLicense(QMap<QString,QString> &dictionary, QMap<QString,QString> &licenseInfo,
                             const QString &path);
};

#endif // _TOOLS_H_

