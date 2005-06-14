#ifndef KEYDEC_H
#define KEYDEC_H

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

class CDate
{
public:
    CDate();
    CDate(int y, int m, int d);
    CDate(CDate *d);
    CDate(uint julianDays);

    int year() const;
    int month() const;
    int day() const;
    uint julianDate() const { return m_jd; }
    
private:
    uint gregorianToJulian(int y, int m, int d) const;
    void julianToGregorian(uint jd, int &y, int &m, int &d) const;
    uint m_jd;
};

class KeyDecoder
{
private:
    // Magic values
    enum {
        ProductMagic = 0xB292,
        PlatformMagic = 0x5C7E,
        LicenseSchemaMagic = 0xE3B,
        LicenseIDMagic = 0xCE57,
        FeatureMagic = 0x4D5
    };

    static const ushort crc_tbl[16];
    static const char Xalphabet[];
    static const int XAlphabetSize;
    static const CDate StartDate;
    static const uint MaxDays;

    void encodeBaseX(uint k, char *str);
    uint decodeBaseX(const char *str);

    void encodedExpiryDate(const CDate &date, char *str);
    CDate decodedExpiryDate(const char *encodedDate);

    ushort qChecksum(const char *data, uint len);

    uint products;
    uint platforms;
    uint licenseSchema;
    uint licenseFeatures;
    uint licenseID;
    CDate expiryDate;
    bool m_valid;

public:
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
        QtProductMask   = 0x000003ff,

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

    // Platforms
    enum {
        PlatformX11 = 0x01,
        PlatformEmbedded = 0x02,
        PlatformWindows = 0x04,
        PlatformMac = 0x08
    };

    // LicenseSchema
    enum {
        SupportedEvaluation = 0x01,
        UnsupportedEvaluation = 0x02,
        FullSourceEvaluation = 0x04,
        FullCommercial = 0x08
    };

    // LicenseFeatures
    enum {
        USCustomer = 0x01
    };

    KeyDecoder(const char *licenseKey);

    uint getProducts() {return products;}
    uint getPlatforms() {return platforms;}
    uint getLicenseSchema() {return licenseSchema;}
    uint getLicenseFeatures() {return licenseFeatures;}
    uint getLicenseID() {return licenseID;}
    CDate getExpiryDate() {return expiryDate;}

    bool IsValid() { return m_valid; }
};

#endif //KEYDEC_H