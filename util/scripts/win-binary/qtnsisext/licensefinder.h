#ifndef LICENSEFINDER_H
#define LICENSEFINDER_H

#define MAX_KEY_LENGTH 512
#define MAX_LICENSEE_LENGTH 512

typedef unsigned long ulong;

class LicenseFinder
{
public:
    LicenseFinder();
    char *getLicenseKey();
    char *getLicensee();

private:
    void searchLicense();
    bool lookInDirectory(const char* dir);
    char *findPattern(char *h, const char *n, ulong hlen);
    bool searched;
    char m_key[MAX_KEY_LENGTH];
    char licensee[MAX_LICENSEE_LENGTH];
};

#endif