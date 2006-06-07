#ifndef KEYINFO_H
#define KEYINFO_H

#include "../../scripts/mac-binary/package/InstallerPane/keydec.h"
#include <QDate>
#include <QByteArray>

QByteArray generateLicenseKey(uint products,
                              uint platforms,
                              uint licenseSchema,
                              uint licenseFeatures,
                              uint licenseID,
                              QDate expiryDate);

QByteArray generateLicenseKey(const QByteArray &products,
                              const QByteArray &platforms,
                              const QByteArray &licenseSchema,
                              const QByteArray &licenseFeatures,
                              const QByteArray &licenseID,
                              const QByteArray &expiryDate);

#endif // KEYINFO_H
