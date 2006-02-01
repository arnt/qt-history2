#include "keyinfo.h"

#include <QList>

#include <stdio.h>

enum {
    ProductMagic       = 0xB292,
    PlatformMagic      = 0x5C7E,
    LicenseSchemaMagic = 0xE3B,
    LicenseIDMagic     = 0xCE57,
    FeatureMagic       = 0x4D5,
    ExpiryDateMagic    = 0x5B7EC4
};

static const char Xalphabet[] = "WX9MUEC7AJH3KS6DB4YFG2L5PQRT8VNZ";
static const int XAlphabetSize = sizeof(Xalphabet) - 1;

static QByteArray encodeBaseX(uint k)
{
    QByteArray str;
    do {
	str += Xalphabet[ k % XAlphabetSize ];
	k /= XAlphabetSize;
    } while (k > 0u);
    return str;
}

static QByteArray encodedExpiryDateExt(const QDate& date)
{
    uint day = date.toJulianDay();
    return encodeBaseX(day ^ ExpiryDateMagic);
}

QByteArray generateLicenseKey(uint products,
                              uint platforms,
                              uint licenseSchema,
                              uint licenseFeatures,
                              uint licenseID,
                              QDate expiryDate)
{
    QByteArray productPart = encodeBaseX(products ^ ProductMagic);
    QByteArray platformPart = encodeBaseX(platforms ^ PlatformMagic);
    QByteArray licenseSchemaPart = encodeBaseX(licenseSchema ^ LicenseSchemaMagic);
    QByteArray licenseFeaturePart = encodeBaseX(licenseFeatures ^ FeatureMagic);
    QByteArray licenseIDPart = encodeBaseX(licenseID ^ LicenseIDMagic);
    QByteArray expiryPart = encodedExpiryDateExt(expiryDate);
    QByteArray keyPart = productPart
                         + '-' + platformPart
                         + '-' + licenseSchemaPart
                         + '-' + licenseFeaturePart
                         + '-' + licenseIDPart
                         + '-' + expiryPart;
    int crc = qChecksum(keyPart.constData(), keyPart.size());
    QByteArray checksumPart = QString("%1%2")
                              .arg((crc & 0xff),2,16,QChar('0'))
                              .arg((crc >> 8 & 0xff),2,16,QChar('0'))
                              .toUpper().toLatin1();
    return keyPart + '-' + checksumPart;
}

static uint processMultiple(const char *str, const char *list[], int listSize)
{
    uint returnValue = 0u;
    // process the product argument
    const char *end;
    do {
        QByteArray arg;
        end = strchr(str, ',');
        if (end) {
            arg = QByteArray(str, end - str);
            str = end + 1;
        } else {
            arg = str;
        }
        int p = 0;
        for (int i = 0; i < listSize; ++i) {
            if (arg == list[i]) {
                p = 1 << i;
                break;
            }
        }
        if (p == 0)
            return 0u;
        returnValue |= p;
    } while (end != NULL);
    return returnValue;
}

static uint processSingle(const char *str, const char *list[], int listSize)
{
    for (int i = 0; i < listSize; ++i) {
        if (qstrcmp(str, list[i]) == 0)
            return 1u << i;
    }
    return 0u;
}

QByteArray generateLicenseKey(const QByteArray &products,
                              const QByteArray &platforms,
                              const QByteArray &licenseSchema,
                              const QByteArray &licenseFeatures,
                              const QByteArray &licenseID,
                              const QByteArray &expiryDate)
{
    uint _products = processMultiple(products.constData(),
                                     Products,
                                     NumberOfProducts);
    if (_products == 0u) {
        fprintf(stderr,
                "generateLicenseKey: invalid products '%s'\n",
                products.constData());
        return QByteArray();
    }
    uint _platforms = processMultiple(platforms.constData(),
                                      Platforms,
                                      NumberOfPlatforms);
    if (_platforms == 0u) {
        fprintf(stderr,
                "generateLicenseKey: invalid platforms '%s'\n",
                platforms.constData());
        return QByteArray();
    }
    uint _licenseSchema = processSingle(licenseSchema.constData(),
                                        LicenseSchemas,
                                        NumberOfLicenseSchemas);
    if (_licenseSchema == 0u) {
        fprintf(stderr,
                "generateLicenseKey: invalid licenseSchema '%s'\n",
                licenseSchema.constData());
        return QByteArray();
    }
    uint _licenseFeatures = processMultiple(licenseFeatures.constData(),
                                            LicenseFeatures,
                                            NumberOfLicenseFeatures);
    // note: license features can be zero
    uint _licenseID = licenseID.toInt();
    if (_licenseID == 0u) {
        fprintf(stderr,
                "generateLicenseKey: invalid licenseID '%s'\n",
                licenseID.constData());
        return QByteArray();
    }
    QDate _expiryDate = QDate::fromString(expiryDate, "yyyy-MM-dd");
    if (!_expiryDate.isValid()) {
        fprintf(stderr,
                "generateLicenseKey: invalid expiryDate '%s'\n",
                expiryDate.constData());
        return QByteArray();
    }
    return generateLicenseKey(_products,
                              _platforms,
                              _licenseSchema,
                              _licenseFeatures,
                              _licenseID,
                              _expiryDate);
}

