#ifndef LICENSEFINDER_H
#define LICENSEFINDER_H

typedef unsigned long ulong;

class LicenseFinder
{
public:
    LicenseFinder();
    char *getLicenseKey(int part);
    char *getLicensee();

private:
    void searchLicense();
    bool lookInDirectory(const char* dir);
    char *findPattern(char *h, const char *n, ulong hlen);
    bool searched;
    char key1[5];
    char key2[5];
    char key3[5];
    char licensee[256];
};

#endif