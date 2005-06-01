#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "keydec.h"

#define MAX_STRSIZE 256
#define NUMBER_OF_PARTS 7

const ushort KeyDecoder::crc_tbl[] = {
    0x0000, 0x1081, 0x2102, 0x3183,
    0x4204, 0x5285, 0x6306, 0x7387,
    0x8408, 0x9489, 0xa50a, 0xb58b,
    0xc60c, 0xd68d, 0xe70e, 0xf78f
};

const char KeyDecoder::Xalphabet[] = "WX9MUEC7AJH3KS6DB4YFG2L5PQRT8VNZ";
const int KeyDecoder::XAlphabetSize = sizeof(Xalphabet) - 1;
const CDate KeyDecoder::StartDate = CDate(2001, 1, 1);
const uint KeyDecoder::MaxDays = 4000;

CDate::CDate() : m_jd(0)
{
}

CDate::CDate(int y, int m, int d)
{
    m_jd = gregorianToJulian(y, m, d);
}

CDate::CDate(CDate *d)
{
    m_jd = d->m_jd;
}

int CDate::year() const
{
    int y, m, d;
    julianToGregorian(m_jd, y, m, d);
    return y;
}

int CDate::month() const
{
    int y, m, d;
    julianToGregorian(m_jd, y, m, d);
    return m;
}
    
int CDate::day() const
{
    int y, m, d;
    julianToGregorian(m_jd, y, m, d);
    return d;
}

int CDate::daysTo(CDate d) const
{
    return d.m_jd - m_jd;
}

CDate CDate::addDays(int ndays) const
{
    CDate d;
    d.m_jd = m_jd + ndays;
    return d;
}

uint CDate::gregorianToJulian(int y, int m, int d) const
{
    uint c, ya;
    if (y <= 99)
        y += 1900;
    if (m > 2) {
        m -= 3;
    } else {
        m += 9;
        y--;
    }
    c = y;                                        // NOTE: Sym C++ 6.0 bug
    c /= 100;
    ya = y - 100*c;
    return 1721119 + d + (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5;
}

void CDate::julianToGregorian(uint jd, int &y, int &m, int &d) const
{
    uint x;
    uint j = jd - 1721119;
    y = (j*4 - 1)/146097;
    j = j*4 - 146097*y - 1;
    x = j/4;
    j = (x*4 + 3) / 1461;
    y = 100*y + j;
    x = (x*4) + 3 - 1461*j;
    x = (x + 4)/4;
    m = (5*x - 3)/153;
    x = 5*x - 3 - 153*m;
    d = (x + 5)/5;
    if (m < 10) {
        m += 3;
    } else {
        m -= 9;
        y++;
    }
}

void KeyDecoder::encodeBaseX(uint k, char *str)
{
    memset(str, 0, MAX_STRSIZE);

    do {
        str[strlen(str)] = Xalphabet[ k % XAlphabetSize ];
	k /= XAlphabetSize;
    } while (k > 0u);
}

uint KeyDecoder::decodeBaseX(const char *str)
{
    uint k = 0;
    int i = (int)strlen(str);
    while (i > 0) {
	i--;
	const char *p = strchr(Xalphabet, str[i]);
	if (p == 0) {
	    return 0;
	} else {
	    k = (k * XAlphabetSize) + (p - Xalphabet);
	}
    }
    return k;
}

void KeyDecoder::encodedExpiryDate(const CDate &date, char *str)
{
    uint days = StartDate.daysTo(date);
    if (days >= MaxDays)
	days = MaxDays - 1;
    uint x = (days << 7) ^ days;
    return encodeBaseX(x ^ 0x0000beef, str);
}

CDate KeyDecoder::decodedExpiryDate(const char *encodedDate)
{
    CDate date;
    uint y = decodeBaseX(encodedDate);
    uint x = y ^ 0x0000beef;
    uint days = ( (x >> 7) ^ x ) >> 7;
    if (days >= MaxDays)
        return CDate();
    date = StartDate.addDays(days);

    char str[MAX_STRSIZE];
    encodedExpiryDate(date, str);

    if (strcmp(encodedDate, str) != 0)
        return CDate();
    return date;
}

ushort KeyDecoder::qChecksum(const char *data, uint len)
{
    ushort crc = 0xffff;
    uchar c;
    const uchar *p = reinterpret_cast<const uchar *>(data);
    while (len--) {
        c = *p++;
        crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
        c >>= 4;
        crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
    }
    return ~crc & 0xffff;
}

KeyDecoder::KeyDecoder(const char *clicenseKey)
    : products(0), platforms(0), licenseSchema(0), licenseFeatures(0), licenseID(0)
{
    m_valid = false;

    char *licenseKey = (char *)malloc(strlen(clicenseKey) * sizeof(char));
    char *crcCheckKey = (char *)malloc(strlen(clicenseKey) * sizeof(char));
    strcpy(licenseKey, clicenseKey);

    int partNumber = 0;
    char *licenseParts[NUMBER_OF_PARTS];
    licenseParts[0] = licenseKey;

    while((*licenseKey) != '\0') {
        licenseKey++;
        if ((*licenseKey) == '-') {
            partNumber++;
            if (partNumber >= NUMBER_OF_PARTS) {
                free(licenseKey);
                free(crcCheckKey);
                return; //invalid key
            }
            (*licenseKey) = '\0';
            licenseParts[partNumber] = ++licenseKey;
        }
    }

    if (partNumber < (NUMBER_OF_PARTS-1))
        return; //invalid key

    sprintf(crcCheckKey, "%s-%s-%s-%s-%s-%s",
        licenseParts[0], licenseParts[1], licenseParts[2],
        licenseParts[3], licenseParts[4], licenseParts[5]);
    
    int crc = qChecksum(crcCheckKey, (uint)strlen(crcCheckKey));

    char checksumVerification[5];
    sprintf(checksumVerification, "%.2X%.2X", (crc & 0xff), (crc >> 8 & 0xff));

    if (strcmp(checksumVerification, licenseParts[6]) != 0) {
        return; //invalid checksum
    }

    products = decodeBaseX(licenseParts[0]) ^ ProductMagic;
    platforms = decodeBaseX(licenseParts[1]) ^ PlatformMagic;
    licenseSchema = decodeBaseX(licenseParts[2]) ^ LicenseSchemaMagic;
    licenseFeatures = decodeBaseX(licenseParts[3]) ^ FeatureMagic;
    licenseID = decodeBaseX(licenseParts[4]) ^ LicenseIDMagic;
    expiryDate = decodedExpiryDate(licenseParts[5]);

    m_valid = true;
}
