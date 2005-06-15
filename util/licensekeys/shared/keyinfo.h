#ifndef KEYINFO_H
#define KEYINFO_H

#include <QtCore/QDate>
#include <QtCore/QByteArray>

// Products
enum {
    // the first 10 bits are reserved for Qt editions
    QtUniversal     = 0x00000001,
    QtDesktop       = 0x00000002,
    QtDesktopLight  = 0x00000004,
    QtConsole       = 0x00000008,
    QtDesignerOnly  = 0x00000010,
    QtReserved1     = 0x00000020,
    QtReserved2     = 0x00000040,
    QtReserved3     = 0x00000080,
    QtReserved4     = 0x00000100,
    QtReserved5     = 0x00000200,

    // the next 10 bits are reserved for Qtopia editions
    QtopiaPDA       = 0x00000400,
    QtopiaPhone     = 0x00000800,
    QtopiaRefboard  = 0x00001000,
    QtopiaReserved1 = 0x00002000,
    QtopiaReserved2 = 0x00004000,
    QtopiaReserved3 = 0x00008000,
    QtopiaReserved4 = 0x00010000,
    QtopiaReserved5 = 0x00020000,
    QtopiaReserved6 = 0x00040000,
    QtopiaReserved7 = 0x00080000,

    // other products get the upper 12 bits
    Teambuilder     = 0x00100000,
    Solutions       = 0x00200000,
    QSA             = 0x00400000,
    OtherReserved1  = 0x00800000,
    OtherReserved2  = 0x01000000,
    OtherReserved3  = 0x02000000,
    OtherReserved4  = 0x04000000,
    OtherReserved5  = 0x08000000,
    OtherReserved6  = 0x10000000,
    OtherReserved7  = 0x20000000,
    OtherReserved8  = 0x40000000,
    OtherReserved9  = 0x80000000,
};
static const char *Products[] = {
    "Qt Universal",
    "Qt Desktop",
    "Qt Desktop Light",
    "Qt Console",
    "Qt Designer Only",
    0,
    0,
    0,
    0,
    0,

    "Qtopia PDA",
    "Qtopia Phone",
    "Qtopia Reference Board",
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    "Teambuilder",
    "Solutions",
    "QSA",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

// Platforms
enum {
    X11      = 0x0001,
    Embedded = 0x0002,
    Windows  = 0x0004,
    Mac      = 0x0008
};
static const char *Platforms[] = {
    "X11",
    "Embedded",
    "Windows",
    "Mac"
};

// License Schema
enum {
    SupportedEvaluation   = 0x0001,
    UnsupportedEvaluation = 0x0002,
    FullSourceEvaluation  = 0x0004,
    FullCommercial        = 0x0008
};
static const char *LicenseSchemas[] = {
    "Supported Evaluation",
    "Unsupported Evaluation",
    "Full Source Evaluation",
    "Full Commercial"
};

// License Feature
enum {
    USCustomer = 0x0001
};
static const char *LicenseFeatures[] = {
    "US Customer",
    0,
    0,
    0
};

enum {
    NumberOfProducts = sizeof(Products) / sizeof(const char *),
    NumberOfPlatforms = sizeof(Platforms) / sizeof(const char *),
    NumberOfLicenseSchemas = sizeof(LicenseSchemas) / sizeof(const char *),
    NumberOfLicenseFeatures = sizeof(LicenseFeatures) / sizeof(const char *),
};

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

bool decodeLicenseKey(const QByteArray &licenseKey,
                      uint *products,
                      uint *platforms,
                      uint *licenseSchema,
                      uint *licenseFeatures,
                      uint *licenseID,
                      QDate *expiryDate);

#endif // KEYINFO_H
